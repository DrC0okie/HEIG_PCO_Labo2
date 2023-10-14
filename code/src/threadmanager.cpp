#include "threadmanager.h"
#include <pcosynchro/pcomutex.h>
#include <pcosynchro/pcothread.h>

#include <atomic>
#include <QVector>
#include <memory>
#include <string>
#include <vector>

#include "math.h"
#include "mythread.h"


ThreadManager::ThreadManager(QObject* parent)
    : QObject(parent) {
    threadPool          = ThreadPool();
    workQueue           = std::queue<std::pair<int, int>>();
    foundPassword       = QString();
    totalChunks         = 0;
    chunkProgressFactor = 0.;
}

void ThreadManager::incrementPercentComputed(double percentComputed) {
    emit sig_incrementPercentComputed(percentComputed);
}

QString ThreadManager::startHacking(
    QString      charset,
    QString      salt,
    QString      hash,
    unsigned int nbChars,
    unsigned int nbThreads) {
    // Reset member variables.
    foundPassword = QString();
    threadPool.clear();

    const size_t totalCombinations = std::pow(charset.size(), nbChars);
    const size_t basicChunkSize    = totalCombinations / (nbThreads);

    totalChunks         = std::ceil((double)totalCombinations / (double)basicChunkSize);
    chunkProgressFactor = 1. / totalChunks;
    double unitProgressFactor  = chunkProgressFactor / basicChunkSize;
    size_t countForProgress = basicChunkSize / 16; // FIXME: tweak this value to adjust how much reporting we want
    setupWork(totalCombinations, basicChunkSize);

    std::atomic<bool> foundFlag(false);

    // Create and launch worker threads.
    for (size_t i = 0; i < nbThreads; i++) {
        size_t start, end;
        std::tie(start, end) = workQueue.front();
        workQueue.pop();
        ThreadParameters params = {*this, charset, salt, hash, nbChars, std::ref(foundFlag), start, end, unitProgressFactor, countForProgress};
        PcoThread*       t      = new PcoThread(BruteForceThread::run, params);
        threadPool.push_back(std::unique_ptr<PcoThread>(t));
    }

    joinThreads();

    // Return the found password.
    return foundPassword;
}

void ThreadManager::setupWork(size_t combinations, size_t size) {
    for (size_t i = 0; i < combinations; i += size) {
        int end = std::min(i + size, combinations);
        workQueue.push({i, end});  // Push the range to the queue for threads to pick up.
    }
}

void ThreadManager::startWork(size_t count) {
}

bool ThreadManager::getWork(size_t& start, size_t& end) {
    // A chunk was done, increment the progress bar.
//    incrementPercentComputed(chunkProgressFactor);

    return true;
}

void ThreadManager::joinThreads() {
    for (auto& thread : threadPool) {
        thread->join();
    }
}

void ThreadManager::setFoundPassword(QString password) {
    foundPassword = password;
}

void ThreadManager::cancelWork() {
    for (auto& thread : threadPool) {
        thread->requestStop();
    }
    std::queue<std::pair<int, int>> emptyQueue;
    std::swap(workQueue, emptyQueue);
}

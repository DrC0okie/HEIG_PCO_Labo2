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

PcoMutex ThreadManager::queueMutex = PcoMutex();
PcoMutex ThreadManager::resultMutex = PcoMutex();

ThreadManager::ThreadManager(QObject* parent)
    : QObject(parent) {
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
    // Reset foundPassword
    foundPassword = QString();

    const size_t totalCombinations = std::pow(charset.size(), nbChars);
    const size_t basicChunkSize    = totalCombinations / (nbThreads * 8);

    totalChunks         = std::ceil((double)totalCombinations / (double)basicChunkSize);
    chunkProgressFactor = 1. / totalChunks;
    setupWork(totalCombinations, basicChunkSize);

    ThreadPool        threads;
    std::atomic<bool> foundFlag(false);

    // Create and launch worker threads.
    ThreadParameters params = {*this, charset, salt, hash, nbChars, std::ref(foundFlag)};
    startWork(threads, params, nbThreads);

    joinThreads(threads);

    // Return the found password.
    return foundPassword;
}

void ThreadManager::setupWork(size_t combinations, size_t size) {
    for (size_t i = 0; i < combinations; i += size) {
        int end = std::min(i + size, combinations);
        workQueue.push({i, end});  // Push the range to the queue for threads to pick up.
    }
}

void ThreadManager::startWork(ThreadPool& pool, ThreadParameters params, size_t count) {
    for (size_t i = 0; i < count; i++) {
        PcoThread* t = new PcoThread(BruteForceThread::run, params);
        pool.push_back(std::unique_ptr<PcoThread>(t));
    }
}

bool ThreadManager::getWork(size_t& start, size_t& end) {
    queueMutex.lock();  // This is the critical section.
    if (workQueue.empty()) {
        queueMutex.unlock();
        return false;
    }

    std::tie(start, end) = workQueue.front();  // Get a new chunk. FIXME: should it include percentage incrementation ?
    workQueue.pop();
    queueMutex.unlock();

    // A chunk was done, increment the progress bar.
    incrementPercentComputed(chunkProgressFactor);

    return true;
}

void ThreadManager::joinThreads(ThreadPool& pool) {
    for (auto& thread : pool) {
        thread->join();
    }
}

void ThreadManager::setFoundPassword(QString password) {
    foundPassword = password;
}

void ThreadManager::cancelWork() {
    std::queue<std::pair<int, int>> emptyQueue;
    std::swap(workQueue, emptyQueue);
}

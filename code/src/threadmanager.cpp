#include "threadmanager.h"

#include <pcosynchro/pcomutex.h>
#include <pcosynchro/pcothread.h>

#include <atomic>
#include <cmath>

#include "mythread.h"


ThreadManager::ThreadManager(QObject* parent)
    : QObject(parent),
      workQueue(),
      threadPool(),
      threadCount(0),
      hashesToCompute(0),
      countForProgress(0),
      foundPassword() {
}

void ThreadManager::resetInstance() {
    workQueue = WorkQueue();
    threadPool.clear();
    threadCount      = 0;
    hashesToCompute  = 0;
    countForProgress = 0;
    foundPassword    = QString();
}

QString ThreadManager::startHacking(
    QString      charset,
    QString      salt,
    QString      hash,
    unsigned int nbChars,
    unsigned int nbThreads) {
    // Reset the instance in case the caller did not create a new one.
    resetInstance();

    threadCount     = nbThreads;
    hashesToCompute = std::pow(charset.size(), nbChars);
    // Report every 1% of the work of each thread.
    countForProgress = static_cast<size_t>(std::ceil(hashesToCompute / 100 / threadCount));

    setupWork(hashesToCompute);

    // Flag needed to stop the threads when the password is found.
    // The use of std::atomic ensures that the flag is updated atomically.
    std::atomic<bool> foundFlag(false);

    BruteForceThread::Parameters defaultParams = {
        std::bind(&ThreadManager::setFoundPassword, this, std::placeholders::_1),
        std::bind(&ThreadManager::incrementProgress, this),
        charset, salt, hash, nbChars, std::ref(foundFlag), 0, 0, countForProgress};

    startWork(defaultParams);

    joinThreads();

    incrementProgress();  // Account for the chunk of work that found the password if any.

    return foundPassword;
}

void ThreadManager::setupWork(size_t combinations) {
    const size_t size = hashesToCompute / threadCount;

    for (size_t i = 0; i < combinations; i += size) {
        const int end = std::min(i + size, combinations);
        workQueue.push({i, end});  // Push the range to the queue for the threads to pick up.
    }
}

void ThreadManager::startWork(BruteForceThread::Parameters params) {
    BruteForceThread thread;
    size_t           start, end;

    // Create and launch worker threads.
    for (size_t i = 0; i < threadCount; i++) {
        std::tie(start, end) = workQueue.front();
        workQueue.pop();
        params.rangeStart = start;
        params.rangeEnd   = end;

        // Just as with std::thread, we need to pass the object along with the method.
        // We're using unique_ptr to avoid having to deallocate afterward.
        threadPool.push_back(std::make_unique<PcoThread>(&BruteForceThread::run, &thread, params));
    }
}

void ThreadManager::joinThreads() {
    for (auto& thread : threadPool) {
        thread->join();
    }
}

void ThreadManager::setFoundPassword(QString password) {
    foundPassword = password;
    cancelWork();
}

void ThreadManager::cancelWork() {
    for (auto& thread : threadPool) {
        thread->requestStop();
    }
}

void ThreadManager::incrementProgress() {
    // Compute the progress percentage as one over a hundredth and emit the signal.
    const double percent = countForProgress * (100. / hashesToCompute) / 100;
    emit         sig_incrementPercentComputed(percent);
}

/**
 * \file threadmanager.cpp
 * \author Yann Thoma
 * \author Timothée Van Hove <timothe.vanhove@heig-vd.ch>
 * \author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * \date Created on 24.02.2017. Last modified on 16.10.2023.
 * \brief Implementation of the ThreadManager class.
 */

#include "threadmanager.h"

#include <atomic>  // std::atomic
#include <cmath>   // std::ceil
#include <tuple>   // std::tie


ThreadManager::ThreadManager(QObject* parent)
    : QObject(parent),
      workQueue(),
      threadPool(),
      threadCount(0),
      hashesToCompute(0),
      countForProgress(0),
      foundPassword() {}

void ThreadManager::resetInstance() {
    workQueue = WorkQueue();
    threadPool.clear();
    threadCount      = 0;
    hashesToCompute  = 0;
    countForProgress = 0;
    foundPassword    = QString();
}

QString ThreadManager::startHacking(QString      charset,
                                    QString      salt,
                                    QString      hash,
                                    unsigned int nbChars,
                                    unsigned int nbThreads) {
    // Reset the instance in case the caller did not create a new one.
    resetInstance();

    threadCount     = nbThreads;
    hashesToCompute = std::pow(charset.size(), nbChars);
    // Report every 1% of the work of each thread.
    countForProgress = static_cast<std::size_t>(
        std::ceil(hashesToCompute / 100 / threadCount));

    setupWork(hashesToCompute);

    // Flag needed to stop the threads when the password is found.
    // The use of std::atomic ensures that the flag is updated atomically.
    std::atomic<bool> foundFlag(false);

    BruteForceThread::Parameters defaultParams = {
        .passwordFoundCallback = std::bind(
            &ThreadManager::setFoundPassword, this, std::placeholders::_1),
        .progressCallback = std::bind(&ThreadManager::incrementProgress, this),
        .charset          = charset,
        .salt             = salt,
        .hash             = hash,
        .length           = nbChars,
        .flag             = std::ref(foundFlag),
        .rangeStart       = 0,
        .rangeEnd         = 0,
        .countForProgress = countForProgress};

    startWork(defaultParams);

    joinThreads();

    incrementProgress();  // Account for the chunk of work that found the
                          // password if any.

    return foundPassword;
}

void ThreadManager::setupWork(std::size_t combinations) {
    const std::size_t size = hashesToCompute / threadCount;

    // Push the range to the queue for the threads to pick up.
    for (std::size_t i = 0; i < combinations; i += size) {
        const int end = std::min(i + size, combinations);
        workQueue.push({i, end});
    }
}

void ThreadManager::startWork(BruteForceThread::Parameters params) {
    BruteForceThread thread;
    std::size_t      start, end;

    // Create and launch worker threads.
    for (std::size_t i = 0; i < threadCount; i++) {
        std::tie(start, end) = workQueue.front();
        workQueue.pop();
        params.rangeStart = start;
        params.rangeEnd   = end;

        // Just as with std::thread, we need to pass the object along with the
        // method. We're using unique_ptr to avoid having to deallocate manually
        // afterward.
        threadPool.push_back(std::make_unique<PcoThread>(
            &BruteForceThread::run, &thread, params));
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
    // Compute the progress percentage as one over a hundredth and emit the
    // signal.
    const double percent = countForProgress * (100. / hashesToCompute) / 100;
    emit         sig_incrementPercentComputed(percent);
}

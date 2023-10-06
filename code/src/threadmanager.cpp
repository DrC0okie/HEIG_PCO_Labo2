#include <QVector>
#include <string>
#include <vector>
#include <atomic>
#include <queue>
#include "math.h"
#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcomutex.h>

#include "threadmanager.h"
#include "mythread.h"


ThreadManager::ThreadManager(QObject *parent) :
    QObject(parent)
{
    workQueue = std::queue<std::pair<int, int>>();
    foundPassword = QString();
    totalChunks = 0;
    chunkProgressFactor = 0.;
}


void ThreadManager::incrementPercentComputed(double percentComputed)
{
    emit sig_incrementPercentComputed(percentComputed);
}

// This function starts the brute force hacking process using multiple threads.
// The function divides the work among the threads, waits for them to finish,
// and then returns the found password (if any).
QString ThreadManager::startHacking(
        QString charset,       // The character set for the password (e.g., "abc...XYZ0123...")
        QString salt,          // (Not used yet) Salt to be combined with the password before hashing
        QString hash,          // The target hash we're trying to match
        unsigned int nbChars,  // Length of the original password
        unsigned int nbThreads // Number of threads to use
)
{
    // Calculate total number of combinations based on charset size and password length.
    const size_t totalCombinations = std::pow(charset.size(), nbChars);

    // Determine the amount of work each thread should handle.
    // Here, each thread should handle approximately 8 chunks
    size_t basicChunkSize = totalCombinations / (nbThreads * 8);

    totalChunks = std::ceil((double)totalCombinations / (double)basicChunkSize);
    chunkProgressFactor = 1. / totalChunks;

    // Container for the worker threads.
    std::vector<PcoThread> threads;

    // Atomic flag to signal when the correct password has been found.
    std::atomic<bool> foundFlag(false);

    // Divide the total work into tasks of size basicChunkSize.
    for (size_t i = 0; i < totalCombinations; i += basicChunkSize) {
        int end = std::min(i + basicChunkSize, totalCombinations);
        workQueue.push({i, end});  // Push the range to the queue for threads to pick up.
    }

    // Create and launch worker threads.
    for (size_t i = 0; i < nbThreads; i++) {
        threads.push_back(PcoThread(
            [this, charset, hash, nbChars, &foundFlag]() {
                MyThread::bruteForceThread(*this, charset, nbChars, hash, foundFlag);
            }
        ));
    }

    // Wait for all threads to complete.
    for (auto& thread : threads) {
        thread.join();
    }

    // Return the found password.
    return foundPassword;
}

bool ThreadManager::getNextWorkChunk(size_t& start, size_t& end) {
    // Fetch a range of combinations to test from the workQueue.
    std::lock_guard<PcoMutex> lock(ThreadManager::queueMutex);
    if (workQueue.empty()){
        return false;
    }

    // Get new chunk
    std::tie(start, end) = workQueue.front();
    workQueue.pop();

    // A chunk was done. Increment the progress bar.
    incrementPercentComputed(chunkProgressFactor);

    return true;
}

void ThreadManager::setFoundPassword(QString password) {
    foundPassword = password;
}

void ThreadManager::clearWorkQueue() {
    std::lock_guard<PcoMutex> lock(ThreadManager::queueMutex);
    std::queue<std::pair<int, int>> emptyQueue;
    std::swap(workQueue, emptyQueue);
}



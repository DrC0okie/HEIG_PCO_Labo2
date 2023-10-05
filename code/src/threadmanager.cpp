#include <QVector>
#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <queue>
#include "math.h"
#include <pcosynchro/pcothread.h>
#include <pcosynchro/pcomutex.h>

#include "threadmanager.h"
#include "mythread.h"


std::queue<std::pair<int, int>> ThreadManager::workQueue = std::queue<std::pair<int, int>>();
PcoMutex ThreadManager::queueMutex = PcoMutex(); // Locks the access to the queue
PcoMutex ThreadManager::resultMutex = PcoMutex(); // Locks the access to the Qstring
QString ThreadManager::foundPassword = QString();

/*
 * std::pow pour les long long unsigned int
 */
long long unsigned int intPow (
        long long unsigned int number,
        long long unsigned int index)
{
    long long unsigned int i;

    if (index == 0)
        return 1;

    long long unsigned int num = number;

    for (i = 1; i < index; i++)
        number *= num;

    return number;
}

ThreadManager::ThreadManager(QObject *parent) :
    QObject(parent)
{}


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
            [charset, hash, nbChars, &foundFlag]() {
                MyThread::bruteForceThread(charset, nbChars, hash, foundFlag);
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

void ThreadManager::clearWorkQueue() {
    std::lock_guard<PcoMutex> lock(ThreadManager::queueMutex);
    std::queue<std::pair<int, int>> emptyQueue;
    std::swap(ThreadManager::workQueue, emptyQueue);
}



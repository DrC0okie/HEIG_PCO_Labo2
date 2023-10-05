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

// Concurrent work queue containing ranges
std::queue<std::pair<int, int>> workQueue;  // Thread-safe variant needed
PcoMutex queueMutex; //Locks the access to the queue
PcoMutex resultMutex; //LOcks the access to the Qstring
QString foundPassword = "";


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
                bruteForceThread(charset, nbChars, hash, foundFlag);
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

// This function represents the work done by each thread.
// Each thread picks up tasks (ranges) from the workQueue until either
// the password is found or there's no more work left.
void ThreadManager::bruteForceThread(const QString& charset, size_t desiredLength,
                  const QString& targetHash, std::atomic<bool>& foundFlag) {

    // MD5 hashing object
    QCryptographicHash md5(QCryptographicHash::Md5);
    size_t start, end;

    // Continue while the password hasn't been found.
    while (!foundFlag.load() && getNextWorkChunk(start, end)) {

        // Test each combination in the range.
        for (size_t i = start; i < end && !foundFlag.load(); i++) {
            QString combination = idToCombination(i, charset, desiredLength);
            QString hash = computeHash(combination, md5);

            // If a match is found, store the result, empty the queue and set the found flag.
            if (hash == targetHash) {
                std::lock_guard<PcoMutex> lock(resultMutex);
                foundPassword = combination;
                foundFlag.store(true);
                clearWorkQueue();
                return;
            }
        }
    }
}

bool ThreadManager::getNextWorkChunk(size_t& start, size_t& end) {

    // Fetch a range of combinations to test from the workQueue.
    std::lock_guard<PcoMutex> lock(queueMutex);
    if (workQueue.empty()){
        return false;
    }

    //Get new chunk
    std::tie(start, end) = workQueue.front();
    workQueue.pop();
    return true;
}

void ThreadManager::clearWorkQueue() {
    std::lock_guard<PcoMutex> lock(queueMutex);
    std::queue<std::pair<int, int>> emptyQueue;
    std::swap(workQueue, emptyQueue);
}

// Compute the MD5 hash of a given combination.
QString ThreadManager::computeHash(const QString &combination, QCryptographicHash &md5){
    md5.reset();
    // md5.addData(salt.toLatin1());  // TODO: To be implemented
    md5.addData(combination.toLatin1());

    // Convert the hash result to a hex string and return.
    return md5.result().toHex();
}

// Convert a number (id) to its corresponding combination in the charset.
// This function effectively converts the number to a base-n representation,
// where n is the size of the charset.
QString ThreadManager::idToCombination(size_t id, const QString& charset, size_t passwordLength){
    QString result(passwordLength, Qt::Uninitialized);  // Preallocate string

    for (int pos = passwordLength - 1; pos >= 0; --pos) {
        result[pos] = charset.at(id % charset.size());
        id /= charset.size();
    }

    return result;
}

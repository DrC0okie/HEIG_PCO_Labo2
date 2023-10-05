#include <QCryptographicHash>

#include "mythread.h"
#include "threadmanager.h"

// This function represents the work done by each thread.
// Each thread picks up tasks (ranges) from the workQueue until either
// the password is found or there's no more work left.
void MyThread::bruteForceThread(const QString& charset, size_t desiredLength,
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
                std::lock_guard<PcoMutex> lock(ThreadManager::resultMutex);
                ThreadManager::foundPassword = combination;
                foundFlag.store(true);
                ThreadManager::clearWorkQueue();
                return;
            }
        }
    }
}

bool MyThread::getNextWorkChunk(size_t& start, size_t& end) {
    // Fetch a range of combinations to test from the workQueue.
    std::lock_guard<PcoMutex> lock(ThreadManager::queueMutex);
    if (ThreadManager::workQueue.empty()){
        return false;
    }

    //Get new chunk
    std::tie(start, end) = ThreadManager::workQueue.front();
    ThreadManager::workQueue.pop();
    return true;
}

// Compute the MD5 hash of a given combination.
QString MyThread::computeHash(const QString &combination, QCryptographicHash &md5){
    md5.reset();
    // md5.addData(salt.toLatin1());  // TODO: To be implemented
    md5.addData(combination.toLatin1());

    // Convert the hash result to a hex string and return.
    return md5.result().toHex();
}

// Convert a number (id) to its corresponding combination in the charset.
// This function effectively converts the number to a base-n representation,
// where n is the size of the charset.
QString MyThread::idToCombination(size_t id, const QString& charset, size_t passwordLength){
    QString result(passwordLength, Qt::Uninitialized);  // Preallocate string

    for (int pos = passwordLength - 1; pos >= 0; --pos) {
        result[pos] = charset.at(id % charset.size());
        id /= charset.size();
    }

    return result;
}
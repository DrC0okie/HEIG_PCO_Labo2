#include "mythread.h"

#include <QCryptographicHash>

void BruteForceThread::run(ThreadManager&     manager,
                           const QString&     charset,
                           size_t             desiredLength,
                           const QString&     targetHash,
                           std::atomic<bool>& foundFlag) {
    size_t start, end;

    // Continue while the password hasn't been found.
    while (!foundFlag.load() && manager.getNextWorkChunk(start, end)) {
        // Test each combination in the range.
        for (size_t i = start; i < end && !foundFlag.load(); i++) {
            QString combination = idToCombination(i, charset, desiredLength);
            QString hash        = computeHash(combination);

            // If a match is found, store the result, empty the queue and set the found flag.
            if (hash == targetHash) {
                handleHashFound(manager, combination, foundFlag);
                return;
            }
        }
    }
}

QString BruteForceThread::computeHash(const QString& combination) {
    QCryptographicHash md5(QCryptographicHash::Md5);

    md5.reset();
    // md5.addData(salt.toLatin1());  // TODO: To be implemented
    md5.addData(combination.toLatin1());

    // Convert the hash result to a hex string and return.
    return md5.result().toHex();
}

QString BruteForceThread::idToCombination(size_t id, const QString& charset, size_t passwordLength) {
    QString result(passwordLength, Qt::Uninitialized);  // Preallocate string

    for (int pos = passwordLength - 1; pos >= 0; --pos) {
        result[pos] = charset.at(id % charset.size());
        id /= charset.size();
    }

    return result;
}

void BruteForceThread::handleHashFound(ThreadManager& manager, const QString& combination, std::atomic<bool>& foundFlag) {
    std::lock_guard<PcoMutex> lock(manager.resultMutex);
    manager.setFoundPassword(combination);
    foundFlag.store(true);
    manager.cancelWork();
}

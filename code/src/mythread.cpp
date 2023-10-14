#include "mythread.h"

#include <QCryptographicHash>


void BruteForceThread::run(ThreadManager::ThreadParameters params) {
    // Continue while the password hasn't been found.
    while (!params.flag.load()) {
        // Test each combination in the range.
        for (size_t i = params.start; i < params.end && !params.flag.load(); i++) {
            QString combination = idToCombination(i, params.charset, params.length);
            QString hash        = computeHash(combination, params.salt);

            // If a match is found, store the result, empty the queue and set the found flag.
            if (hash == params.hash) {
                handleHashFound(params.manager, combination, params.flag);
                return;
            }
        }
    }
}

QString BruteForceThread::computeHash(const QString& combination, const QString& salt) {
    QCryptographicHash md5(QCryptographicHash::Md5);

    md5.reset();
    md5.addData(salt.toLatin1());
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
    manager.setFoundPassword(combination);
    foundFlag.store(true);
    manager.cancelWork();
}

#include "mythread.h"

#include <QCryptographicHash>


void BruteForceThread::run(Parameters params) {
    size_t hashesComputed = 0;

    // Test each combination in the parametrized range while the preimage hasn't been found
    // by any thread.
    for (size_t i = params.rangeStart; i < params.rangeEnd; i++) {
        if (params.flag.load()) {
            return;  // Another thread found the preimage.
        }

        QString combination = idToCombination(i, params.charset, params.length);
        QString hash        = computeHash(combination, params.salt);

        // Call the progress callback every countForProgress hashes.
        if (++hashesComputed % params.countForProgress == 0) {
            params.progressCallback();
        }

        // If a match is found, store the result, empty the queue and set the found flag.
        if (hash == params.hash) {
            params.flag.store(true);
            params.passwordFoundCallback(combination);
            return;
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

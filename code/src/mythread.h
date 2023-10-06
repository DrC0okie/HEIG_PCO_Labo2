#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <pcosynchro/pcothread.h>

#include <atomic>
#include <QCryptographicHash>
#include <QString>
#include <queue>
#include "threadmanager.h"

class MyThread {
    public:
    // This function represents the work done by each thread.
    // Each thread picks up tasks (ranges) from the workQueue until either
    // the password is found or there's no more work left.
    static void bruteForceThread(ThreadManager& manager,
        const QString& charset, size_t desiredLength,
                                 const QString&     targetHash,
                                 std::atomic<bool>& foundFlag);

    private:
    static bool getNextWorkChunk(size_t& start, size_t& end);

    // Compute the MD5 hash of a given combination.
    static QString computeHash(const QString& combination, QCryptographicHash& md5);

    // Convert a number (id) to its corresponding combination in the charset.
    // This function effectively converts the number to a base-n representation,
    // where n is the size of the charset.
    static QString idToCombination(size_t id, const QString& charset,
                                   size_t passwordLength);
};

#endif  // MYTHREAD_H

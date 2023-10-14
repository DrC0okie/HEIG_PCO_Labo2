/**
 * \file mythread.h
 * \author Timothée Van Hove <timothe.vanhove@heig-vd.ch>
 * \author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * \date 08.10.2023
 * \brief MD5 brute forcing thread class.
 */

#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <atomic>
#include <functional>
#include <QString>

#include "threadmanager.h"

/**
 * \brief The BruteForceThread class
 * \details This class is used to reverse an md5 hash by brute force.
 */
class BruteForceThread {
    public:

    // TODO: add documentation
    struct Parameters {
        std::function<void(QString)>       passwordFoundCallback;
        std::function<void(size_t)>        progressCallback;
        QString            charset;
        QString            salt;
        QString            hash;
        unsigned int       length;
        std::atomic<bool>& flag;
        size_t             start;
        size_t             end;
        double             percentPerHash;
        size_t             countForProgress;
    };

    /**
     * \brief Run an MD5 brute force algorithm on a given range of combinations.
     * \param params The parameters for the brute force algorithm.
     * \details This function starts the brute force hacking process using multiple threads.
     * The function divides the work among the threads, waits for them to finish,
     * and then returns the found password if any.
     * \remark Passing parameters by copy might be slower than passing by reference but since
     * we do not create many new threads after this function is called, it is not a concern.
     */
    void run(Parameters params);

    private:
    /**
     * \brief Compute the MD5 hash of a given combination.
     * \param combination The combination to be hashed
     * \param salt The salt to be used
     * \return The MD5 hash of the combination
     */
    QString computeHash(const QString& combination, const QString& salt);

    /**
     * \brief Convert a number (id) to its corresponding combination in the charset.
     * \param id The number to be converted
     * \param charset A string containing all the characters that can be used to form a hash
     * \param passwordLength The length of the password
     * \return The combination corresponding to the given number
     */
    QString idToCombination(size_t id, const QString& charset, size_t passwordLength);
};

#endif  // MYTHREAD_H

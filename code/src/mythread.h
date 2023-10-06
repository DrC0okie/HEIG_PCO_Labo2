/**
  \file mythread.h
  \authors Timothée Van Hove <timothe.vanhove@heig-vd.ch>, Aubry Mangold <aubry.mangold@heig-vd.ch>
  \date 08.10.2023
  \brief Classe de brute-forcing de hash MD5.
*/

#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <atomic>
#include <QString>

#include "threadmanager.h"

class BruteForceThread {
    public:
    /**
     * \brief Run an MD5 brute force algorithm on a given range of combinations.
     * \param manager A reference to the ThreadManager object
     * \param charset A string containing all the characters that can be used to form a hash
     * \param desiredLength The length of the password
     * \param targetHash The hash to be reversed
     * \param foundFlag An atomic flag that indicates whether the password has been found
     */
    static void run(ThreadManager&     manager,
                    const QString&     charset,
                    size_t             desiredLength,
                    const QString&     targetHash,
                    std::atomic<bool>& foundFlag);

    private:
    /**
     * \brief Compute the MD5 hash of a given combination.
     * \param combination The combination to be hashed
     * \param md5 A reference to the QCryptographicHash object
     * \return The MD5 hash of the combination
     */
    static QString computeHash(const QString& combination);

    /**
     * \brief Convert a number (id) to its corresponding combination in the charset.
     * \param id The number to be converted
     * \param charset A string containing all the characters that can be used to form a hash
     * \param passwordLength The length of the password
     * \return The combination corresponding to the given number
     */
    static QString idToCombination(size_t id, const QString& charset, size_t passwordLength);

    /**
     * \brief Handle the case where the hash has been found.
     * \param manager The ThreadManager object
     * \param combination The combination that was found
     * \param foundFlag An atomic flag that indicates whether the password has been found
     */
    static void handleHashFound(ThreadManager& manager, const QString& combination, std::atomic<bool>& foundFlag);
};

#endif  // MYTHREAD_H

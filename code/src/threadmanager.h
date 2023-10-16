/**
 * \file threadmanager.h
 * \author Yann Thoma
 * \author Timothée Van Hove <timothe.vanhove@heig-vd.ch>
 * \author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * \date Created on 24.02.2017. Last modified on 14.10.2023.
 * \brief MD5 brute forcing thread manager class.
 * \details This class manages threads to reverse an md5 hash by brute force. Work
 * is split into chunks and each thread is assigned a chunk to work on. The
 * manager is responsible for creating, starting and joining the threads.
 */

#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <pcosynchro/pcothread.h>  // PcoThread

#include <memory>   // std::unique_ptr, std::make_unique
#include <QObject>  // QObject
#include <QString>  // QString
#include <queue>    // std::queue
#include <utility>  // std::pair
#include <vector>   // std::vector

#include "mythread.h"  // BruteForceThread::Parameters


/**
 * \brief The ThreadManager class
 * \details This class manages threads used to reverse an md5 hash by brute force.
 * \var workQueue The queue containing the ranges.
 * \var threadPool The pool of running threads.
 * \var threadCount The number of threads.
 * \var hashesToCompute The total number of hashes to compute.
 * \var countForProgress The number of hashes to compute before reporting progress.
 * \var foundPassword The found password.
 */
class ThreadManager : public QObject {
    public:
    /**
     * \brief The PcoThread container
     * \details This data structure is used to store the threads used by the class.
     */
    typedef std::vector<std::unique_ptr<PcoThread>> ThreadPool;

    /**
     * \brief The work queue
     * \details This data structure is used to store the work to be done by the threads.
     */
    typedef std::queue<std::pair<int, int>> WorkQueue;

    private:
    Q_OBJECT

    WorkQueue   workQueue;
    ThreadPool  threadPool;
    std::size_t threadCount;
    std::size_t hashesToCompute;
    std::size_t countForProgress;
    QString     foundPassword;

    public:
    /**
     * \brief ThreadManager Simple constructor
     * \param parent QObject parent
     */
    explicit ThreadManager(QObject *parent);

    /**
     * \brief Reset the instance to a clean state.
     */
    void resetInstance();

    /**
     * \brief startHacking Start the brute force attack.
     * \param charset The alphabet to be used for the attack
     * \param salt The salt to be used for the attack
     * \param hash The hash to be reversed
     * \param nbChars The length of the password
     * \param nbThreads The number of threads to be used
     * \return The hash preimage if found, an empty string otherwise
     *
     * \details This function starts the brute force hacking process. It splits the
     * work into chunks and starts the threads. It is called from the GUI class.
     */
    QString startHacking(
        QString      charset,
        QString      salt,
        QString      hash,
        unsigned int nbChars,
        unsigned int nbThreads);

    /**
     * \brief Setup the work queue by dividing the work into chunks.
     * \param combinations The total number of combinations
     */
    void setupWork(std::size_t combinations);

    /**
     * \brief Setup and start the threads.
     * \param params The parameters to be passed to the threads
     */
    void startWork(BruteForceThread::Parameters params);

    /**
     * \brief Joins all threads.
     */
    void joinThreads();

    /**
     * \brief Empties the work queue.
     */
    void cancelWork();

    /**
     * \brief Set the found password.
     * \param password QString The found password
     */
    void setFoundPassword(QString password);

    /**
     * \brief Increment the progress of the attack by a given factor.
     */
    void incrementProgress();

    signals:
    /**
     * \brief Signal to increment the progress bar by a given factor.
     * \param percentComputed double The factor by which to increment the progress bar
     */
    void sig_incrementPercentComputed(double percentComputed);
};

#endif  // THREADMANAGER_H

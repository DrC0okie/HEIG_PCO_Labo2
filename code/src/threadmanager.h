/**
 * \file threadmanager.h
 * \author Yann Thoma
 * \author Timothée Van Hove <timothe.vanhove@heig-vd.ch>
 * \author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * \date Created on 24.02.2017. Last modified on 14.10.2023.
 * \brief Classe pour reverser un hash md5 par brute force.
 * \details  Ce fichier contient la définition de la classe ThreadManager, qui permet de
 * reverser un hash md5 par brute force.
 */

#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <pcosynchro/pcomutex.h>
#include <pcosynchro/pcothread.h>

#include <QObject>
#include <QString>
#include <queue>

/**
 * \brief The Thread container
 * \details This data structure is used to store the threads.
 */
using ThreadPool = std::vector<std::unique_ptr<PcoThread>>;

/**
 * \brief The ThreadManager class
 * \details This class manages threads used to reverse an md5 hash by brute force.
 */
class ThreadManager : public QObject {
    Q_OBJECT

    private:
    ThreadPool                      threadPool;
    std::size_t                     totalChunks;
    double                          chunkProgressFactor;
    std::queue<std::pair<int, int>> workQueue;  // Thread-safe variant needed
    QString                         foundPassword;

    public:
    /**
     * \brief ThreadManager Simple constructor
     * \param parent QObject parent
     */
    ThreadManager(QObject* parent);

    /**
     * \brief startHacking tâche qui s'occupe de trouver le hash md5.
     * \param charset QString tous les caractères possibles composant le mot de
     * passe
     * \param salt QString sel qui permet de modifier dynamiquement le hash
     * \param hash QString hash à reverser
     * \param nbChars taille du mot de passe
     * \param nbThreads nombre de threads qui doivent reverser le hash
     * \return Le hash trouvé, ou une chaine vide sinon
     *
     * Cette fonction exécute réellement la recherche.
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
     * \param size The size of a chunk
     */
    void setupWork(size_t combinations, size_t size);

    /**
     * \brief Setup and start the threads.
     * \param params The parameters to be passed to the threads
     * \param count The number of threads to be created
     */
    void startWork(size_t count);

    /**
     * \brief Joins all threads.
     * \param pool The thread pool to join threads from
     */
    void joinThreads();

    /**
     * \brief Empties the work queue.
     */
    void cancelWork();

    /**
     * \brief Set the found password.
     * \param password QString mot de passe trouvé
     */
    void setFoundPassword(QString password);

    /**
     * \brief Increment the progress of the attack.
     * \param count The number of hashes that have been computed
     */
    void incrementProgress(size_t count);

    signals:
    /**
     * \brief Signal to increment the progress bar by a given factor.
     * \param percentComputed double The factor by which to increment the progress bar
     */
    void sig_incrementPercentComputed(double percentComputed);
};

#endif  // THREADMANAGER_H

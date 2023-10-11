/**
  \file threadmanager.h
  \author Yann Thoma
  \date 24.02.2017
  \brief Classe pour reverser un hash md5 par brute force.


  Ce fichier contient la définition de la classe ThreadManager, qui permet de
  reverser un hash md5 par brute force.
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
 * This data structure is used to store the threads.
 */
using ThreadPool = std::vector<std::unique_ptr<PcoThread>>;

/**
 * \brief The ThreadManager class
 *
 * This class manages threads used to reverse an md5 hash by brute force.
 */
class ThreadManager : public QObject {
    Q_OBJECT


    private:
    std::size_t                     totalChunks;
    double                          chunkProgressFactor;
    std::queue<std::pair<int, int>> workQueue;  // Thread-safe variant needed
    QString                         foundPassword;

    public:
    // Thread parameters
    struct ThreadParameters {
        ThreadManager&     manager;
        QString            charset;
        QString            salt;
        QString            hash;
        unsigned int       length;
        std::atomic<bool>& flag;
    };

    static PcoMutex queueMutex;   // Locks the access to the queue
    static PcoMutex resultMutex;  // Locks the access to the result string

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
    void startWork(ThreadPool& pool, ThreadParameters params, size_t count);

    /**
     * \brief Returns the next chunk of work to be done by a thread.
     * \param start The start index of the range
     * \param end The end index of the range
     * \return True if there is work to be done, false otherwise
     *
     * This is the critical section of the program because multiple threads can
     * ask for work at the same time.
     */
    bool getWork(size_t& start, size_t& end);

    /**
     * \brief Joins all threads.
     * \param pool The thread pool to join threads from
     */
    void joinThreads(ThreadPool& pool);

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
     * \brief Increment the progress bar by a given factor.
     * \param percentComputed double The factor by which to increment the progress bar
     */
    void incrementPercentComputed(double percentComputed);

    signals:
    /**
     * \brief Signal to increment the progress bar by a given factor.
     * \param percentComputed double The factor by which to increment the progress bar
     */
    void sig_incrementPercentComputed(double percentComputed);
};

#endif  // THREADMANAGER_H

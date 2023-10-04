#include <QCryptographicHash>

#include "mythread.h"

void hack(QString salt, QString hash) {
    QCryptographicHash md5(QCryptographicHash::Md5);

    // Récupérer mdp de la file d'attente



    md5.reset();
    /* On préfixe le mot de passe avec le sel */
    md5.addData(salt.toLatin1());
    //md5.addData(currentPasswordString.toLatin1());
    /* On calcul le hash */
    QString currentHash = md5.result().toHex();

    /*
     * Si on a trouvé, on retourne le mot de passe courant (sans le sel)
     * NECESISTE BOOL TERMINE
     */
    //if (currentHash == hash)
    //    return currentPasswordString;

    // TODO Aubry: terminer méthode
}

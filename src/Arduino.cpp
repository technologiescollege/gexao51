//-------------------------------------------------------------------------------
/**
 * @file        Arduino.cpp
 * @brief       Connexion, détection, lecture et écriture de l'Arduino
 *
 * @author      S.GUICHARD
 * @author      STS IRIS, Lycée Nicolas APPERT, ORVAULT (FRANCE)
 * @since       1/02/16
 * @version     1.0
 * @date        20/04/16
 *
 * Cette classe va permettre que lorsque la maquette est déconnectée il y a possibilité
 * de la reconnecter automatiquement. Elle va permettre également de communiquer avec l'Arduino.
 *
 * Fabrication  Gexao51.pro
 */

//=====   En-Têtes Personnels   =====
#include "Arduino.h"
#include "qextserialPort.h"
#include "qextserialenumerator.h"

//=====   En-Têtes standards    =====
#include <QDebug>
#include <QString>
#include <unistd.h>

//-------------------------------------------------------------------------------

/**
 * Constructeur par défaut.
 */
Arduino::Arduino():
    Semaphore()
{
    this->Semaphore = new QSemaphore (1);
}

//-------------------------------------------------------------------------------

/**
 * Déstructeur.
 */
Arduino::~Arduino()
{
    this->Port = NULL;
    this->Semaphore = NULL;
    this->Buffer = "";
    this->NomPort = "";
}

//-------------------------------------------------------------------------------

/**
 * Cette méthode va permettre de détecter si un Arduino Méga 2560 est connecté sur un port USB.
 * La méthode va également permettre l'ouverture du port afin de pouvoir communiquer avec le Shell Méga.
 * @code
 *     Ouvrir();
 * @endcode
 */
bool Arduino::Ouvrir()
{

    //Création de la variable de retour de la fonction
    bool bRetour (false);

    //Création de la variable qui permet de tester si un arduino a été détecté
    bool ArduinoPresent (false);

    //L'objet mentionnant les infos des ports série
    QextSerialEnumerator enumerateur;

    //on met ces infos dans une liste
    QList<QextPortInfo> Ports = enumerateur.getPorts();

    //On parcours la totalité des ports séries détectés
    for(int i = 0; i < Ports.length(); i++)
        {
            if(Ports[i].vendorID == 0x2341)    //Si un port avec un VID de 0x2341 l'Arduino est détecté
            {
                ArduinoPresent = true;  //La variable prend la valeur true si un arduino est détecté

                this->NomPort = Ports[i].portName;    //NomPort est complété par le numéro de port de l'Arduino

                //L'objet Port série
                this->Port = new QextSerialPort();

                //Quelques règlages pour que tout marche bien
                this->Port->setPortName    (this->NomPort);
                this->Port->setBaudRate    (BAUD9600);           //On règle la vitesse utilisée
                this->Port->setParity      (PAR_NONE);           //On règle la parité
                this->Port->setStopBits    (STOP_1);             //On règle le nombre de bits de stop
                this->Port->setDataBits    (DATA_8);             //On règle le nombre de bits de données
                this->Port->setFlowControl (FLOW_OFF);           //Pas de contrôle de flux

                //On ouvre la connexion avec l'arduino
                this->Port->open(QextSerialPort::ReadWrite);

                //On test si la connexion avec l'arduino à réussie
                if(this->Port->isOpen())
                {
                    bRetour = true;
                    qDebug() << "La connexion avec l'arduino à réussie";
                }
                else
                {
                    qDebug() << "La connexion avec l'arduino à échouée";
                }
            }
        }

        //Vérification de la détection d'un arduino
        if(ArduinoPresent == false)
        {
            qDebug() << "Aucun arduino n'a été détecté";
        }

    return bRetour;
}

//-------------------------------------------------------------------------------

/**
 * Cette méthode va permettre de fermer le port.
 * @code
 *     Fermer();
 * @endcode
 */
bool Arduino::Fermer()
{
    bool bRetour (false);

    //On ferme la connexion avec l'arduino
    this->Port->close();

    //On test si la fermeture de la connexion avec l'arduino à réussie
    if(!this->Port->isOpen())
    {
        bRetour = true;
        qDebug() << "La fermeture de la connexion avec l'arduino à réussie";
    }
    else
    {
        qDebug() << "La fermeture de la connexion avec l'arduino à échouée";
    }

    return bRetour;
}

//-------------------------------------------------------------------------------

/**
 * Cette méthode va permettre d'écrire sur le port série la commande reçu en paramètre.
 * @code
 *    EcrirePort("A10");
 * @endcode
 * @param Commande Commande à écrire sur le port permettant d'interroger le Shell Méga sur l'une de ses broches.
 */
void Arduino::EcrirePort(QString Commande)
{
    //Si le port série est ouvert
    if(this->Port->isOpen() == true)
    {
        this->Port->write(Commande.toStdString().c_str()); //On écrit sur le port la commande reçue en paramètre
    }
    //Sinon un message d'erreur est envoyé en sortie de compilation
    else
    {
        qDebug() << "L'écriture sur le port série à échouée";
    }
}

//-------------------------------------------------------------------------------

/**
 * Cette méthode va permettre de lire sur le port les données envoyées par le Shell Méga.
 * @code
 *    LirePort();
 * @endcode
 * @return QByteArray Buffer, retourne un buffer contenant les données lues sur le port.
 */
QByteArray Arduino::LirePort()
{
    //Si le port série est ouvert
    if(this->Port->isOpen() == true)
    {
        this->Buffer += this->Port->readAll(); //On remplie le Buffer
    }
    //Sinon un message d'erreur est envoyé en sortie de compilation
    else
    {
        qDebug() << "La lecture sur le port série à échouée";
    }

    return this->Buffer;
}

//-------------------------------------------------------------------------------

/**
 * Cette méthode va permettre d'interpréter les données reçu par le Shell Méga en récupérant la valeur du capteur.
 * @code
 *    LireCapteur("A10");
 * @endcode
 * @param Commande Commande à écrire sur le port permettant d'interroger le Shell Méga sur l'une de ses broches.
 * @return int Retour, retourne la valeur du capteur.
 */
int Arduino::LireCapteur(QString Commande)
{

    //Acquisition du sémaphore
    this->Semaphore->acquire(1);

    QByteArray RetourLecturePort("");
    QString DonneesLues("");
    int Retour(0);

    //On efface entièrement le Buffer pour une nouvelle lecture
    while(this->Buffer.isEmpty() != true)
    {
        this->Buffer.clear();
    }

    //Type d'entrée Jack ou I2C
    this->EcrirePort(Commande);

    //Lecture du port jusqu'à la fin de la ligne
    while(RetourLecturePort.right(1) != "\n" )
    {
        RetourLecturePort = this->LirePort(); //On récupère le Buffer de la fonction LirePort()
    }

    DonneesLues += RetourLecturePort;     //On copie le Buffer dans un QString

    //qDebug() << DonneesLues;

    DonneesLues = DonneesLues.remove(0,6);      //On supprime les 6 premiers caractères (VALUE=)
    DonneesLues.resize(DonneesLues.size()-2);   //On supprime les deux derniers caractères (\r\n)

    //Si l'entrée est de type jack, on renvoie la valeur maintenant
    //La valeur de retour est un entier
    Retour = DonneesLues.toInt(0,10);

    //Si l'entrée est de type I2C (si la commande commence par 'i'), la donnée à interpréter est sur deux octets,
    //il faut donc calculer la valeur décimale en fonction du poids fort et du poids faible
    if(Commande[0] == 'i')
    {
        QString CopieDonneesLues  ("");
        unsigned char PoidsFort   (0);
        unsigned char PoidsFaible (0);

        //On copie la variable DonneesLues
        CopieDonneesLues = DonneesLues;

        //On redimentionne la variable pour vérifier si elle commence par le caractère '-',
        //afin de savoir si une sonde à été détectée ou non
        DonneesLues.resize(1);

        //Retourne la valeur -1 si aucun capteur n'est détecté sur le BUS I2C à l'adresse passée en paramètre
        if(DonneesLues == "-")
        {
            Retour = (-1);
        }
        else
        {
            //Si le caractère n'est pas '-', alors il s'agit de l'octet de poids fort, qu'on met dans une variable
            PoidsFort = DonneesLues.toInt(0,10);

            //On utilise la variable de copie pour récupérer l'octet de poids faible
            //On supprime les deux premiers caractères (le caractère correspondant au poids fort et la virgule)
            CopieDonneesLues = CopieDonneesLues.remove(0,2);

            //On met le poids faible dans une variable
            PoidsFaible = CopieDonneesLues.toInt(0,10);

            //On renvoie la valeur décimale
            Retour = ((PoidsFort*256)+PoidsFaible);
        }
    }

    //On libère le sémaphore
    this->Semaphore->release(1);

    return Retour;
}

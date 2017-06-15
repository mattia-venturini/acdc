/*
 * StrategyCA.h
 *
 *  Created on: Jun 6, 2017
 *      Author: mattia
 */

#ifndef STRATEGYCA_H_
#define STRATEGYCA_H_


#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/simtime_t.h>
#include <random>

#define NCHAMPIONS 6

#define CHEATER 1       // valori di ritorno per isCheater
#define NOT_CHEATER -1
#define UNKNOWN 0

using namespace omnetpp;
using namespace std;

/*
 * Classa astratta che definisce i metodi con cui un leader cerca di individuare il look-ahead cheat
 */
class StrategyCA
{
    // costanti comuni a tutti i nodi
  public:
    simtime_t delayLimit = 10.0;
    simtime_t threshold = 1.0;   // soglia oltre cui un nodo è dichiarato cheater

    int index = 0;                 // numero di latenze ricevute dall'ultimo incremento di delay
    simtime_t averageLatency = 0;    // latenza media dei messaggi dal nodo sospetto, considerando gli ultimi NCHAMPIONS messaggi
    simtime_t oldSuspectedLatency = 0;

    // variabili utili al leader
    bool doCA = false;
    int suspectedNode = -1;
    int isCheater = UNKNOWN;
    simtime_t delay = 1.0;

  public:

    /*bool isDoingCA() { return doCA; }

    int getSuspectedNode() { return suspectedNode; }

    simtime_t getAverageLatency() { return averageLatency; }

    simtime_t getOldAverageLatency() { return oldSuspectedLatency; }

    simtime_t getThreshold() { return threshold; }*/

    /**
     * Cambio nodo da verificare
     */
    void setNewSuspect(int node)
    {
        suspectedNode = node;
        doCA = false;
        delay = 1.0;
    }

    /**
     * Definisce il cambiamento di ritardo da parte del leader
     */
    virtual void counterAttack() = 0;   // non implementato

    /**
     * Riceve un ritardo di trasmissione e lo memorizza nel modo utile alla Cheating Detection
     * @param delay: ritardo dell'ultimo messaggio
     */
    virtual void registerMsgDelay(simtime_t msgDelay) = 0;  // non implementato


    /**
     * Verifica se il nodo sospetto è etichettato come cheater in base ai dati a disposizione
     * @return 1 se è un cheater, -1 se non lo è, 0 se ancora non si può dire
     */
    int isCheaterDetected()
    {
        return isCheater;
    }
};



#endif /* STRATEGYCA_H_ */


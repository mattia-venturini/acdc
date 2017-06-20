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
  protected:
    int nChampions;         // numero di latenze da ricevere tra un contrattacco e l'altro
    int index = 0;          // numero di latenze ricevute dall'ultimo incremento di delay

  public:
    // variabili utili al leader
    bool doCA = false;
    int suspectedNode = -1;
    int isCheater = UNKNOWN;
    simtime_t delay = 1.0;

  public:
    /**
     * Cambio nodo da verificare
     * @param node: nuovo nodo
     */
    virtual void setNewSuspect(int node) = 0;   // non implementato

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


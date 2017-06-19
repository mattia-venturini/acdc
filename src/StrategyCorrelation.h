/*
 * StrategyRandom.h
 *
 *  Created on: Jun 16, 2017
 *      Author: mattia
 */

#ifndef STRATEGYRANDOM_H_
#define STRATEGYRANDOM_H_

#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/simtime_t.h>
#include <random>
#include <math.h>

using namespace omnetpp;
using namespace std;

/**
 * Strategia alternativa per ACDC
 * Si occupa di creare un ritardo casuale verso il nodo sospetto e verifica la correlazione dei suoi ritardi
 */
class StrategyCorrelation : public StrategyCA
{
  protected:
    //cRNG *random;       // per creare il ritardo casuale
    int repetitions;    // numero di messaggio che usa per calcolare la correlazione
    double minCorrelation = 0.5;   // soglia oltre cui un nodo è dichiarato cheater

    // dati di trasmissione e ricezione
    simtime_t *sent;
    simtime_t *rec;

  public:
    /**
     * COSTRUTTORE
     */
    StrategyCorrelation(cRNG *rng, int rep)
    {
        //random = new rng;
        repetitions = rep;

        sent = new simtime_t[repetitions];
        rec = new simtime_t[repetitions];

        doCA = true;
    }

    StrategyCorrelation(cRNG *rng, int rep, double minCorr)
    {
        minCorrelation = minCorr;
        repetitions = rep;

        sent = new simtime_t[repetitions];
        rec = new simtime_t[repetitions];

        doCA = true;
    }


    /**
     * Cambio nodo da verificare
     * @param node: nuovo nodo
     */
    virtual void setNewSuspect(int node)
    {
        suspectedNode = node;
        index = -1;

        isCheater = UNKNOWN;
        // NB: non serve re-inizializzare gli array perché andranno riempiti completamente
    }


    /**
     * Riceve un ritardo di trasmissione e lo memorizza nel modo utile alla Cheating Detection
     * @param delay: ritardo dell'ultimo messaggio
     *
     * @override
     */
    void registerMsgDelay(simtime_t msgDelay)
    {
        if(index > 0)   // ignora eventuali messaggi ricevuto prima del contrattacco
        {
            rec[index] = msgDelay;

            printf("x: %f\ty: %f\n", sent[index].dbl(), rec[index].dbl());
        }

        counterAttack();
    }



    /**
     * Definisce il cambiamento di ritardo da parte del leader
     * @override
     */
    virtual void counterAttack()
    {
        //double d = random->doubleRand();      // cambio il delay
        double d = (double)rand() / RAND_MAX;
        delay = d;

        index++;

        if(index < repetitions)
            sent[index] = delay;
        else
        {
            // THEN: calcolare correlazione
            double correlation = correlationIndex();

            if(abs(correlation) >= minCorrelation)
                // THEN: è etichettato come cheater
                isCheater = CHEATER;
            else
                // ELSE: è etichettato come non-cheater
                isCheater = NOT_CHEATER;
        }
    }


    /**
     * Calcola l'indice di correlazione dei ritardi
     */
    double correlationIndex()
    {
        // statistiche per determinare il cheater
        simtime_t avgSent = 0;      // media dei ritardi dati da ACDC
        simtime_t avgRec = 0;       // media dei ritardi dei msg ricevuti
        simtime_t stddevSent = 0;     // varianza dei ritardi dati da ACDC
        simtime_t stddevRec = 0;      // varianza dei ritardi dei msg ricevuti
        simtime_t covariance = 0;   // covarianza

        // calcolo delle medie dei ritardi
        for(int i = 0; i < repetitions; i++)
        {
            avgSent += sent[i];
            avgRec += rec[i];
        }
        avgSent /= repetitions;
        avgRec /= repetitions;

        // calcolo delle varianze dei ritardi
        for(int i = 0; i < repetitions; i++)
        {
            stddevSent += (sent[i] - avgSent).dbl() * (sent[i] - avgSent).dbl();
            stddevRec += (rec[i] - avgRec).dbl() * (rec[i] - avgRec).dbl();
            covariance += (sent[i] - avgSent).dbl() * (rec[i] - avgRec).dbl();
        }
        stddevSent /= repetitions;
        stddevRec /= repetitions;
        covariance /= repetitions;

        // scarti quadratici medi
        stddevSent = sqrt(stddevSent.dbl());
        stddevRec = sqrt(stddevRec.dbl());

        // indice di correlazione
        return (covariance/(stddevSent.dbl() * stddevRec.dbl())).dbl();

    }
};



#endif /* STRATEGYRANDOM_H_ */

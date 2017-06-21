/*
 * StrategyIncrease.h
 *
 *  Created on: Jun 15, 2017
 *      Author: mattia
 */

#ifndef STRATEGYINCREASE_H_
#define STRATEGYINCREASE_H_


/**
 * Strategia di contrattacco basilare per ACDC.
 * Valuta la latenza media dal nodo sospetto, dopodiché comincia a ritardare i messaggi diretti verso di esso,
 * per vedere se la latenza aumenta a sua volta.
 * Ad ogni passo calcola la latenza media e se è più alta della prima, oltre una certa soglia, lo segnala come cheater
 */
class StrategyIncrease : public StrategyCA
{
  protected:
    const double increaseFactor = 1.5;    // fattore moltiplicativo con cui si incrementa il delay

    simtime_t delayLimit;
    simtime_t threshold;   // soglia oltre cui un nodo è dichiarato cheater

    simtime_t averageLatency = 0;    // latenza media dei messaggi dal nodo sospetto, considerando gli ultimi NCHAMPIONS messaggi
    simtime_t oldSuspectedLatency = 0;

  public:
    /**
     * COSTRUTTORE
     */
    StrategyIncrease(simtime_t CAlimit, simtime_t gamma)
    {
        delayLimit = CAlimit;
        threshold = gamma;

        nChampions = 6;
    }

    /**
     * Cambio nodo da verificare
     * @param node: nuovo nodo
     */
    virtual void setNewSuspect(int node)
    {
        suspectedNode = node;
        doCA = false;
        delay = 0.1;

        // inizializzo le latenze per il nodo sospetto, così da vederne il cambiamento
        isCheater = UNKNOWN;
        oldSuspectedLatency = 0;
        averageLatency = 0;
        index = 0;
    }

    /**
     * Riceve un ritardo di trasmissione e lo memorizza nel modo utile alla Cheating Detection
     * @param delay: ritardo dell'ultimo messaggio
     *
     * @override
     */
    void registerMsgDelay(simtime_t msgDelay)
    {
        // aggiungo la nuova (divisa per NCHAMPIONS per essere pronta a far parte della media)
        msgDelay /= nChampions;

        if(doCA)
        {
            // aggiorno la stima della latenza media sul canale
            averageLatency += msgDelay;
        }
        else
        {
            // aggiorno la stima della latenza media sul canale
            oldSuspectedLatency += msgDelay;
        }

        index++; // incremento index per il prossimo messaggio

        if(index == nChampions) // se ho fatto il giro del vettore (e sono il leader)
        {
            //printf("----> Ritardo medio dal nodo sospetto: %f (old: %f, threshold: %f). \n", averageLatency.dbl(),
            //        oldSuspectedLatency.dbl(), threshold.dbl());

            if(doCA)
            {
                // verifico se il ritardo medio è aumentato troppo
                if(averageLatency >= oldSuspectedLatency + threshold)
                    // THEN: il nodo è etichettato come CHEATER
                    isCheater = CHEATER;
                else
                    // ELSE: non ho prove per dire che è un cheater: incremento il delay
                    counterAttack();
            }
            else
                doCA = true;    // ho la latenza media, posso iniziare ad eseguire ACDC

            index = 0;
        }
    }



    /**
     * Definisce il cambiamento di ritardo da parte del leader
     * @override
     */
    virtual void counterAttack()
    {
        delay *= increaseFactor;      // aumento il delay
        averageLatency = 0;     // resetto la valutazione della latenza media

        //DEBUG
        //printf("ritardo del leader: %f\n", delay.dbl());

        if(delay >= delayLimit) // se ho superato una certa soglia di delay
        {
            // THEN: ritengo che il nodo non è un cheater
            isCheater = NOT_CHEATER;
        }
    }

};


#endif /* STRATEGYINCREASE_H_ */

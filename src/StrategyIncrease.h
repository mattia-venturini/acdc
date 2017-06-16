/*
 * StrategyIncrease.h
 *
 *  Created on: Jun 15, 2017
 *      Author: mattia
 */

#ifndef STRATEGYINCREASE_H_
#define STRATEGYINCREASE_H_


/**
 * Strategia di contrattacco basilare per ACDC
 */
class StrategyIncrease : public StrategyCA
{
  protected:
    simtime_t delayLimit = 10.0;
    simtime_t threshold = 1.0;   // soglia oltre cui un nodo è dichiarato cheater

    simtime_t averageLatency = 0;    // latenza media dei messaggi dal nodo sospetto, considerando gli ultimi NCHAMPIONS messaggi
    simtime_t oldSuspectedLatency = 0;

  public:
    /**
     * COSTRUTTORE
     */
    StrategyIncrease()
    { }

    StrategyIncrease(simtime_t CAlimit, simtime_t gamma)
    {
        delayLimit = CAlimit;
        threshold = gamma;
    }

    /**
     * Cambio nodo da verificare
     * @param node: nuovo nodo
     */
    virtual void setNewSuspect(int node)
    {
        suspectedNode = node;
        doCA = false;
        delay = 1.0;

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
        msgDelay /= NCHAMPIONS;

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

        if(index == NCHAMPIONS) // se ho fatto il giro del vettore (e sono il leader)
        {
            printf("%Ritardo medio dal nodo sospetto: %f (old: %f, threshold: %f). \n", msgDelay.dbl(),
                    oldSuspectedLatency.dbl(), threshold.dbl());

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
        delay *= 1.02;      // aumento il delay
        averageLatency = 0;     // resetto la valutazione della latenza media

        //DEBUG
        printf("ritardo del leader: %f\n", delay.dbl());

        if(delay >= delayLimit) // se ho superato una certa soglia di delay
        {
            // THEN: ritengo che il nodo non è un cheater
            isCheater = NOT_CHEATER;
        }
    }

};


#endif /* STRATEGYINCREASE_H_ */

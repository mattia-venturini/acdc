/*
 * StrategyIncrease.h
 *
 *  Created on: Jun 15, 2017
 *      Author: mattia
 */

#ifndef STRATEGYINCREASE_H_
#define STRATEGYINCREASE_H_


class StrategyIncrease : public StrategyCA
{
  public:

    /**
     * COSTRUTTORE
     */
    StrategyIncrease()
    { }

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
            averageLatency = averageLatency + msgDelay;
        }
        else
        {
            // aggiorno la stima della latenza media sul canale
            oldSuspectedLatency = averageLatency + msgDelay;
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
        delay *= 1.02;

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

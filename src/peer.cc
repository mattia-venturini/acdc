//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "peer.h"

using namespace std;

namespace acdc {

/* Modulo: Peer
 *
 * Rappresenta un nodo in una rete P2P. Genera continuamente messaggi in un tempo casuale e li manda in broadcast.
 * Per gli utlimi NCHAMPIONS messaggi ricevuti memorizza il ritardo di trasmissione (servendosi del timestamp), per ogni altro nodo.
 *
 * Se è il leader della rete, controlla se un certo nodo sta effettuando un Look-ahead cheat, servendosi del metodo ACDC.
 * In caso affermativo chiede di verificare anche ad un altro nodo (che diventerà il nuovo leader), in caso negativo verifica un altro nodo.
 * Comunque cede il privilegio di leader dopo un certo timeout.
 *
 * Se è un cheater, memorizza per un certo tempo i timestamp degli avversari, ed effettua la mossa ponendone uno inferiore ad essi.
 */

Define_Module(Peer);


// inizializza
void Peer::initialize()
{
    nPeers = gateSize("gate");     // numero di peers = numero di collegamenti (rete fortemente connessa)

    latencies = new simtime_t[NCHAMPIONS];    // latenze degli ultimi NCHAMPIONS messaggi
    index = 0;              // prima posizione
    averageLatency = 0;     // latenza media messaggi dai peers
    oldSuspectedLatency = 0;    // latenza precendente al contrattacco

    // inizializzo ogni elemento del campione
    for(int j = 0; j < NCHAMPIONS; j++)
    {
        latencies[j] = 0;
        diffVector.record(0);
    }

    if(strcmp(getName(),"peer0") == 0)  // il primo peer parte come leader
    {
        leader = true;
        doCA = false;    // non effettuare ACDC finché non ho abbastanza dati sulle latenze
        suspectedNode = 0;

        // imposta la scadenza del tempo come leader
        cMessage *timeout = new cMessage();
        timeout->setKind(TIMEOUT_LEADER);
        scheduleAt(simTime()+TIME_LEADER, timeout);

        // counterAttack();
    }
    else if(par("cheater"))
    {
        cheatedMove();
    }

    scheduleNextMessage();
}


// gestisce un messaggio in ingresso
void Peer::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage())    // inviato da sé stesso per schedulare un evento futuro
    {
        if(msg->getKind() == TIMEOUT_LEADER)   // timeout del leader
        {
            leader = false;
            delay = 1;

            // invio il token ad un altro nodo casuale
            cMessage *token = new cMessage();
            token->setKind(TOKEN_LEADER);

            int nextLeader = random->intRand() % nPeers;
            send(token, "gate$o", nextLeader);

            printf("%f: Token riasciato per timeout. \n", simTime().dbl());

            // avviso tutti che il TOKEN_LEADER è stato rilasciato
            cMessage *infoMsg = new cMessage();
            infoMsg->setKind(INFO_TOKEN_RELEASED);
            sendToAll(infoMsg);
        }
        else if(msg->getKind() == MSG_ACDC && leader) // messaggio in ritardo per il nodo sospetto
        {
            // NB: potrebbe aver ceduto il TOKEN_LEADER nel frattempo, quindi verifico se sono ancora leader
            send(msg->dup(), "gate$o", suspectedNode);
        }
        else if(par("cheater") && msg->getKind() == MSG_CHEATEDMOVE)    // mossa del cheater
        {
            cheatedMove();
        }
        else if(msg->getKind() == MSG_TYPE_A)    // spedizione di nuovo messaggio comune
        {
            msg->setTimestamp();

            sendToAll(msg);             // invia a tutti i nodi

            scheduleNextMessage();      // avvia generazione casuale del prossimo evento
        }
    }
    // messaggio ricevuto da un altro nodo
    else if(msg->getKind() == TOKEN_LEADER) // token che mi rende il leader
    {
        printf("%s è il nuovo LEADER!\n", getName());

        leader = true;
        doCA = false;
        suspectedNode = 0;  // TODO: ricevere un sospetto da verificare

        // inizializzo le latenze per il nodo sospetto, così da vederne il cambiamento
        oldSuspectedLatency = 0;
        averageLatency = 0;

        // imposta la scadenza del tempo come leader
        cMessage *timeout = new cMessage();
        timeout->setKind(TIMEOUT_LEADER);
        scheduleAt(simTime()+TIME_LEADER, timeout);

    }
    else    // messaggio generico
    {
        int numGate = msg->getArrivalGate()->getIndex();

        // il leader processa i messaggi del nodo sospetto
        if(leader && numGate == suspectedNode)
        {
            checkLatency(msg, numGate);
        }

        // se è un cheater: memorizza il timestamp minimo ed attende di fare la mossa
        if(par("cheater"))
        {
            if(msg->getTimestamp() < minTimestamp || minTimestamp == 0)
                minTimestamp = msg->getTimestamp();
        }
    }
    delete msg;
}


// schedula la generazione del prossimo messaggio
void Peer::scheduleNextMessage()
{
    if(!par("cheater"))
    {
        cMessage *nextMsg = new cMessage();
        nextMsg->setKind(MSG_TYPE_A);

        scheduleAt(simTime()+par("generationTime"), nextMsg);
    }
}


// invia un messaggio in broadcast a tutti i nodi, tranne all'eventuale nodo sospetto
void Peer::sendToAll(cMessage *msg)
{
    for(int j=0; j<gateSize("gate"); j++)
    {
        if(leader && suspectedNode == j && doCA)    // se è leader: messaggio in ritardo per il nodo sospetto
        {
            cMessage *msgACDC = msg->dup();
            msgACDC->setKind(MSG_ACDC);
            scheduleAt(simTime()+delay, msgACDC);
        }
        else    // messaggio per tutti gli altri nodi
        {
            cMessage *copy = msg->dup();
            send(copy, "gate$o", j); // invio il messaggio effettivo, a tutti i nodi
        }
    }
}


// incrementa il ritardo al nodo sospetto
void Peer::counterAttack(cMessage *msg)
{
    delay *= 1.02;
    //if(delay > 60)
    printf("ritardo del leader: %f\n", delay.dbl()); // errore: raggiunge valori troppo alti, farlo fermare
}


// cheater: invia la propria mossa con timestamp inferiore al più piccolo dei messaggi ricevuti nell'ultimo intervallo
void Peer::cheatedMove()
{
    cMessage *move = new cMessage();

    if(minTimestamp > 1.0)      // evita di impostare un timestamp negativo
        move->setTimestamp(minTimestamp -1);    // mossa cheated: timestamp immediatamente inferiore
    else
        move->setTimestamp();   // nessun messaggio ricevuto: mossa normale

    sendToAll(move);

    cMessage *endInterval = new cMessage();
    endInterval->setKind(MSG_CHEATEDMOVE);
    scheduleAt(simTime()+intervalS, endInterval);
}

/*
 * Calcola la latenza di un messaggio, la stima di latenza media e verifica se è un (probabile) cheater
 */
void Peer::checkLatency(cMessage *msg, int numGate)
{
    // calcolo latenza del messaggio
    simtime_t msgDelay = simTime() - msg->getTimestamp();

    diffVector.record(msgDelay);  // raccolta dati per statistiche

    // aggiungo la nuova (divisa per NCHAMPIONS per essere pronta a far parte della media)
    msgDelay /= NCHAMPIONS;
    if(doCA)
    {
        // aggiorno la stima della latenza media sul canale
        oldSuspectedLatency = averageLatency + msgDelay;
    }
    else
    {
        // aggiorno la stima della latenza media sul canale
        averageLatency = averageLatency + msgDelay;
    }

    index++; // incremento index per il prossimo messaggio

    if(index == NCHAMPIONS) // se ho fatto il giro del vettore (e sono il leader)
    {
        printf("%s: ritardo medio dal nodo sospetto %d: %f\n", getName(), numGate, msgDelay.dbl());

        if(doCA)
        {
            // verifico se il ritardo medio è aumentato troppo
            if(averageLatency >= oldSuspectedLatency + threshold)
            {
                // THEN: il nodo è etichettato come CHEATER

                printf("%s: %d è un cheater! averageLatency: %f (old: %f, threshold: %f). \n", getName(), numGate, msgDelay.dbl(),
                        oldSuspectedLatency.dbl(), threshold.dbl());

                // rilascia il TOKEN_LEADER ad un terzo nodo
                cMessage *token = new cMessage();
                token->setKind(TOKEN_LEADER);
                int nextLeader;
                do
                {   // selezione casuale tra i nodi rimanenti
                    nextLeader = random->intRand() % nPeers;
                } while(nextLeader != numGate);

                send(token, "gate$o", nextLeader);

                leader = false;
                delay = 1.0;
                suspectedNode = -1;

                // DEBUG
                printf("%s: TOKEN_LEADER rilasciato. \n", getName());

                // avviso tutti che il TOKEN_LEADER è stato rilasciato
                cMessage *infoMsg = new cMessage();
                infoMsg->setKind(INFO_TOKEN_RELEASED);
                sendToAll(infoMsg);
            }
            else    // non ho prove per dire che è un cheater: incremento il delay
                counterAttack(msg);    // ma al nodo sospetto lo schedula per mandarlo il ritardo
        }
        else
            doCA = true;    // ho la latenza media, posso iniziare ad eseguire ACDC

        index = 0;
    }
}

} // end namespace acdc

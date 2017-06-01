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

Define_Module(Peer);


// inizializza
void Peer::initialize()
{
    int n_peers = gateSize("gate");     // numero di peers = numero di collegamenti (rete fortemente connessa)
    diff = new simtime_t[n_peers];      // latenza media messaggi dai peers
    diffVector = new cOutVector[n_peers]; // per vedere statistiche

    for(int j=0; j<n_peers; j++)
    {
        diff[j] = 0;
        diffVector[j].record(0);
    }

    if(strcmp(getName(),"peer0") == 0)  // il primo peer parte come leader
    {
        leader = true;
        suspectedNode = 0;

        // imposta la scadenza del tempo come leader
        cMessage *timeout = new cMessage();
        timeout->setKind(TIMEOUT_LEADER);
        scheduleAt(simTime()+TLeader, timeout);

        //startCounterAttack();
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
            cMessage *token = new cMessage();
            token->setKind(TOKEN_LEADER);
            send(token, "gate$o", 0);
        }
        else if(msg->getKind() == MSG_ACDC) // messaggio in ritardo per il nodo sospetto
        {
            send(msg->dup(), "gate$o", suspectedNode);
        }
        else if(par("cheater") && msg->getKind() == MSG_CHEATEDMOVE)    // mossa del cheater
        {
            cheatedMove();
        }
        else       // nuovo messaggio comune
        {
            msg->setTimestamp();

            sendToAll(msg);             // invia a tutti i nodi
            if(leader)
                startCounterAttack(msg);    // ma al nodo sospetto lo schedula per mandarlo il ritardo

            scheduleNextMessage();      // avvia generazione causale del prossimo evento
        }
    }
    else if(msg->getKind() == TOKEN_LEADER) // token che mi rende il leader
    {
        leader = true;
        suspectedNode = 0;

        // imposta la scadenza del tempo come leader
        cMessage *timeout = new cMessage();
        timeout->setKind(TIMEOUT_LEADER);
        scheduleAt(simTime()+TLeader, timeout);

        //startCounterAttack();
    }
    else    // messaggio generico
    {
        // aggiorno la stima della latenza media sul canale
        int numGate = msg->getArrivalGate()->getIndex();
        simtime_t delay = simTime() - msg->getTimestamp();
        diff[numGate] = (diff[numGate]*(NChampions-1)/NChampions) + (delay/NChampions);
        diffVector[numGate].record(diff[numGate]);  // raccolta dati per statistiche

        if(leader && diff[numGate] >= threshold)  // il nodo è etichettato come cheater
        {
            printf("%d è un cheater! Ci mette %f secondi. \n", numGate, delay.dbl());
            this->delay = 1.0;
        }

        if(par("cheater")) // se è un cheater: memorizza il timestamp minimo ed attende di fare la mossa
        {
            if(msg->getTimestamp() < minTimestamp || minTimestamp == 0)
                minTimestamp = msg->getTimestamp();

            /*cMessage *response = new cMessage();
            response->setTimestamp(msg->getTimestamp() - 1);    // imposta un timestamp inferiore

            sendToAll(response);*/
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
        if(!leader || suspectedNode != j)   // il leader ignora il nodo sospetto (lo invierà in ritardo)
        {
            cMessage *copy = msg->dup();
            send(copy, "gate$o", j); // invio il messaggio effettivo, a tutti i nodi
        }
    }
}


// inizia ad inviare messaggi ritardati al nodo sospetto
void Peer::startCounterAttack(cMessage *msg)
{
    msg->setKind(MSG_ACDC);
    scheduleAt(simTime()+delay, msg->dup());

    delay *= 1.02;
    //if(delay > 60)
    //printf("delay: %f\n", delay.dbl()); // errore: raggiunge valori troppo alti, farlo fermare
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

} // end namespace acdc

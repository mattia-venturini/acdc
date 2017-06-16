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
    nLinks = gateSize("gate");      // numero di peers = numero di collegamenti (rete fortemente connessa)
    nActivePeers = nLinks+1;        // = nodi connessi + sé stesso

    idPeers = new int[nLinks];
    activeLink = new bool[nLinks];

    references = 0;

    timeoutLeader = getSystemModule()->par("timeoutLeader");

    // selezione parametrica della strategia da usare
    if(strcmp(getSystemModule()->par("strategy"), "increase") == 0)     // StrategyIncrease
    {
        simtime_t delayLimit = getSystemModule()->par("delayLimit");
        simtime_t threshold = getSystemModule()->par("threshold");
        strategy = new StrategyIncrease(delayLimit, threshold);
    }
    else if(strcmp(getSystemModule()->par("strategy"), "correlation") == 0)     // StrategyCorrelation
    {
        int repetitions = getSystemModule()->par("repetitions");
        double minCorrelation = getSystemModule()->par("minCorrelation");
        strategy = new StrategyCorrelation(this->getRNG(0), repetitions, minCorrelation);
    }

    // inizializza vettori
    for(int i = 0; i < nLinks; i++)
    {
        // costruisce il vettore di id dei peers
        cModule *suspModule = gate("gate$o", i)->getPathEndGate()->getOwnerModule();
        int id = suspModule->getId();
        idPeers[i] = id;

        // costruisce il vettore activeLink
        activeLink[i] = true;
    }

    if(strcmp(getName(),"peer0") == 0)  // il primo peer parte come leader
    {
        leader = true;
        // per vedere graficamente il cambiamento
        getDisplayString().setTagArg("b",3,"blue");

        strategy->setNewSuspect(setSuspectNode());  // comincia a verificare un nodo

        // imposta la scadenza del tempo come leader
        cMessage *timeout = new cMessage();
        timeout->setKind(TIMEOUT_LEADER);
        scheduleAt(simTime()+TIME_LEADER, timeout);
    }
    //else
    //    getDisplayString().setTagArg("b",3,"white");

    if(par("cheater"))
    {
        cheatedMove();
    }

    scheduleNextMessage();
}


// gestisce un messaggio in ingresso
void Peer::handleMessage(cMessage *msg)
{
    // inviato da sé stesso per schedulare un evento futuro
    if(msg->isSelfMessage())
    {
        if(msg->getKind() == TIMEOUT_LEADER && leader)   // timeout del leader
        {
            // NB: potrebbe aver ceduto il TOKEN_LEADER nel frattempo, quindi verifico se sono ancora leader

            leader = false;
            // per vedere graficamente che non sono più leader
            getDisplayString().setTagArg("b",3,"white");

            // invio il token ad un altro nodo casuale
            cMessage *token = new cMessage();
            token->setKind(TOKEN_LEADER);

            if(nActivePeers > 1)    // se non è l'ultimo nodo rimasto
            {
                int nextLeader;
                do
                {   // selezione casuale tra i nodi rimanenti
                    nextLeader = random->intRand(nLinks);
                } while(!activeLink[nextLeader]); // se ho preso il cheater riprovo

                send(token, "gate$o", nextLeader);

                printf("%f: Token riasciato per timeout. \n", simTime().dbl());

                // avviso tutti che il TOKEN_LEADER è stato rilasciato
                cMessage *infoMsg = new cMessage();
                infoMsg->setKind(INFO_TOKEN_RELEASED);
                infoMsg->setTimestamp();
                sendToAll(infoMsg);
            }
        }
        else if(msg->getKind() == MSG_ACDC && leader && strategy->suspectedNode >= 0) // messaggio in ritardo per il nodo sospetto
        {
            // NB: potrebbe aver ceduto il TOKEN_LEADER nel frattempo, quindi verifico se sono ancora leader
            // NB: potrebbero non esserci sospetti (unico nodo attivo)
            send(msg->dup(), "gate$o", strategy->suspectedNode);
        }
        else if(par("cheater") && msg->getKind() == MSG_CHEATEDMOVE)    // mossa del cheater
        {
            cheatedMove();
        }
        else if(msg->getKind() == MSG_COMMON)    // spedizione di nuovo messaggio comune
        {
            msg->setTimestamp();

            sendToAll(msg);             // invia a tutti i nodi

            scheduleNextMessage();      // avvia generazione casuale del prossimo evento
        }
    }

    // messaggio ricevuto da un altro nodo (attivo)
    else if(activeLink[msg->getArrivalGate()->getIndex()])
    {
        // token che mi rende il leader
        if(msg->getKind() == TOKEN_LEADER)
        {
            printf("%s è il nuovo LEADER!\n", getName());

            leader = true;
            // per vedere graficamente il cambiamento
            getDisplayString().setTagArg("b",3,"blue");


            // ricevere l'id del sospetto da verificare
            if(msg->hasPar("suspectedNode"))
            {
                int suspId = msg->par("suspectedNode").longValue();

                // cerca l'id nell'array, per sapere a quale uscita del gate è collegato
                for(int i = 0; i < nLinks; i++)
                {
                    if (suspId == idPeers[i])
                    {
                        if(strategy->suspectedNode == i)    // nodo già verificato da me
                        {
                            // ri-passa il token
                            leader = false;
                            // per vedere graficamente il cambiamento
                            getDisplayString().setTagArg("b",3,"white");

                            int nextLeader;
                            do
                            {   // selezione casuale tra i nodi rimanenti
                                nextLeader = random->intRand(nLinks);
                            } while(nextLeader == i || !activeLink[nextLeader]); // se ho preso il cheater riprovo

                            send(msg->dup(), "gate$o", nextLeader);
                        }
                        else
                        {
                            // ricavo il numero di references
                            references = msg->par("references").longValue();

                            strategy->setNewSuspect(i);     // inizio a verificarlo
                        }
                        break;
                    }
                }
            }
            else
            {
                // ELSE: non ci sono nodi già sospettati, ne prendo uno nuovo
                strategy->setNewSuspect(setSuspectNode());
            }

            // imposta la scadenza del tempo come leader
            cMessage *timeout = new cMessage();
            timeout->setKind(TIMEOUT_LEADER);
            scheduleAt(simTime()+TIME_LEADER, timeout);

        }
        // la maggior parte dei peer ha identificato un cheater
        else if(msg->getKind() == INFO_CHEATER_DETECTED)
        {
            // ricevo l'id del cheater
            int suspId = msg->par("suspectedNode").longValue();

            // individuo la porta a cui è collegato e la disattivo
            for(int i = 0; i < nLinks; i++)
                if (suspId == idPeers[i])
                    activeLink[i] = false;

            references = 0;
            nActivePeers--;
        }
        // messaggio generico
        else
        {
            int numGate = msg->getArrivalGate()->getIndex();

            // il leader processa i messaggi del nodo sospetto
            if(leader && numGate == strategy->suspectedNode)
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
    }


    delete msg;
}


// schedula la generazione del prossimo messaggio
void Peer::scheduleNextMessage()
{
    if(!par("cheater"))
    {
        cMessage *nextMsg = new cMessage();
        nextMsg->setKind(MSG_COMMON);

        scheduleAt(simTime()+par("generationTime"), nextMsg);
    }
}


/**
 * Invia un messaggio in broadcast a tutti i nodi attivi, tranne all'eventuale nodo sospetto
 */
void Peer::sendToAll(cMessage *msg)
{
    for(int j=0; j<gateSize("gate"); j++)
    {
        // se è leader: messaggio in ritardo per il nodo sospetto
        if(leader && strategy->suspectedNode == j && strategy->doCA)
        {
            cMessage *msgACDC = msg->dup();
            msgACDC->setKind(MSG_ACDC);
            scheduleAt(simTime()+strategy->delay, msgACDC);
        }
        else if(activeLink[j])    // messaggio per tutti gli altri nodi (attivi)
        {
            cMessage *copy = msg->dup();
            send(copy, "gate$o", j); // invio il messaggio effettivo, a tutti i nodi
        }
    }
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

    strategy->registerMsgDelay(msgDelay);    // aggiunge la latenza ai propri dati

    // verifica se il nodo risulta un cheater
    if(strategy->isCheaterDetected() == CHEATER)
    {
        // THEN: nodo etichettati come cheater

        // ricavo, a partire da numGate, l'ID del nodo a cui sono collegato
        int suspId = idPeers[numGate];

        references++;               // un sospetto in più = nodo corrente

        // verifico se ho raggiunto la maggioranza
        if(references >= nActivePeers/2)
        {
            // THEN: escludo il nodo dalla rete

            printf("CHEATER INDIVIDUATO: %d!\n", suspId);


            activeLink[strategy->suspectedNode] = false;    // setto il collegamento come inattivo

            // evidenzia graficamente che un nodo è stato escluso
            cModule *suspModule = gate("gate$o", numGate)->getPathEndGate()->getOwnerModule();
            suspModule->getDisplayString().setTagArg("b",3,"red");

            references = 0;
            nActivePeers--;

            // Segnalo a tutti i nodi la presenza del cheater

            cMessage *infoMsg = new cMessage();
            infoMsg->setKind(INFO_CHEATER_DETECTED);

            // aggiunge l'ID del nodo sospetto come parametro del token
            cMsgPar *paramSusp = new cMsgPar("suspectedNode");
            paramSusp->setLongValue(suspId);
            infoMsg->addPar(paramSusp);
            infoMsg->setTimestamp();

            sendToAll(infoMsg);

            // imposto un nuovo sospetto
            strategy->setNewSuspect(setSuspectNode());
        }
        else
        {
            // ELSE: chiedo ad altri nodi di verificarlo

            // rilascia il TOKEN_LEADER ad un terzo nodo (tra quelli attivi)
            cMessage *token = new cMessage();
            token->setKind(TOKEN_LEADER);

            int nextLeader;
            do
            {   // selezione casuale tra i nodi rimanenti
                nextLeader = random->intRand(nLinks);
            } while(nextLeader == numGate || !activeLink[nextLeader]); // se ho preso un cheater riprovo

            // aggiunge l'ID del nodo sospetto come parametro del token
            cMsgPar *paramSusp = new cMsgPar("suspectedNode");
            paramSusp->setLongValue(suspId);
            token->addPar(paramSusp);

            // aggiungo il numero di peer che sospetta il nodo
            cMsgPar *paramRefs = new cMsgPar("references");
            paramRefs->setLongValue(references);
            token->addPar(paramRefs);

            send(token, "gate$o", nextLeader);


            leader = false;
            // per vedere graficamente il cambiamento
            getDisplayString().setTagArg("b",3,"white");

            // DEBUG
            printf("%s: TOKEN_LEADER rilasciato. \n", getName());

            // avviso tutti che il TOKEN_LEADER è stato rilasciato
            cMessage *infoMsg = new cMessage();
            infoMsg->setKind(INFO_TOKEN_RELEASED);
            infoMsg->setTimestamp();
            sendToAll(infoMsg);

        }

    }
    else if(strategy->isCheaterDetected() == NOT_CHEATER)
    {
        strategy->setNewSuspect(setSuspectNode());
    }
}


/**
 * Imposta un nuovo nodo sospetto che non sia stato disattivato
 */
int Peer::setSuspectNode()
{
    if(nActivePeers == 1)   // nessun altro nodo oltre a me
        return -1;          // errore

    // cambio nodo da verificare
    int suspectedNode = strategy->suspectedNode;

    // imposto un nuovo sospetto
    do
        suspectedNode = (suspectedNode + 1) % nLinks;
    while(!activeLink[suspectedNode]);  // verifico che non sia un nodo già disattivo

    return suspectedNode;
}


} // end namespace acdc

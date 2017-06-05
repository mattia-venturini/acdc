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

#ifndef __ACDC_PEER_H_
#define __ACDC_PEER_H_

#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/simtime_t.h>
#include <random>

#define ev   (*cSimulation::getActiveEnvir()) // per mandare output di testo al posto di printf

#define MSG_INFO 0
#define MSG_TYPE_A 1
#define MSG_ACDC 10
#define MSG_CHEATEDMOVE 11
#define TOKEN_LEADER 42
#define TIMEOUT_LEADER 43
#define INFO_TOKEN_RELEASED 44

#define NCHAMPIONS 6
#define TIME_LEADER 40.0

using namespace omnetpp;
using namespace std;

namespace acdc {

/**
 * Generated class
 */
class Peer : public cSimpleModule
{
    // costanti comuni a tutti i nodi
  private:
    cRNG* random = this->getRNG(0);       // RNG preso da omnet, con seme fissato e quindi riproducibile
    simtime_t delayLimit = 10.0;
    simtime_t threshold = 1.0;   // soglia oltre cui un nodo è dichiarato cheater

  protected:
    int nPeers;     // numero di Peer a cui è connesso
    int *idPeers;    // memorizza per ogni gate l'id del Peer a cui connette

    simtime_t *latencies;      // latenze degli ultimi NCHAMPIONS messaggi giunti dal nodo sospetto
    int index;                 // posizione in latencies dell'ultimo messaggio ricevuto
    simtime_t averageLatency;    // latenza media dei messaggi dal nodo sospetto, considerando gli ultimi NCHAMPIONS messaggi
    simtime_t oldSuspectedLatency;

    // variabili utili al leader
    bool leader = false;
    bool doCA = false;
    int suspectedNode = -1;
    simtime_t delay = 1.0;

    // variabili per il cheater
    simtime_t intervalS = 1;    // finestra di tempo in cui raccoglie messaggi prima di decidere la propria mossa
    simtime_t minTimestamp = 0.0;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    void scheduleNextMessage();
    void sendToAll(cMessage *msg);
    void counterAttack(cMessage *msg);
    void cheatedMove();
    void checkLatency(cMessage *msg, int numGate);

  private:      // PER VEDERE STATISTICHE
    cOutVector diffVector;

};

} //namespace

#endif

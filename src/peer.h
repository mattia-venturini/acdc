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
#include <list>

#include "StrategyCA.h"
#include "StrategyIncrease.h"
#include "StrategyCorrelation.h"

#define MSG_COMMON 1        // verde
#define MSG_ACDC 2          // blu
#define MSG_CHEATEDMOVE 3   // rosso
#define TOKEN_LEADER 4      // giallo
#define TIMEOUT_LEADER 5        // azzurro
#define INFO_TOKEN_RELEASED 6       // fucsia
#define INFO_CHEATER_DETECTED 7     // nero
#define MSG_UNUSED 0

using namespace omnetpp;
using namespace std;

namespace acdc {

/**
 * Modulo relativo ad un componente di una rete P2P
 */
class Peer : public cSimpleModule
{
    // costanti comuni a tutti i nodi
  private:
    cRNG* random = this->getRNG(0);       // RNG preso da omnet, con seme fissato e quindi riproducibile
    simtime_t timeoutLeader;
    simtime_t minLatency;
    simtime_t maxLatency;
    simtime_t minGenTime;
    simtime_t maxGenTime;

  protected:
    int nActivePeers;   // numero di Peer non cheater
    int nLinks;         // numero di Peer a cui è connesso
    int *idPeers;       // memorizza per ogni gate l'id del Peer a cui connette
    bool *activeLink;   // per ogni gate indica se il nodo è ritenuto onesto (true) o cheater (false)

    list<cMessage*> scheduledList;  // lista di eventi schedulati dal leader che vanno cancellati se cambia il nodo sospetto
    cMessage *timeout;          // timeout del leader, dev'essere cancellato se passa il token

    StrategyCA *strategy;       // gestisce il metodo di contrattacco, implementato come StrategyIncrease o StrategyCorrelation

    // variabili utili al leader
    bool leader = false;
    int references;   // numero di nodi che hanno etichettato un sospetto come cheater

    // variabili per il cheater
    simtime_t intervalS = 0.18;    // finestra di tempo in cui raccoglie messaggi prima di decidere la propria mossa
    simtime_t minTimestamp = 0.0;


    // PROTOTIPI DEI METODI ----------------------------------------------------

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    void scheduleNextMessage();
    void sendToAll(cMessage *msg);
    void counterAttack(cMessage *msg);
    void cheatedMove();
    void checkLatency(cMessage *msg, int numGate);
    int setSuspectNode();


  private:
    // variabili per analisi statistiche
    simtime_t timeStartDetection;   // utile per calcolare il timeDetectionSignal

    // segnali per analisi statistiche
    simsignal_t timeDetectionPosSignal;
    simsignal_t timeDetectionNegSignal;

  public:
    // statiche perché complessive per tutti i Peer
    static int nTP;        // True Positives
    static int nTN;        // True Negatives
    static int nFP;        // False Positives
    static int nFN;        // False Negatives


};

} //namespace

#endif

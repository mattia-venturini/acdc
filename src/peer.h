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

#define MSG_INFO 0
#define MSG_TYPE_A 1
#define MSG_ACDC 10
#define MSG_CHEATEDMOVE 11
#define TOKEN_LEADER 42
#define TIMEOUT_LEADER 43

using namespace omnetpp;

namespace acdc {

/**
 * Generated class
 */
class Peer : public cSimpleModule
{
  private:
    static const int NChampions = 40;
    simtime_t TLeader = 2000.0;

  protected:
    //simtime_t myTstart;
    //simtime_t *Tstart;
    simtime_t *diff;    // diff[j] = latenza media dei messaggi da j considerando gli ultimi NChampions messaggi
    simtime_t threshold = 10.0;   // soglia oltre cui un nodo è dichiarato cheater

    // variabili utili al leader
    bool leader = false;
    int suspectedNode = -1;
    simtime_t delay = 1.0;

    // variabili per il cheater
    simtime_t intervalS = 1;    // finestra di tempo in cui raccoglie messaggi prima di decidere la propria mossa
    simtime_t minTimestamp = 0.0;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    void scheduleNextMessage();
    void sendToAll(cMessage *msg);
    void startCounterAttack(cMessage *msg);
    void cheatedMove();

  private:      // PER VEDERE STATISTICHE
    cOutVector *diffVector;

};

} //namespace

#endif

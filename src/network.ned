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

package acdc;
import ned.DelayChannel;


//
// Rete P2P di nodi che simulano un gioco.
// Alcuni di essi possono sfruttare il look-ahead cheat.
// Nella rete viaggia un token, chi lo possiede diventa il leader e si occupa di individuare i cheater tramite l'algoritmo ACDC
//
network Network
{
    parameters:
        @display("bgb=754,353");
        // parametri di rete
        double minLatency @unit(s) = default(0.05s);
        double maxLatency @unit(s) = default(0.15s);

        // tipo di strategia per ACDC ("increase", "correlation")
        string strategy = "increase";

        double timeoutLeader @unit(s) = default(40.00s);

        // parametri della strategia "increase"
        double delayLimit @unit(s) = default(0.80s);
        double threshold @unit(s) = default(0.10s);

        // parametri della strategia "correlation"
        int repetitions = default(10);
        double minCorrelation = default(0.62);

        // parametri dei Peer
        double minGenTime = default(0.10);
        double maxGenTime = default(0.20);

    submodules:
        peer0: Peer {
            @display("p=242,77");
            generationTime = uniform(minGenTime, maxGenTime);
        }
        peer1: Peer {
            @display("p=381,77");
            generationTime = uniform(minGenTime, maxGenTime);
        }
        peer2: Peer {
            @display("p=456,185");
            generationTime = uniform(minGenTime, maxGenTime);
            cheater = true;
        }
        peer3: Peer {
            @display("p=371,282");
            generationTime = uniform(minGenTime, maxGenTime);
        }

        peer4: Peer {
            @display("p=242,282");
            generationTime = uniform(minGenTime, maxGenTime);
        }

        peer5: Peer {
            @display("p=161,185");
            generationTime = uniform(minGenTime, maxGenTime);
        }

    connections:
        peer0.gate++ <--> DelayChannel <--> peer1.gate++;
        peer0.gate++ <--> DelayChannel <--> peer2.gate++;
        peer2.gate++ <--> DelayChannel <--> peer1.gate++;
        peer0.gate++ <--> DelayChannel <--> peer3.gate++;
        peer1.gate++ <--> DelayChannel <--> peer3.gate++;
        peer2.gate++ <--> DelayChannel <--> peer3.gate++;
        peer0.gate++ <--> DelayChannel <--> peer4.gate++;
        peer0.gate++ <--> DelayChannel <--> peer5.gate++;
        peer1.gate++ <--> DelayChannel <--> peer4.gate++;
        peer1.gate++ <--> DelayChannel <--> peer5.gate++;
        peer2.gate++ <--> DelayChannel <--> peer4.gate++;
        peer2.gate++ <--> DelayChannel <--> peer5.gate++;
        peer3.gate++ <--> DelayChannel <--> peer4.gate++;
        peer3.gate++ <--> DelayChannel <--> peer5.gate++;
        peer4.gate++ <--> DelayChannel <--> peer5.gate++;
}


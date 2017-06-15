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
// TODO documentation
//
network Network
{
    @display("bgb=754,353");
    submodules:
        peer0: Peer {
            @display("p=195,106");
            sendInitialMessage = true;
            generationTime = uniform(0.90, 1.50);
        }
        peer1: Peer {
            @display("p=429,106");
            generationTime = uniform(0.90, 1.50);
            cheater = true;
        }
        peer2: Peer {
            @display("p=323,244");
            generationTime = uniform(0.90, 1.50);
        }
    connections:
        peer0.gate++ <--> DelayChannel {  delay = exponential(1.00s); } <--> peer1.gate++;
        peer0.gate++ <--> DelayChannel {  delay = exponential(1.00s); } <--> peer2.gate++;
        peer2.gate++ <--> DelayChannel {  delay = exponential(1.00s); } <--> peer1.gate++;
}


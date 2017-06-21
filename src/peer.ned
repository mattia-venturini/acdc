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

//
// Nodo di una rete P2P
//
simple Peer
{
    parameters:
        bool sendInitialMessage = default(false);
        double generationTime = default(1);
        bool cheater = default(false);
        
        // statistiche
        @signal[timeDetectionPos](type="double");
        @statistic[timeDetectionPos](title="Time of positive detections"; record=vector,mean);
        @signal[timeDetectionNeg](type="double");
        @statistic[timeDetectionNeg](title="Time of negative detections"; record=vector,mean);
        @signal[nIncrease](type="long");
        @statistic[nIncrease](title="Number of increment per detection"; record=vector,mean);
        
    gates:
        inout gate[];
}

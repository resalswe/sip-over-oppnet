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

package sip_over_oppnet.src;

import inet.applications.IUDPApp;

//
// TODO auto-generated module
//
simple UdpMessageApp like IUDPApp
{
    parameters:
        int localPort = default(3000);  // local port. deault value = 3000
        int destPort = default(3000); // destination port: default value = 3000
        double startTime @unit(s) = default(1s); // application start time (start of the first packet)
        double waitTime @unit(s) = default(-1s);  // time of finishing sending, -1s means forever
        bool source = default(false); // if true, makes the app send message upon added route.
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int typeOfService = default(-1); // if not -1, set the ToS (IPv4) or Traffic Class (IPv6) field of sent packets to this value
        bool receiveBroadcast = default(true); // if true, makes the socket receive broadcast packets
		string outputInterface = default("wlan0"); // Interface used for transmission. default value = wlan0

        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPkLifetime](title="received packet lifetime"; source="messageAge(rcvdPk)"; unit=s; record=stats,vector; interpolationmode=none);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

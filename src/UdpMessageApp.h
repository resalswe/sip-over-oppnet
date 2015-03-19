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

#ifndef __SIP_OVER_OPPNET_UDPMESSAGEAPP_H_
#define __SIP_OVER_OPPNET_UDPMESSAGEAPP_H_

#include <omnetpp.h>

#include <vector>

#include <INETDefs.h>

#include <ApplicationBase.h>
#include "UDPSocket.h"
#include <NotificationBoard.h>

/**
 * UDP application. See NED for more info.
 */
class INET_API UdpMessageApp: public ApplicationBase, public INotifiable {
protected:
    enum SelfMsgKinds {
        SEND = 1, WAIT_ENDED, POLL
    };

    UDPSocket socket;
    int localPort, destPort;
    int interfaceID;

    simtime_t eStartTime;
    simtime_t waitTime;
    bool source; // Initiator of messages
    cMessage *selfMsg;

    // Statistics
    int numSent;
    int numReceived;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

    virtual void processPacket(cPacket *msg);
    virtual void sendPacket();
    virtual void setSocketOptions();
public:
    UdpMessageApp();
    ~UdpMessageApp();

protected:
    virtual int numInitStages() const {
        return 4;
    }
    virtual void initialize(int stage);
    virtual void handleMessageWhenUp(cMessage *msg);
    virtual void finish();

    virtual bool handleNodeStart(IDoneCallback *doneCallback);
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
    virtual void handleNodeCrash();

    virtual void receiveChangeNotification(int category, const cObject *details);
};

#endif

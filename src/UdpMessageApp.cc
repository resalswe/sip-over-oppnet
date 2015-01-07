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

#include "UdpMessageApp.h"

#include "InterfaceTableAccess.h"
#include "IPvXAddressResolver.h"
#include "NodeOperations.h"
#include "UDPControlInfo_m.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"

#include <string>

Define_Module(UdpMessageApp);

simsignal_t UdpMessageApp::sentPkSignal = registerSignal("sentPk");
simsignal_t UdpMessageApp::rcvdPkSignal = registerSignal("rcvdPk");

UdpMessageApp::UdpMessageApp() {
    selfMsg = NULL;
}

UdpMessageApp::~UdpMessageApp() {
    cancelAndDelete(selfMsg);
}

void UdpMessageApp::initialize(int stage) {
    ApplicationBase::initialize(stage);

    if (stage == 0) {
        // Statistics
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        localPort = par("localPort");
        destPort = par("destPort");
        eStartTime = par("startTime").doubleValue();
        waitTime = par("waitTime").doubleValue();
        source = par("source").boolValue();

        std::string interface = std::string(par("outputInterface").stringValue());

        selfMsg = new cMessage("sendAndWaitTimer");

        // Obtain the specified network interface to broadcast on.
        IInterfaceTable* ift = InterfaceTableAccess().get();
        InterfaceEntry *ie = ift->getInterfaceByName(interface.c_str());
        if (ie == NULL)
            throw cRuntimeError(this, "Invalid output interface name : %s", interface.c_str());
        interfaceID = ie->getInterfaceId();

        // Register callback for routes
        NotificationBoard *nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_IPv4_ROUTE_ADDED);
        nb->subscribe(this, NF_IPv4_ROUTE_DELETED);
        nb->subscribe(this, NF_IPv4_ROUTE_CHANGED);
    }
}

void UdpMessageApp::finish() {
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();

    // Register callback for routes
    NotificationBoard *nb = NotificationBoardAccess().get();
    nb->unsubscribe(this, NF_IPv4_ROUTE_ADDED);
    nb->unsubscribe(this, NF_IPv4_ROUTE_DELETED);
    nb->unsubscribe(this, NF_IPv4_ROUTE_CHANGED);
}

void UdpMessageApp::setSocketOptions() {
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int typeOfService = par("typeOfService");
    if (typeOfService != -1)
        socket.setTypeOfService(typeOfService);

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);
}

void UdpMessageApp::sendPacket() {
    char msgName[32];
    sprintf(msgName, "UdpMessageAppData-%d", (numSent + 1));
    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(32);
    payload->setTimestamp();
    payload->setName("UDP_MessageApp");

    // TODO: Get address from added link. Currently uses broadcast address only.
    UDPSocket::SendOptions options;
    options.outInterfaceId = this->interfaceID;

    IPvXAddress destAddr = IPv4Address::ALLONES_ADDRESS;
    EV << " Destination address = " << destAddr.get4();
    socket.sendTo(payload, destAddr, destPort, &options);
    numSent++;

    emit(sentPkSignal, payload);
}

void UdpMessageApp::processPacket(cPacket *pk) {
    emit(rcvdPkSignal, pk);
    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
    numReceived++;
}

//
// Implementation of the abstract method from OperationalBase. This is called from
// the handleMessage if the node is in operational mode. Otherwise the method
// handleMessageWhenDown is called.
//

void UdpMessageApp::handleMessageWhenUp(cMessage *msg) {

    if (msg->isSelfMessage()) {
        EV << "\n Self message received.";

        switch (selfMsg->getKind()) {
        case SEND:
            sendPacket();
//            selfMsg->setKind(WAIT_ENDED);
//            scheduleAt(waitTime, selfMsg);
            break;
        case WAIT_ENDED:
            EV << "handleMessageWhenUp.WAIT not implemented.";
            // TODO: Implement this
            break;
        default:
            throw cRuntimeError("Invalid kind %d in self message",
                    (int) selfMsg->getKind());
        }
    } else if (msg->getKind() == UDP_I_DATA) {
        // process incoming packet
        processPacket(PK(msg));

    } else if (msg->getKind() == UDP_I_ERROR) {
        EV << "Ignoring UDP error report\n";
        delete msg;

    } else {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (ev.isGUI()) {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

//
// Implementation of the ILifecycle interface
//

bool UdpMessageApp::handleNodeStart(IDoneCallback *doneCallback) {

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    setSocketOptions();

    return true;
}

bool UdpMessageApp::handleNodeShutdown(IDoneCallback *doneCallback) {

    if (selfMsg) {
        cancelEvent(selfMsg);
    }
    socket.close();

    return true;
}

void UdpMessageApp::handleNodeCrash() {
    if (selfMsg) {
        cancelEvent(selfMsg);
    }
    socket.close();
}

//
// Implementation of the INotifiable interface
//

void UdpMessageApp::receiveChangeNotification(int category,
        const cObject *details) {

    switch (category) {
    case NF_IPv4_ROUTE_ADDED: {
        EV << "\r- Route added 1: " << details->info();
        EV << "\r- Route added 2: " << details->getClassName();

        simtime_t zeroTime(0.0);
        if (source == false || simTime() == zeroTime) {
            return;
        }
        // Address needs to be obtained and set correctly to be able to send msg.

        simtime_t current = std::max(eStartTime, simTime());
        selfMsg->setKind(SEND);
        scheduleAt(current, selfMsg);
        EV << "\nSelf message scheduled at " << current;

        break;
    }
    case NF_IPv4_ROUTE_DELETED:
        EV << "\r- Route deleted 1: " << details->info();
        EV << "\r- Route deleted 2: " << details->getClassName();

        break;
    case NF_IPv4_ROUTE_CHANGED:
        EV << "  \r - CHANGED - TODO: Needs implementations. Category = "
                  << category;
        break;
    default:
        EV << "Unknown notification received. Category = " << category;
        break;
    }
}

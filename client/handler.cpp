#include "mainwindow.h"


// 패킷 조립 후 전송
bool Handler::SendRawPacketToServer(QString& msg){
    MsgPacket* packet = new MsgPacket();
    packet->SetProtocol(MSG_PROTOCOL);
    packet->SetLength(PACKET_SIZE);
    packet->SetName(this->username);
    packet->SetMsg(msg.toStdString());

    Byte_t buffer[PACKET_SIZE];
    packet->WritePacketHeader(buffer);
    packet->WritePacketBody(buffer);
    Socket->writeData(buffer, PACKET_SIZE);

    delete packet;
}

// stream byte를 Packet으로 만들어서 return 한다.
Packet* Handler::ReceiveRawPacket(){
    Byte_t buffer[PACKET_SIZE];
    Protocol_t protocol;
    Packet* packet;
    int byte = Socket->readData(buffer, PACKET_SIZE);
    if(byte == -1){
        return NULL; //ERROR
    }
    memcpy(&protocol, buffer, 4);
    switch(protocol){
        case MSG_PROTOCOL:
            packet = (Packet*)new MsgPacket();
            packet->ParseBuffer(buffer);
            return packet;

        case RES_USERINFO_PROTOCOL:
            packet = (Packet*)new UserPacket();
            packet->ParseBuffer(buffer);
            return packet;

        default:
            return NULL; // Protocol Error
    }
}

//접속후 기본적인 신상정보를 전송한다.
void Handler::ConnectToServer(){
    qDebug()<<"[DEBUG]"<<"Connecting...";
    Socket->connectToHost(this->host, this->port);
}


Handler::Handler(QString host, quint16 port){
    this->host = QHostAddress(host);
    this->port = port;
    this->Socket = new QTcpSocket;
}

QTcpSocket* Handler::GetSocket(){
    return this->Socket;
}

// packet을 가공해서 해당 메시지 처리
QString Handler::ProcessingResponseMsg(Packet* packet){
    QString msg = "[" + "Time" + "] ";
    msg += packet->GetMsg();
    return msg;
}


// RequestUserList에서 서버쪽으로 패킷을 보내고 Chat::ReceivePacket 메소드에서 Handler의 패킷처리를하고 해당 함수 호출
UserList* Handler::ProcessingResponseUserList(Packet* packet){
    UserList* userlist = new UserList;
    userlist->AppendUser("tuuna");
    userlist->AppendUser("kissesy");
    userlist->AppendUser("sloan");
    return userlist;

}

void Handler::SetUserName(UserPacket* packet){
    this->username = packet->GetName();
    //this->username = packet->GetName();
}

Handler::~Handler(){
    delete this->Socket;
}

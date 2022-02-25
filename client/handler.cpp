#include "mainwindow.h"


// 패킷 조립 후 전송
bool Handler::SendRawPacketToServer(QString& msg){
    if(Socket->state() != QAbstractSocket::ConnectedState){
        return false;
    }
    MsgPacket* packet = new MsgPacket();
    packet->SetProtocol(MSG_PROTOCOL);
    packet->SetLength(PACKET_SIZE);
    packet->SetName(this->username);
    packet->SetMsg(msg.toStdString());

    Byte_t buffer[PACKET_SIZE];
    packet->WritePacketHeader(buffer);
    packet->WritePacketBody(buffer);
    Socket->write(buffer, PACKET_SIZE);

    delete packet;
    return true;
}

// stream byte를 Packet으로 만들어서 return 한다.
Packet* Handler::ReceiveRawPacket(){
    Byte_t buffer[PACKET_SIZE];
    Protocol_t protocol;
    Packet* packet = NULL;
    int byte = Socket->read(buffer, PACKET_SIZE);
    if(byte == -1){
        return NULL; //ERROR
    }
    memcpy(&protocol, buffer, 4);
    switch(protocol){
        qDebug()<<"[DEBUG]"<<"Protocol Value : "<<protocol;
        case MSG_PROTOCOL:
            packet = (Packet*)new MsgPacket();
            packet->ParseBuffer(buffer);
            return packet;

        case RES_USERINFO_PROTOCOL:
            packet = (Packet*)new UserPacket();
            packet->ParseBuffer(buffer);
            return packet;

        case RES_USERLIST_PROTOCOL:
            qDebug()<<"[DEBUG]"<<"UPDATE USER LIST";
            packet = (Packet*)new UserListPacket();
            packet->ParseBuffer(buffer);
            return packet;

        default:
            return NULL; // Protocol Error
    }
}


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

// RequestUserList에서 서버쪽으로 패킷을 보내고 Chat::ReceivePacket 메소드에서 Handler의 패킷처리를하고 해당 함수 호출
UserList* Handler::ProcessingResponseUserList(UserListPacket* packet){
    UserList* userlist = new UserList;
    for(int i=0;i<packet->GetNum();i++){
        if(i > MAX_CLIENT){
            break;
        }
        QString name = packet->GetName(i);
        userlist->AppendUser(name);
    }
    return userlist;
}

void Handler::SetUserName(UserPacket* packet){
    this->username = packet->GetName();
}

std::string Handler::GetName(){
    return this->username;
}

Handler::~Handler(){
    delete this->Socket;
}

#include "mainwindow.h"


// 패킷 조립 후 전송
bool Handler::SendRawPacketToServer(QString& msg){
    qDebug()<<"[DEBUG]"<<"send msg to server : "<<msg;
    //Socket->writeData() // write vs writeData
}

// stream byte를 Packet으로 만들어서 return 한다.
Packet* Handler::ProcessingRawPacket(){
    return NULL;
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
    QString msg = "[" + packet->GetTimeStamp() + "] ";
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

// send msg 만약 비동기로 보낼거면 반환값이 중요하지 않을듯
void Handler::RequestUserList(){
    return;
}


Handler::~Handler(){
    delete this->Socket;
}

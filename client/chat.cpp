#include "mainwindow.h"

Chat::Chat(){
    handler = new Handler("127.0.0.1", 9988);
    BaseHbox = new QHBoxLayout;
    ChatVBox = new QVBoxLayout;
    InputHBox = new QHBoxLayout;
    SendBtn = new QPushButton;
    SendLine = new QLineEdit;
    UserListWidget = new QListWidget;
    MsgListWidget = new QListWidget;

    UserListWidget->setStyleSheet(
                "QListWidget::item {background:#eeeeee; border-radius:4px; color:black}"
                "QListWidget {background-color: #6D6D6D}"

                );
    UserListWidget->setSpacing(5);

    InputHBox->addWidget(SendLine);
    InputHBox->addWidget(SendBtn);

    ChatVBox->addWidget(MsgListWidget);
    ChatVBox->addLayout(InputHBox);

    BaseHbox->addLayout(ChatVBox, 3);
    BaseHbox->addWidget(UserListWidget, 1);

    SendBtn->setText("Send");

    Setup();
}

Chat::~Chat(){
    delete SendLine;
    delete SendBtn;
    delete InputHBox;
    delete UserListWidget;
    delete MsgListWidget;
    delete BaseHbox;
    delete handler;
    DeleteUserListWidgetItem();
    DeleteMsgListWidgetItem();
}

void Chat::Setup(){
    connect(handler->GetSocket(), SIGNAL(connected()), this, SLOT(onConnectedFromServer()));
    connect(handler->GetSocket(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError()));
    connect(handler->GetSocket(), SIGNAL(readyRead()), this, SLOT(onReceivePacket()));
    connect(handler->GetSocket(), SIGNAL(disconnected()), this, SLOT(onDisConnectedFromServer()));
    connect(SendBtn, SIGNAL(clicked()), this, SLOT(onSendMsg()));
    connect(SendLine, SIGNAL(returnPressed()), this, SLOT(onSendMsg()));
}

// 서버와 접속이 끊어졌습니다. 메시지 박스 - socket close?
void Chat::onError(){
    QMessageBox msgBox;
    msgBox.setText(handler->GetSocket()->errorString());
    msgBox.exec();
    exit(0);
}

// 서버와의 접속이 완료되면 호출 - 비동기 콜백
void Chat::onConnectedFromServer(){
    PrintMsg("Successfully Connected From Server! Have good day :)");
}

// 서버와의 접속이 끊어지면 호출 - 비동기 콜백
void Chat::onDisConnectedFromServer(){
    QMessageBox msgBox;
    msgBox.setText("Disconnected From Server");
    msgBox.exec();
    exit(0);
}

// 서버와의 접속 시도 -> 바도 접속이 되지 않고 딜레이 존재 -> 성공시 onConnectedFromServer() 호출
void Chat::ConnectToServer(){
    handler->ConnectToServer();
}

void Chat::onSendMsg(){
    QString msg = this->SendLine->text();
    //패킷 전송 부분
    handler->SendRawPacketToServer(msg); //private
    this->SendLine->setText("");
}

//비동기  콜백 - 서버에서 메시지가 오면 호출
void Chat::onReceivePacket(){
    qDebug()<<"[DEBUG]"<<"Received Packet From Server!";
    // ByteArray -> Assemble To Packet Object;
    Packet* packet = handler->ReceiveRawPacket();
    if(packet == NULL){
        onError();
    }
    switch (packet->GetProtocol()) {
        case MSG_PROTOCOL:
            UpdateMsg((MsgPacket*)packet);
            break;

        case RES_USERINFO_PROTOCOL:
            handler->SetUserName((UserPacket*)packet);
            PrintMsg("Hello " + QString::fromStdString(handler->GetName()));
            break;

        // 유저 목록 갱신
        case RES_USERLIST_PROTOCOL:
            qDebug()<<"[DEBUG]"<<"UPDATE USER LIST";
            DeleteUserListWidgetItem(); //기존 유저 제거
            UpdateUserList((UserListPacket*)packet);
            break;
    }
}

QHBoxLayout* Chat::get_base(){
    return BaseHbox;
}


// 패킷의 msg부분을 업데이트 + 날짜도 추가하기
void Chat::UpdateMsg(MsgPacket *packet){
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(QSize(item->sizeHint().width(), 30));
    QString msg = "[" + QDateTime::currentDateTime().toString() + "] " + "[" + packet->GetName() + "] ";
    msg += packet->GetMsg();
    item->setText(msg);
    MsgListWidget->addItem(item);
}

void Chat::PrintMsg(const QString& msg){
    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(QSize(item->sizeHint().width(), 30));
    QDateTime now = QDateTime::currentDateTime();
    QString temp = "[" + now.toString() + "] " +"[System] " + msg;
    item->setText(temp);
    MsgListWidget->addItem(item);
}

// @TODO 기존 유저 지우기 및 QListWidgetItem 객체 메모리 해제 필요
void Chat::UpdateUserList(UserListPacket* packet){
    UserList* userlist = handler->ProcessingResponseUserList(packet);
    if(userlist == NULL){
        return;
    }
    User* curr = userlist->GetUserList();
    while(curr != NULL){
        QListWidgetItem* username = new QListWidgetItem;
        username->setSizeHint(QSize(username->sizeHint().width(), 30));
        username->setText("▶  " + curr->GetName());
        UserListWidget->addItem(username);
        curr = curr->GetNext();
    }

    delete userlist;
    return;
}

void Chat::DeleteMsgListWidgetItem(){
    int count = MsgListWidget->count();
    for(int i=0;i<count;i++){
        QListWidgetItem* item = MsgListWidget->item(i);
        delete item;
    }
}

void Chat::DeleteUserListWidgetItem(){
    int count = UserListWidget->count();
    for(int i=0;i<count;i++){
        QListWidgetItem* item = UserListWidget->item(i);
        delete item;
    }
}





















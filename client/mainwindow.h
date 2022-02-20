#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QLineEdit>
#include <QLabel>
#include <QSizePolicy>
#include <iostream>
#include <QListWidget>
#include <QMessageBox>
#include <QByteArray>
#include <QIODevice>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QDateTime>
#include <cstring>
#include <string>

#define MSG_PROTOCOL 1
#define REQ_USERLIST_PROTOCOL 2
#define RES_USERLIST_PROTOCOL 3
#define RES_USERINFO_PROTOCOL 4

#define PACKET_SIZE 256

typedef char Byte_t;
typedef uint32_t Protocol_t;
typedef uint32_t Length_t;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};

class Packet{
    protected:
        Protocol_t protocol;
        Length_t len; // protocol size + len size를 뺀 body의 사이즈

    public:
        Packet();
        virtual ~Packet();
        Protocol_t GetProtocol();
        Length_t GetLength();
        void SetProtocol(Protocol_t protocol);
        void SetLength(Length_t len);
        void WritePacketHeader(Byte_t* buffer);
        virtual void WritePacketBody(Byte_t* buffer);
        virtual void ParseBuffer(const Byte_t* buffer); //buffer를 packet화
};

/*
통신을  통해 받은 buffer를 packet화 하기위해서는 ParseBuffer 메소드를 이용하고
통신패킷으로 보낼 buffer는 WritePacketheader와 WritePacketBody를 통해서 전송
*/
class MsgPacket : public Packet{
    private:
        Byte_t name[8];
        Byte_t msg[240];
    public:
        MsgPacket();
        virtual ~MsgPacket();
        virtual void WritePacketBody(Byte_t* buffer); // packet -> buffer
        virtual void ParseBuffer(const Byte_t* buffer); //buffer -> packet
        void SetName(std::string name);
        void SetMsg(std::string msg);
        QString GetName();
        QString GetMsg();
};

class UserPacket : public Packet{
    private:
        Byte_t name[8];
        Byte_t reserve[240];
    public:
        UserPacket();
        virtual ~UserPacket();
        virtual void WritePacketBody(Byte_t* buffer);
        virtual void ParseBuffer(const Byte_t* buffer);
        void SetName(std::string name);
        std::string GetName();
};

class User{
private: 
   QString name;
   User* next;

public:
   User(QString name);
   QString GetName();
   void SetNext(User* next);
   User* GetNext();
   ~User();
};


class UserList{
private:
    User* users;

public:
    UserList();
    User* GetUserList();
    void AppendUser(const QString& name);
    ~UserList();
};

class Handler{
private:
    QTcpSocket* Socket;
    std::string username; //유저를 증명하는 키값 64자리 해쉬값 난수를통해 불러옴
    QHostAddress host;
    quint16 port;

public:
    bool SendRawPacketToServer(QString& msg);
    Handler(QString host, quint16 port);
    void ConnectToServer();
    UserList* ProcessingResponseUserList(Packet* packet);
    QString ProcessingResponseMsg(Packet* packet);
    void RequestUserList();
    Packet* ReceiveRawPacket(); // Raw패킷을 Packet클래스로 전환
    QTcpSocket* GetSocket();
    void SetUserName(UserPacket* packet);
    virtual ~Handler();
};


class Chat : public QObject{

    Q_OBJECT

private:

    Handler* handler;
    // 입력값과 버튼 + 채팅메시지내역 | 사용자 리스트 나누는 레이아웃
    QHBoxLayout* BaseHbox;
    // 채팅메시지 | 입력값과 버튼 나누는 레이아웃
    QVBoxLayout* ChatVBox;
    // 메시지 리스트
    QListWidget* MsgListWidget;
    //입력값 | 버튼 나누는 레이아웃
    QHBoxLayout* InputHBox;
    // 유저 목록 리스트
    QListWidget* UserListWidget;
    QPushButton* SendBtn;
    QLineEdit* SendLine;

public:
    Chat();
    QHBoxLayout* get_base();
    void ConnectToServer();
    void UpdateUserList(Packet* packet); // UI Update
    void UpdateMsg(Packet* packet);
    void Setup();
    void DeleteUserListWidgetItem();
    void DeleteMsgListWidgetItem();
    void PrintMsg(const QString& msg);
    virtual ~Chat();

private slots:
    void onSendMsg(); // CallBack;
    void onReceivePacket(); // 비동기적으로 읽을 데이터가 있다면 해당 메소드 호출
    void onDisConnectedFromServer();
    void onConnectedFromServer();
    void onError();


};



#endif // MAINWINDOW_H






























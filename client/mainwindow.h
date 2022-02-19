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
#define UPDATE_MSG 1
#define UPDATE_USER 2
#define ERROR 3

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

// 패킷을 상속해서 여러버전으로 만들까
class Packet{
private:
    unsigned int size;
    //std::string roomkey; //roomkey
    unsigned int type;  // 0 : send msg size, 1 : get user , 2 : send message , 4 : Error
    QString msg;
    QDateTime timestamp;

public:
    Packet(unsigned int size,
           unsigned int type,
           QString msg);
    QString GetMsg();
    unsigned int GetType();
    unsigned int GetSize();
    QString GetTimeStamp();
    ~Packet();
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
    std::string userid; //유저를 증명하는 키값 64자리 해쉬값 난수를통해 불러옴
    QHostAddress host;
    quint16 port;

public:
    bool SendRawPacketToServer(QString& msg);
    Handler(QString host, quint16 port);
    void ConnectToServer();
    UserList* ProcessingResponseUserList(Packet* packet);
    QString ProcessingResponseMsg(Packet* packet);
    void RequestUserList();
    Packet* ProcessingRawPacket(); // Raw패킷을 Packet클래스로 전환
    QTcpSocket* GetSocket();
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
    void TestPrintMsg(const QString& msg);
    virtual ~Chat();

private slots:
    void onSendMsg(); // CallBack;
    void onReceivePacket(); // 비동기적으로 읽을 데이터가 있다면 해당 메소드 호출
    void onDisConnectedFromServer();
    void onConnectedFromServer();
    void onError();


};



#endif // MAINWINDOW_H






























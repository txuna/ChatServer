#include <iostream> 
#include <string> 
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <memory>
#include <stdexcept>
#include <vector>
#include <thread> 
#include <streambuf> 
#include <mutex> 
#include <condition_variable>
#include <cstring>
#include <queue>
#include <chrono>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h> 
#include <netdb.h> 
#include <signal.h>
#include <atomic>

#ifndef SERVER_H
#define SERVER_H

#define INFO 1 
#define WARNNING 2 
#define ERROR 3 

#define BUF_SIZE 36
#define PORT 9988 

#define THREAD_NUM_COUNT 5

#define EPOLL_MAX_EVENT 25

#define MAX_MSG_SIZE 240
#define PACKET_SIZE 256


#define MSG_PROTOCOL 1
#define REQ_USERLIST_PROTOCOL 2
#define RES_USERLIST_PROTOCOL 3
#define RES_USERINFO_PROTOCOL 4

#define STDIN 1 

#define MAX_CLIENT 25

typedef char Byte_t;
typedef int Socket_t; 
typedef int Epoll_t; 
typedef uint32_t Protocol_t; 
typedef uint32_t Length_t;

/*
EPOLLRDHUP 등 소켓에 대한 클로즈 작업 
https://blog.csdn.net/itworld123/article/details/104854238

Reactor 패턴 
https://blog.csdn.net/itworld123/article/details/114583351
*/

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    auto buf = std::make_unique<char[]>( size );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}


class Time{
    private:
        int year, month, day, hour, min, sec; 
    public:
        Time(int year, int month, int day, int hour, int min, int sec); 
        static Time GetCurrentTime();
        static std::string ConvertString(Time& time);  
        int GetYear(); 
        int GetMonth(); 
        int GetDay(); 
        int GetHour(); 
        int GetMin(); 
        int GetSec(); 
        ~Time(); 
}; 

/*
    REQUIREMENT 
    1. ofstream - file 
    2. ostream -> astream 여기에 값을 쓸것 ... 
    3. buffer - 4096 BYTE size 

    queue의 존재이유 : 순간적으로 overflow가 많이 발생할 수 있다. 하지만 worker스레드에서는 처리를 진행중일때 
    queue에 값을 넣어서 순차적으로 접근이 가능
*/

class FileBuffer : public std::streambuf{
    private:
        //std::ostream async_stream; // ex) logging.info("START") -> async_stream << msg; 
        //Queue queue; // when do overflow, descturctor, timer, insert buffer to queue and setp buffer 
        std::ofstream file;
        std::thread io_thread; 
        std::thread timer_thread;  
        std::mutex mutx; 
        std::condition_variable condition;
        std::string path;
        std::char_traits<char>::char_type* buffer = new std::char_traits<char>::char_type[BUF_SIZE]; //stream buffer 
        std::queue<std::char_traits<char>::char_type*> buffer_queue;
        bool ready;
    public:
        FileBuffer(std::string path); 
        ~FileBuffer(); // sync or flush and buffer delete 
        // 지정된 buffer가 넘칠 때 
        virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type c); 
        // stream buffer to write file and call thread (set condition.noify() )
        virtual int sync();  
        void io_worker(); //thread 
        void timer_worker(); //thread 설정된 시간마다 buffer queue로 push 및 초기화 
        void buffer_init(); // buffer setp 초기화 및 할당 
        void queue_init();
        void stop_thread();
};

class Logging{
    private:
        std::string path; 
        std::string GetCurrentPath();
        //FileBuffer file_buf;
        //std::ostream async_stream;
        std::ofstream fstream;
        void WriteData(std::string msg);

    public:
        Logging();
        void info(std::string msg);
        void warning(std::string msg); 
        void error(std::string msg); 
        void debug(std::string msg);
        ~Logging();
};


class Client{
    private:
        Socket_t client_fd; 
        std::string name; // Client 접속시 임의로 지정  
        Client* next;
        struct sockaddr_in info; 
    public:
        Client(std::string name, Socket_t client_fd, struct sockaddr_in info); 
        Socket_t GetClientFd(); 
        std::string GetClientName(); 
        void SetNext(Client* next);
        Client* GetNext();  
        std::string GetIP();
        int GetPort();  
        ~Client(); 
};  


// thread not safe 
class ClientList{
    private:
        Client* head; 
        Client* GetTail(); 
    public:
        ClientList(); 
        bool IsEmpty();
        Client* GetClient(Socket_t client_fd); 
        bool AddClient(std::string name, Socket_t client_fd, struct sockaddr_in info); 
        void DeleteClient(Socket_t client_fd);
        Client* GetFront();
        int GetClientCount();
        ~ClientList();
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
        virtual void PrintPacket(); 
        virtual bool PacketInvalidation(); 
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
        virtual void PrintPacket();
        void SetName(std::string name); 
        void SetMsg(std::string msg);
        virtual bool PacketInvalidation(); 
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
        virtual void PrintPacket(); 
        void SetName(std::string name);
};

class UserListPacket : public Packet{
    private:
        uint32_t num; // num of user
        Byte_t name_list[232]; // max 29 * 8 + NULL(*)
        Byte_t reserve[12];

    public:
        UserListPacket(); 
        virtual ~UserListPacket(); 
        virtual void WritePacketBody(Byte_t* buffer); 
        virtual void ParseBuffer(const Byte_t* buffer); 
        virtual void PrintPacket(); 
        void SetNum(uint32_t num); 
        void AddUserName(int index, std::string username);
};

// 패킷 send시 fd - packet 쌍 
struct ClientPacketMap{
    Socket_t fd; 
    Packet* packet; 
};

class Handler{
    private:
        struct epoll_event events[EPOLL_MAX_EVENT];
        Epoll_t Epollfd; 
        Socket_t ServSock;

        std::condition_variable read_client_condition;
        std::condition_variable send_client_condition; 

        std::thread read_client_thread[THREAD_NUM_COUNT];
        std::thread send_client_thread[THREAD_NUM_COUNT];

        std::mutex client_list_mutx;  // 클라이언트 리스트 뮤텍스 
        std::mutex read_client_queue_mutx;  // 클라이언트의 입력 처리를 위한 큐 뮤텍스  
        std::mutex send_client_queue_mutx; // 클라이언트에게 패킷전송을 위한 큐 뮤텍스 

        std::queue<struct epoll_event> read_client_queue;  // 클라이언트 read queue 
        std::queue<struct ClientPacketMap> send_client_queue; // 클라이언트 send queue
        ClientList* client_list; //클라이언트 리스트
        Logging logging; 

        
        // packet의 protocol 타입을 보고 파싱한다. 그 뒤로는? 
        void ParsePacket(Packet* packet);
        void ReadPacket(Socket_t fd); //Packet* 타입을 반환하고 protocol보고 해당하는 타입 생성
        void SendPacket(Socket_t fd, Packet* packet); 
        void read_client_worker(); 
        void send_client_worker(); 
        void RegisterNewClient();
        const char* CheckEventType(int type); 
        void StopServer();
        void ProcessingMsgPacket(const Byte_t* buffer); // 클라이언트로 부터 전달받은 메시지를  처리하는 메소드
        bool CheckClientClose(int event);
        void CloseClient(Socket_t fd);
        void SendUserInfoToClient(std::string username, Socket_t fd);
        void SendUserListToConnectedClient();
        bool CheckMaxClient();
    public:
        void StopThread(); 
        Handler(); 
        Client* GetClient(Socket_t client_fd);
        void ManageRequestFromClient(); 
        ~Handler();
}; 


#endif
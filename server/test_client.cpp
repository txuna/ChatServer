#include <netdb.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <memory>
#include <cstring>

#define MAX 80
#define PORT 9988
#define SA struct sockaddr

#define MSG_PROTOCOL 1
#define REQ_USERLIST_PROTOCOL 2 
#define REQ_USERINFO_PROTOCOL 3

typedef char Byte_t;
typedef int Socket_t; 
typedef int Epoll_t; 
typedef uint32_t Protocol_t; 
typedef uint32_t Length_t;

void func(int sockfd)
{
    int offset = 0;
    char buffer[256] = {0, };
    read(sockfd, buffer, 256); // 이거 없으면 서버단에서 segmentation fault발생 // GetUserInfo
    // Bad File Descriptor 
    std::cout<<"[Name]"<<std::endl;
    std::cout<<buffer+8<<std::endl;
    memset(buffer, 0, 256);

    //Get User List
    read(sockfd, buffer, 256); 
    uint32_t num=0; 
    memcpy(&num, buffer+8, 4);
    std::cout<<"User Num : "<<num<<std::endl;
    /*for(int i=0;i<num;i++){
        char name[8] = {0, };
        memcpy(name, buffer+12+(8*i), 8); 
        name[7] = 0x0;
        std::cout<<"User Name : "<<name<<std::endl;
    }
    memset(buffer, 0, 256);

    Protocol_t protocol = MSG_PROTOCOL; 
    Length_t len = 256;
    memcpy(buffer, &protocol, sizeof(Protocol_t));
    offset += sizeof(Protocol_t); 
    memcpy(buffer+offset, &len, sizeof(Length_t)); 
    offset += sizeof(Length_t); 
    memcpy(buffer+offset, "tuuna", 8);
    offset += 8; 
    memcpy(buffer+offset, "hello world!", 240);  
    //std::cout<<buffer+offset<<std::endl;
    write(sockfd, buffer, 256);
    //sleep(5);
    
    memset(buffer, 0, 256); 
    read(sockfd, buffer, 256);
    std::cout<<"[Message]"<<std::endl;
    std::cout<<buffer+16<<std::endl;
    sleep(3);
}
   
int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
   
    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    memset(&servaddr, 0x0, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
   
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
   
    // function for chat
    func(sockfd);
   
    // close the socket
    close(sockfd);
}
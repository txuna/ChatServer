#include "main.h"

bool is_stop_thread = false;

// 소켓 설정 및 accept까지의 설정 
// 스레드 생성 
Handler::Handler(){
    client_list = new ClientList();
    ServSock = socket(AF_INET, SOCK_STREAM, 0); 
    struct sockaddr_in serv_sockaddr; 
    serv_sockaddr.sin_family = AF_INET; 
    serv_sockaddr.sin_port = htons(PORT); 
    serv_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    int optval = 1;
    setsockopt(ServSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if(bind(ServSock, (struct sockaddr*)&serv_sockaddr, sizeof(serv_sockaddr)) == -1){
        logging.error(string_format("Bind() Error : %s", strerror(errno)));
        throw std::runtime_error(string_format("Bind() Error : %s", strerror(errno))); 
    }

    if(listen(ServSock, 20) == -1){
        logging.error(string_format("Listen() Error : %s", strerror(errno))); 
        throw std::runtime_error(string_format("Listen() Error : %s", strerror(errno)));
    }

    for(int i=0;i<THREAD_NUM_COUNT; i++){
        read_client_thread[i] = std::thread(&Handler::read_client_worker, this);
        send_client_thread[i] = std::thread(&Handler::send_client_worker, this);
    }
}

/*
    모든 소켓 fd를 관리하며 메인 핸들러 역할 - 생산자 
    condition_variable은 내부적으로 unlock() -> lock() 을 지님
*/
void Handler::ManageRequestFromClient(){
    logging.info("Start Server...");
    //int client_fd = accept(ServSock, (struct sockaddr*)&client_addr, &length);
    Epollfd = epoll_create1(0); 
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // | EPOLLONESHOT ... 
    ev.data.fd = ServSock; 
    
    // 왜 ev는포인터를 넘기는가. EPOLLING | EPOLLRDHUD(클라이언트 연결 끊김 감지 )
    // 포인터를 넘겨도 커널단에서 필요한 정보 다 흡수함 나중에 새로 클라이언트에 재활용 가능
    if(epoll_ctl(Epollfd, EPOLL_CTL_ADD, ServSock, &ev) == -1){
        logging.error(string_format("epoll_ctl() Error : %s", strerror(errno)));
        throw std::runtime_error("epoll_ctl error()");
    }
    ev.events = EPOLLIN | EPOLLET; 
    ev.data.fd = STDIN;
    if(epoll_ctl(Epollfd, EPOLL_CTL_ADD, STDIN, &ev) == -1){
        logging.error(string_format("epoll_ctl() Error : %s", strerror(errno)));
        throw std::runtime_error("epoll_ctl error()");
    }
    while(true){
        // EPOLLONESHOT을 사용해서 다른 스레드가 해당 모니터링된 파일디스크립터를 한번만 처리할 수 있도록 함 
        // 다시 읽는 경우에는 어떻게? 
        int reacted_fd_num = epoll_wait(Epollfd, events, EPOLL_MAX_EVENT, -1); 
        if(reacted_fd_num == -1){
            logging.error(string_format("epoll_wait() Error : %s", strerror(errno)));
        }
        for(int i=0;i<reacted_fd_num;i++){
            // client의 새로운 요청
            if(events[i].data.fd == ServSock){
                RegisterNewClient();
            }else if(events[i].data.fd == 1){
                char temp[256]; 
                read(events[i].data.fd, temp, 255);
                StopServer();
            }else{
                // read 하는 과정에서 계속된 요청이 생길듯 ...  그럼 Edge 트리거로? 
                // notify할때도 Lock이 필순가. 
                std::unique_lock<std::mutex> guard(this->read_client_queue_mutx);
                read_client_queue.push(events[i]);
                read_client_condition.notify_one(); //notify_one할때도내부에서 unlock -> lock() 
            }
        }
    }
}

// read send thread 정리등등
void Handler::StopServer(){
    logging.info("Stop Server...");
    this->~Handler();
    exit(0);
}

/*
    새로운 클라이언트에 대해 이름을 짓고 설정한다. 
    이름은 User + fd  그리고 클라이언트에게 OK 신호와 유저이름 전송
*/
void Handler::RegisterNewClient(){
    Socket_t client_fd; 
    struct sockaddr_in client_addr; 
    unsigned int length = sizeof(client_addr); 
    client_fd = accept(ServSock, (struct sockaddr*)&client_addr, &length); 
    std::string username = string_format("%s%d", "User", client_fd);
    // mutex 필요
    {
        std::unique_lock<std::mutex> guard(this->client_list_mutx); 
        client_list->AddClient(username, client_fd, client_addr);
    }
    struct epoll_event ev; 
    ev.data.fd = client_fd; 
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLERR; // EPOLLET -> 해당 소켓을 Edge Trigger로 설정 -> 클라이언트의 입력이 생기는 그 즉시만 이벤트링
    epoll_ctl(Epollfd, EPOLL_CTL_ADD, client_fd, &ev);
    Client* client = NULL;
    {
        std::unique_lock<std::mutex> guard(this->client_list_mutx); 
        client = client_list->GetClient(client_fd);
    } 
    logging.info(string_format("User:%s [IP:%s] Connected", client->GetClientName().c_str(), client->GetIP().c_str()));
    std::cout<<string_format("User:%s [IP:%s] Connected", client->GetClientName().c_str(), client->GetIP().c_str())<<std::endl;
    // Client에게 Client  유저의 정보 전송 - 클라이언트는 받은 정보를 기반으로 닉네임 확인 가능
    SendUserInfoToClient(username, client_fd);
    std::cout<<"Have Managed Client Count : "<<client_list->GetClientCount()<<std::endl;
}

void Handler::SendUserInfoToClient(std::string username, Socket_t fd){
    // 클라이언트에게 전송 - 자신의 닉네임 부여 
    UserPacket* userpacket = new UserPacket();
    userpacket->SetProtocol(RES_USERINFO_PROTOCOL);
    userpacket->SetLength(PACKET_SIZE);
    userpacket->SetName(username); 
    {
        std::unique_lock<std::mutex> guard(this->send_client_queue_mutx); 
        struct ClientPacketMap clpkmap = {fd, (Packet*)userpacket}; 
        send_client_queue.push(clpkmap);
    }
    send_client_condition.notify_one();
}

// epoll_ctl - EPOLL_CTL_DEL 모니터링 제거 -> Client_list에서 제거
// read and send 큐에서 해당 파일디스크립터 제거? 안해도 될려남 UUMMMMMMMMMMMMMM
// read queue는 지울필요가 없고 send 큐에? send 할때 해당 큐가 client_list에 있는지 확인
void Handler::CloseClient(Socket_t fd){
    {
        Client* client = NULL;
        std::unique_lock<std::mutex> guard(this->client_list_mutx); 
        client = client_list->GetClient(fd);
        if(client == NULL){
            return;
        }

        logging.info(string_format("User:%s [IP:%s] Disconnected", client->GetClientName().c_str(), client->GetIP().c_str()));
        std::cout<<string_format("User:%s [IP:%s] Disconnected", client->GetClientName().c_str(), client->GetIP().c_str())<<std::endl;

        client_list->DeleteClient(fd);
        epoll_ctl(Epollfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        std::cout<<"Have Managed Client Count : "<<client_list->GetClientCount()<<std::endl;
    }
}

bool Handler::CheckClientClose(int event){
    switch (event)
    {
    case EPOLLRDHUP:
        return true;
    
    case EPOLLRDHUP+EPOLLIN: // Ubuntu 18.04에서 peer close가 이렇게 감지
        return true; 

    case EPOLLERR:
        return true;
    default:
        return false;
    }
}

/*
    클라이언트의 입출력을 담당하는 작업 스레드이다. - 소비자 역할 
    Event Type이 EOF인지도 확인 필요
*/

void Handler::read_client_worker(){
    while(!is_stop_thread){
        std::unique_lock<std::mutex> guard(this->read_client_queue_mutx); 
        while(read_client_queue.empty()){
            if(is_stop_thread) return;
            this->read_client_condition.wait(guard);
        }
        struct epoll_event event = read_client_queue.front();
        read_client_queue.pop(); 
        guard.unlock();
        Socket_t client_fd = event.data.fd; 
        // Close Client Socket - 일부 시스템에서는 EPOLLRDHUP 지원을 안함 -> 그럼 어떻게?
        if(CheckClientClose(event.events)){
            CloseClient(client_fd);
        }else{
            // rereturn Packet 추후 type 체크하여 알맞은 값으로 캐스팅
            ReadPacket(client_fd); 
        }
    }
}


void Handler::ReadPacket(Socket_t fd){
    Byte_t buffer[PACKET_SIZE]; 
    Protocol_t protocol; 
    std::string username; 
    std::string ip; 
    {
        std::unique_lock<std::mutex> guard(this->client_list_mutx);
        Client* client = client_list->GetClient(fd);
        username = client->GetClientName(); 
        ip = client->GetIP();
    }
    // 한번에 PacketSIZE를 못읽을 수 도 있다. 그렇기에 반복문과 return되는 바이트를 확인해서 크기 조정
    int byte = recv(fd, buffer, PACKET_SIZE, MSG_WAITALL); //MSG_WAITALL : 요청한 메시지의 크기가 차야 반환 
    if(byte == -1){ //Error 송신했는데 왜 세그폴트? 
        logging.warning(string_format("Can't read Packet From User:%s [IP:%s]: %s", username.c_str(), ip.c_str(), strerror(errno)));
        CloseClient(fd);
        return;
    }
    logging.info(string_format("Read Packet From User:%s [IP:%s]", username.c_str(), ip.c_str()));
    
    memcpy(&protocol, buffer, 4);

    switch(protocol){
        case MSG_PROTOCOL: {
            ProcessingMsgPacket(buffer);
            break; 
        }
        
        case REQ_USERLIST_PROTOCOL:
            break; 

        // Invalid Porotocol 또는 연결 해제 EOF -> 구분 필요 -> 둘다 연결 끊을거
        default:
            logging.warning(string_format("Invalid Protocol from User:%s [IP : %s]", username.c_str(), ip.c_str()));
            break; 
    }
}

// 클라이언트에게 보낼 패킷 제작 보낸사람 포함
void Handler::ProcessingMsgPacket(const Byte_t* buffer){
    {
        std::unique_lock<std::mutex> guard(this->send_client_queue_mutx);
        {
            std::unique_lock<std::mutex> guard(this->client_list_mutx); 
            Client* client = client_list->GetFront(); 
            while(true){
                if(client == NULL){
                    break; 
                }
                MsgPacket* packet = new MsgPacket();
                packet->ParseBuffer(buffer);
                struct ClientPacketMap clpkmap = {client->GetClientFd(), (Packet*)packet}; //생각해보니 packet의 주소가 같은 영역임 -> buffer를 여기서 파싱해서 새로운 객체를 매번 만들어야할듯 
                send_client_queue.push(clpkmap);
                client = client->GetNext();
            }
        }
    }
    send_client_condition.notify_one();
}

void Handler::send_client_worker(){
    while(!is_stop_thread){
        std::unique_lock<std::mutex> guard(this->send_client_queue_mutx); 
        while(send_client_queue.empty()){
            if(is_stop_thread) return;
            this->send_client_condition.wait(guard);
        }
        struct ClientPacketMap clpkmap = send_client_queue.front(); 
        send_client_queue.pop(); 
        guard.unlock();
        SendPacket(clpkmap.fd, clpkmap.packet);
    }
}


// 전송후 delete packet 
void Handler::SendPacket(Socket_t fd, Packet* packet){
    Byte_t buffer[PACKET_SIZE] = {0, };
    std::cout<<"Server to : "<<fd<<std::endl;
    packet->PrintPacket(); //패킷이 왜 비어져있지
    packet->WritePacketHeader(buffer);
    packet->WritePacketBody(buffer); 
    Client* client = NULL;
    {
        std::unique_lock<std::mutex> guard(this->client_list_mutx);
        client = client_list->GetClient(fd);
    }
    if(client == NULL){
        return; // 이미 종료된 소켓 
    }
    //Error Check -  // 전송된 데이터량 반환 및 에러체크
    int byte = send(fd, buffer, PACKET_SIZE, 0);
    if(byte == -1){
        logging.warning(string_format("Can't send Packet To User:%s [IP:%s] : %s", client->GetClientName().c_str(), client->GetIP().c_str(), strerror(errno))); 
        CloseClient(fd);
        return;
    }
    logging.info(string_format("Send Packet To User:%s [IP:%s]", client->GetClientName().c_str(), client->GetIP().c_str()));

    // 어떤스레드는 해제하고 어떤스레드는 접근하나?  YES -> ProcessingMsg 함수에서 동일한 힙주소를 가진 packet를 send_client_queue에 넣어서 매번 해제(같은 거를)해서 double free bug 발생한거임
    delete packet;

}

const char* Handler::CheckEventType(int type){
    switch (type)
    {
    case EPOLLIN:
        return "EPOLLIN";
    case EPOLLOUT:
        return "EPOLLOUT";
    case EPOLLPRI:
        return "EPOLLPRI"; 
    case EPOLLERR:
        return "EPOLLERR";    
    case EPOLLRDHUP:
        return "EPOLLRDHUP";
    case EPOLLRDHUP + EPOLLIN: 
        return "EPOLLRDHUP + EPOLLIN";
    default:
        std::cout<<"EVENT TYPE : "<<type<<std::endl;
        return "ERROR";
    }
}

void Handler::StopThread(){
    is_stop_thread = true;
    read_client_condition.notify_all(); //notify_one할때도내부에서 unlock -> lock() 
    send_client_condition.notify_all();
}

//스레드 조인 
Handler::~Handler(){
    StopThread();
    for(int i=0;i<THREAD_NUM_COUNT; i++){
        read_client_thread[i].join(); 
        send_client_thread[i].join();
    }
    delete client_list;
}
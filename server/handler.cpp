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
        logging.error(string_format("%s in %s and Line number is %d", "can not observe ServSock using epoll_ctl ", __FILE__, __LINE__ - 1));
        throw std::runtime_error("epoll_ctl error()");
    }
    ev.events = EPOLLIN | EPOLLET; 
    ev.data.fd = STDIN;
    if(epoll_ctl(Epollfd, EPOLL_CTL_ADD, STDIN, &ev) == -1){
        logging.error(string_format("Can't Add STDIN in epoll", __FILE__, __LINE__ - 1));
        throw std::runtime_error("epoll_ctl error()");
    }
    while(true){
        // EPOLLONESHOT을 사용해서 다른 스레드가 해당 모니터링된 파일디스크립터를 한번만 처리할 수 있도록 함 
        // 다시 읽는 경우에는 어떻게? 
        int reacted_fd_num = epoll_wait(Epollfd, events, EPOLL_MAX_EVENT, -1); 
        if(reacted_fd_num == -1){
            logging.error(string_format("%s in %s and Line number is %d", "epoll_wait return -1", __FILE__, __LINE__));
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

/*
void Handler::read_client_worker(){
    while(!is_stop_thread){
        std::unique_lock<std::mutex> guard(this->read_client_queue_mutx); 
        this->read_client_condition.wait(guard); //wait에서 깨어나면 다시 lock 을 가지는감
        // Thread 실행 도중 중지 
        if(is_stop_thread){
            return;
        }
        // 거짓 통보나 큐가 비어져있다면
        if(read_client_queue.empty()){
            continue;
        }
        struct epoll_event event = read_client_queue.front();
        read_client_queue.pop(); 
        //여기서 read를 해버리면 lock된 상태임 IO는 시간이 걸리므로 unlock을 해줘야함
        guard.unlock(); // lock_guard는 unlock이라는 기능이 없어서 따로 구현해야함 그래서 condition_variable에 사용못함
        
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
*/

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
    Packet* packet = NULL;

    switch(protocol){
        case MSG_PROTOCOL:
            packet = (Packet*)new MsgPacket();
            packet->ParseBuffer(buffer); //packet이 가리키는 타입에 따라 가상 메소드 호출 
            // Check Packet Invalidation
            /*
            if(packet->PacketInvalidation()){
                std::unique_lock<std::mutex> guard(this->client_list_mutx);
                Client* client = client_list->GetClient(fd);
                if(client != NULL){
                    logging.warning(string_format("Invalid Packet from Client IP : %s", client->GetIP().c_str()));
                }else{
                    logging.warning(string_format("%d client descriptor isn't exist in ClientList", fd));
                }   
            }
            */
            //packet->PrintPacket();
            ProcessingMsgPacket(packet);
            break; 
        
        case REQ_USERLIST_PROTOCOL:
            break; 

        // Invalid Porotocol 또는 연결 해제 EOF -> 구분 필요 -> 둘다 연결 끊을거
        default:
            logging.warning(string_format("Invalid Protocol from User:%s [IP : %s]", username.c_str(), ip.c_str()));
            break; 
    }
}

// 클라이언트에게 보낼 패킷 제작 보낸사람 포함
void Handler::ProcessingMsgPacket(Packet* packet){
    {
        std::unique_lock<std::mutex> guard(this->send_client_queue_mutx);
        {
            std::unique_lock<std::mutex> guard(this->client_list_mutx); 
            Client* client = client_list->GetFront(); 
            while(true){
                if(client == NULL){
                    break; 
                }
                struct ClientPacketMap clpkmap = {client->GetClientFd(), packet};
                send_client_queue.push(clpkmap);
                client = client->GetNext();
            }
        }
    }
    send_client_condition.notify_all();
}

void Handler::send_client_worker(){
    while(!is_stop_thread){
        std::unique_lock<std::mutex> guard(this->send_client_queue_mutx); 
        while(send_client_queue.empty()){
            this->send_client_condition.wait(guard);
        }
        struct ClientPacketMap clpkmap = send_client_queue.front(); 
        send_client_queue.pop(); 
        guard.unlock();
        SendPacket(clpkmap.fd, clpkmap.packet);
    }
}

/*
void Handler::send_client_worker(){
    while(!is_stop_thread){
        // 조건 변수의 원자성을 부여하기 위해 뮤텍스를 사용하는건데 조견변수용 뮤텍스를 만들어야 할듯
        // send_client_queue_mutex가 아닌 send_client_condition_mutex로!!!!!!!!!!! 아닌가 
        // 스레드 5개가 있고 6개의 데이터가 있다면 5개가 하나씩만 처리하고 1개를 처리하기전에 wait에 빠질듯
        // https://stackoverflow.com/questions/2379806/using-condition-variable-in-a-producer-consumer-situation
        std::unique_lock<std::mutex> guard(this->send_client_queue_mutx); 
        this->send_client_condition.wait(guard, [this]{return !send_client_queue.empty();});
        if(is_stop_thread){
            return;
        }
        
        if(send_client_queue.empty()){
            continue;
        }
        
        struct ClientPacketMap clpkmap = send_client_queue.front(); 
        send_client_queue.pop();
        guard.unlock();

        SendPacket(clpkmap.fd, clpkmap.packet);
    }
}
*/

// 전송후 delete packet 
void Handler::SendPacket(Socket_t fd, Packet* packet){
    Byte_t buffer[PACKET_SIZE];
    std::cout<<"Server to : "<<fd<<std::endl;
    std::cout<<"[Packet]"<<std::endl;
    packet->PrintPacket();
    packet->WritePacketHeader(buffer);
    packet->WritePacketBody(buffer); 
    std::string username;
    std::string ip; 
    {
        std::unique_lock<std::mutex> guard(this->client_list_mutx);
        Client* client = client_list->GetClient(fd);
        username = client->GetClientName(); 
        ip = client->GetIP();
    }
    //Error Check -  // 전송된 데이터량 반환 및 에러체크
    int byte = send(fd, buffer, PACKET_SIZE, 0);
    if(byte == -1){
        logging.warning(string_format("Can't send Packet To User:%s [IP:%s] : %s", username.c_str(), ip.c_str(), strerror(errno))); 
        CloseClient(fd);
        return;
    }
    logging.info(string_format("Send Packet To User:%s [IP:%s]", username.c_str(), ip.c_str()));

    /*
    std::cout<<"BEFORE"<<std::endl;
    delete packet;
    std::cout<<"AFTER"<<std::endl;
    */
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
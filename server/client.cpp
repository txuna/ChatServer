#include "main.h"

Client::Client(std::string name, Socket_t client_fd, struct sockaddr_in info){
    this->name = name; 
    this->client_fd = client_fd; 
    this->info = info;
}



int Client::GetClientFd(){
    return this->client_fd;
}

std::string Client::GetIP(){
    char* tmp_ip = inet_ntoa(info.sin_addr); 
    std::string ip_string(tmp_ip); // DO NOT FREE using static buffer
    return ip_string;
}

int Client::GetPort(){
    return ntohl(info.sin_port);
}

std::string Client::GetClientName(){
    return this->name;
}

Client* Client::GetNext(){
    return this->next;
}

void Client::SetNext(Client* next){
    this->next = next; 
}

Client::~Client(){

}

ClientList::ClientList() : 
    head(NULL)
{

}

int ClientList::GetClientCount(){
    int count=0;
    Client* curr = this->GetFront(); 
    while(curr != NULL){
        count += 1; 
        curr = curr->GetNext();
    }
    return count;
}

Client* ClientList::GetFront(){
    return this->head;
}

Client* ClientList::GetClient(Socket_t client_fd){
    Client* curr = this->head; 
    while(curr != NULL){
        if(curr->GetClientFd() == client_fd){
            return curr;
        }
        curr = curr->GetNext();
    }
    return NULL;
}

Client* ClientList::GetTail(){
    Client* curr = this->head; 
    while(curr->GetNext() != NULL){
        curr = curr->GetNext();
    }
    return curr; 
}

// 실패시 클라이언트에게 EOF 전송 
bool ClientList::AddClient(std::string name, Socket_t client_fd, struct sockaddr_in info){
    Client* client = new Client(name, client_fd, info); 
    client->SetNext(NULL);
    if(client == NULL){
        return false;
    }
    // 첫 번째 일때
    if(this->head == NULL){
        this->head = client; 
    }else{
        Client* tail = GetTail(); 
        tail->SetNext(client);
    }
    return true;
}

void ClientList::DeleteClient(int client_fd){
    Client* curr = this->head; 
    Client* prev = this->head;
    while(curr != NULL){
        if(curr->GetClientFd() == client_fd){
            if(curr == this->head){
                this->head = curr->GetNext();
            }else{
                prev->SetNext(curr->GetNext());
            }
            delete curr; 
            return;     
        }
        prev = curr; 
        curr = curr->GetNext(); 
    }
    return;
}

ClientList::~ClientList(){
    Client* curr = this->head; 
    while(curr != NULL){
        Client* tmp = curr;
        curr = curr->GetNext();
        delete tmp; 
    }
}


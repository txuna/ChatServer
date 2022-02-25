#include "mainwindow.h"

Packet::Packet(){

}

Packet::~Packet(){

}

Protocol_t Packet::GetProtocol(){
    return this->protocol;
}

Length_t Packet::GetLength(){
    return this->len;
}

void Packet::WritePacketHeader(Byte_t* buffer){
    memcpy(buffer, &protocol, 4);
    memcpy(buffer+4, &len, 4);
}

void Packet::SetProtocol(Protocol_t protocol){
    this->protocol = protocol;
}

void Packet::SetLength(Length_t len){
    this->len = len;
}

void Packet::WritePacketBody(Byte_t* buffer){
    return;
}

void Packet::ParseBuffer(const Byte_t* buffer){
    return;
}


MsgPacket::MsgPacket(){
    memset(this->name, 0, 8);
    memset(this->msg, 0, 240);
}

MsgPacket::~MsgPacket(){

}


void MsgPacket::WritePacketBody(Byte_t* buffer){
    int offset = sizeof(protocol) + sizeof(len);
    memcpy(buffer+offset, name, 8);
    offset += sizeof(name);
    memcpy(buffer+offset, msg, 240);
}

void MsgPacket::ParseBuffer(const Byte_t* buffer){
    int offset = 0;
    memcpy(&protocol, buffer+offset, 4);
    offset += sizeof(protocol);
    memcpy(&len, buffer+offset, 4);
    offset+= sizeof(len);
    memcpy(name, buffer+offset, 7);
    memset(name+7, 0x0, 1);  // SET NULL
    offset+=sizeof(name);
    memcpy(msg, buffer+offset, 240);
    memset(msg+239, 0x0, 1); // SET NULL
}


// max len : 7
void MsgPacket::SetName(std::string name){
    name.copy(this->name, 7);
    name[7] = 0x0;
}


// max len : 239
void MsgPacket::SetMsg(std::string msg){
    msg.copy(this->msg, 239);
    msg[239] = 0x0;
}

QString MsgPacket::GetMsg(){
    return QString(msg);
}

QString MsgPacket::GetName(){
    return QString(name);
}


/* UserPacket */

UserPacket::UserPacket(){
    memset(this->name, 0, 8);
    memset(this->reserve, 0, 240);
}

void UserPacket::WritePacketBody(Byte_t* buffer){
    int offset = sizeof(protocol) + sizeof(len);
    memcpy(buffer+offset, name, 8);
    offset += sizeof(name);
    memcpy(buffer+offset, reserve, 240);
}

void UserPacket::ParseBuffer(const Byte_t* buffer){
    int offset = 0;
    memcpy(&protocol, buffer+offset, 4);
    offset += sizeof(protocol);
    memcpy(&len, buffer+offset, 4);
    offset+= sizeof(len);
    memcpy(name, buffer+offset, 7);
    memset(name+7, 0x0, 1);  // SET NULL
    offset+=sizeof(name);
    memcpy(reserve, buffer+offset, 240);
}


void UserPacket::SetName(std::string name){
    name.copy(this->name, 7);
    name[7] = 0x0;
}

std::string UserPacket::GetName(){
    return std::string(name);
}

UserPacket::~UserPacket(){

}


/*  UserListPacket */
UserListPacket::UserListPacket(){
    this->num = 0;
    memset(name_list, 0, 232);
    memset(reserve, 0, 4);
}

UserListPacket::~UserListPacket(){

}

void UserListPacket::WritePacketBody(Byte_t* buffer){
    int offset = sizeof(protocol) + sizeof(len);
    memcpy(buffer+offset, &num, sizeof(uint32_t));
    offset += sizeof(num);
    memcpy(buffer+offset, name_list, 232);
    offset += sizeof(name_list);
    memcpy(buffer+offset, reserve, 4);
}

// 서버쪽에 Request없이 클라이언트가 접속시, 특정 클라이언트의 접속이 끊어질때마다 연결된 클라이언트에게 보낼예정
void UserListPacket::ParseBuffer(const Byte_t* buffer){
    int offset = 0;
    memcpy(&protocol, buffer+offset, 4);
    offset += sizeof(protocol);
    memcpy(&len, buffer+offset, 4);
    offset += sizeof(len);
    memcpy(&num, buffer+offset, 4);
    offset += sizeof(num);
    memcpy(name_list, buffer+offset, 232);
    offset += sizeof(name_list);
    memcpy(reserve, buffer+offset, 4);
}

void UserListPacket::PrintPacket(){
    int offset = 8;
    std::cout<<"======================="<<std::endl;
    std::cout<<"Protocol    : "<<protocol<<std::endl;
    std::cout<<"Length      : "<<len<<std::endl;
    std::cout<<"UserCount   : "<<num<<std::endl;
    std::cout<<"[Name List]"<<std::endl;
    for(int i=0;i<num;i++){
        std::cout<<"==> "<<this->name_list+(offset*i)<<std::endl;
    }
    std::cout<<"======================="<<std::endl;
}

void UserListPacket::SetNum(uint32_t num){
    this->num = num;
}

void UserListPacket::AddUserName(int index, std::string name){
    int offset = 8;
    memcpy(this->name_list+(offset*index), name.c_str(), 8);
    memset(this->name_list+(offset*index)+7, 0, 1); //NULL;
}

QString UserListPacket::GetName(int index){
    char name[8] = {0, };
    int offset = 8;
    memcpy(name, name_list+(offset*index), 8);
    name[7] = 0x0;
    return QString(name);
}

uint32_t UserListPacket::GetNum(){
    return num;
}

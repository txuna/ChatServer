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

}

QString MsgPacket::GetName(){

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

}

UserPacket::~UserPacket(){

}

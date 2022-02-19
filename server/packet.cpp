#include "main.h"

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

}

void Packet::ParseBuffer(const Byte_t* buffer){

}

void Packet::PrintPacket(){

}

bool Packet::PacketInvalidation(){

}


MsgPacket::MsgPacket(){

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


void MsgPacket::PrintPacket(){
    std::cout<<"======================="<<std::endl;
    std::cout<<"Protocol : "<<protocol<<std::endl;
    std::cout<<"Length   : "<<len<<std::endl;
    std::cout<<"Name     : "<<name<<std::endl;
    std::cout<<"Msg      : "<<msg<<std::endl;
    std::cout<<"======================="<<std::endl;
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

// 패킷값에 대한 유효성 검사 -> 값이 없다거나 등등
bool MsgPacket::PacketInvalidation(){
    for(int i=0;i<8;i++){
        if(!(name[i] >= 'A' and name[i] <= 'z')){
            return true;
        }
    }
    return false;
}


/* UserPacket */

UserPacket::UserPacket(){

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

void UserPacket::PrintPacket(){
    std::cout<<"======================="<<std::endl;
    std::cout<<"Protocol : "<<protocol<<std::endl;
    std::cout<<"Length   : "<<len<<std::endl;
    std::cout<<"Name     : "<<name<<std::endl;
    std::cout<<"======================="<<std::endl;
}

void UserPacket::SetName(std::string name){
    name.copy(this->name, 7); 
    name[7] = 0x0;
}

UserPacket::~UserPacket(){

}
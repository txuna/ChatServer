#include "mainwindow.h"



Packet::Packet(unsigned int size,
               unsigned int type,
               QString msg){
    this->size = size;
    this->type = type;
    this->msg = msg;
}

QString Packet::GetMsg(){
    return this->msg;
}

unsigned int Packet::GetType(){
    return this->type;
}

unsigned int Packet::GetSize(){
    return this->size;
}

QString Packet::GetTimeStamp(){
    return this->timestamp.toString();
}


Packet::~Packet(){

}

#include "main.h"

std::string Logging::GetCurrentPath(){
    char temp[1024]; 
    if(getcwd(temp, 1024) != 0){
        return std::string(temp); 
    }
    int error = errno; 
    switch(error){
        case EACCES:
            throw std::runtime_error("Access denied"); 

        case ENOMEM:
            throw std::runtime_error("Insufficient storage"); 

        default: {
            throw std::runtime_error("Unrecognised Error"); 

        }
    }
}
/*
file_buf(GetCurrentPath() + "/log.txt"), 
async_stream(&file_buf)
*/
Logging::Logging(){
    fstream.open(GetCurrentPath() + "/log.txt", std::ios_base::out | std::ios_base::app);
}


Logging::~Logging(){
}

void Logging::info(std::string msg){
    Time time = Time::GetCurrentTime();
    std::string str = "[" + Time::ConvertString(time) + "] " + "[INFO]" + " " + msg + "\n"; 
    this->WriteData(str); 
}
void Logging::warning(std::string msg){
    Time time = Time::GetCurrentTime();
    std::string str = "[" + Time::ConvertString(time) + "] " + "[WARNING]" + " " + msg + "\n"; 
    this->WriteData(str); 
}
void Logging::error(std::string msg){
    Time time = Time::GetCurrentTime();
    std::string str = "[" + Time::ConvertString(time) + "] " + "[ERROR]" + " " + msg + "\n"; 
    this->WriteData(str); 
}
void Logging::debug(std::string msg){
    Time time = Time::GetCurrentTime();
    std::string str = "[" + Time::ConvertString(time) + "] " + "[DEBUG]" + " " + msg + "\n"; 
    this->WriteData(str); 
}


// file is ofstream 
void Logging::WriteData(std::string msg){
    fstream << msg << std::flush; 
    //this->async_stream << msg; //자체 buffer에 값을 씀 
    //this->async_stream << std::flush;
    return;
}
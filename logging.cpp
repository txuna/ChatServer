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

Logging::Logging(){
	this->path = GetCurrentPath() + "/log.txt"; 
	file.open(path, std::ios_base::app); 
	if(!file.is_open()){
		throw std::runtime_error(string_format("Unrecognised Error : %d", errno));

	}

}


Logging::~Logging(){
	file.close();

}

void Logging::WriteData(int type, std::string msg){
	std::string type_str; 
	switch(type){
		case INFO:
			type_str = "[INFO]"; 
			break; 
		case WARNNING:
			type_str = "[WARNNING]";
			break; 
		case ERROR:
			type_str =  "[ERROR]";
			break; 

	}
	Time time = Time::GetCurrentTime();
	std::string str = "[" + Time::ConvertString(time) + "] " +type_str + " " + msg + "\n"; 
	if(file.is_open()){
		file<<str; 

	}
	return;

}


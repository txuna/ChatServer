#include "main.h"

int main(int argc, char** argv){
	Logging* logging = new Logging(); 
	logging->WriteData(INFO, "Booting... System");
	logging->WriteData(WARNNING, "Access DataBase");
	delete logging; 
	return 0; 

} 

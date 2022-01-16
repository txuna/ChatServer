#include <iostream> 
#include <string> 
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <memory>
#include <stdexcept>

#ifndef SERVER_H
#define SERVER_H
#define INFO	 1 
#define WARNNING 2 
#define ERROR 3 

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args  )
{
	int size_s = std::snprintf( nullptr, 0, format.c_str(), args ...  ) + 1; // Extra space for '\0'
	if( size_s <= 0  ){ throw std::runtime_error( "Error during formatting."  );  }
	auto size = static_cast<size_t>( size_s  );
	auto buf = std::make_unique<char[]>( size  );
	std::snprintf( buf.get(), size, format.c_str(), args ...  );
	return std::string( buf.get(), buf.get() + size - 1  ); // We don't want the '\0' inside

}

class Time{
	private:
		int year, month, day, hour, min, sec; 
	public:
		Time(int year, int month, int day, int hour, int min, int sec); 
		static Time GetCurrentTime();
		static std::string ConvertString(Time& time);  
		int GetYear(); 
		int GetMonth(); 
		int GetDay(); 
		int GetHour(); 
		int GetMin(); 
		int GetSec(); 
		~Time(); 

}; 

class Logging{
	private:
		std::string path; 
		std::ofstream file; // On	ly Write 
		std::string GetCurrentPath(); 
	public:
		Logging();
		void WriteData(int type, std::string msg);
		~Logging();

};


class Handler{
	private:

	public:
		Handler();
		~Handler();

}; 


#endif

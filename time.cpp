#include "main.h"

Time Time::GetCurrentTime(){
	std::time_t t = std::time(0); 
	std::tm* now = std::localtime(&t); 
	Time time(
			now->tm_year + 1900, 
			now->tm_mon + 1, 
			now->tm_mday, 
			now->tm_hour, 
			now->tm_min, 
			now->tm_sec

			);
	return time;  

}

std::string Time::ConvertString(Time& time){
	return string_format("%d-%d-%d %d:%d:%d", time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMin(), time.GetSec());

}

int Time::GetYear(){
	return this->year; 

}

int Time::GetMonth(){
	return this->month; 

}

int Time::GetDay(){
	return this->day; 

}

int Time::GetHour(){
	return this->hour; 

}

int Time::GetMin(){
	return this->min; 

}

int Time::GetSec(){
	return this->sec; 

}

Time::Time(int year, int month, int day, int hour, int min, int sec){
	this->year = year; 
	this->month = month; 
	this->day = day; 
	this->hour = hour; 
	this->min = min;
	this->sec = sec; 

}

Time::~Time(){


}

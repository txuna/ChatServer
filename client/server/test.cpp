#include <iostream>
#include <fstream>

class Base{
    public:
        Base(){

        }
        virtual ~Base(){

        }
};

class Derived : public Base{
    public:
        Derived(){

        }
        virtual ~Derived(){
            std::cout<<"asdsa"<<std::endl;
        }
};

int main(void){
    Derived* d = new Derived(); 
    Base* b = (Base*)d; 
    delete b;
}
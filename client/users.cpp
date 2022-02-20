#include "mainwindow.h"


User::User(QString name){
    this->name = name;
    this->next = NULL;
}


QString User::GetName(){
    return this->name;
}

void User::SetNext(User* next){
    this->next = next;
}

User* User::GetNext(){
    return this->next;
}


User::~User(){

}


UserList::UserList(){
    this->users = NULL;
}


User* UserList::GetUserList(){
    return this->users;
}


void UserList::AppendUser(const QString& name){
    User* newuser = new User(name);
    newuser->SetNext(NULL);
    if(this->users == NULL){
        this->users = newuser;
    }else{
        User* curr = this->users;
        while(curr->GetNext() != NULL){
            curr = curr->GetNext();
        }
        curr->SetNext(newuser);
    }
}


UserList::~UserList(){
    if(this->users == NULL){
        return;
    }
    User* curr = this->users;
    while(curr != NULL){
        delete curr;
        curr = curr->GetNext();
    }
}

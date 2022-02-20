#include "main.h"

template <class T> 
Node<T>::Node(const T* buffer, const Node* next){
    this->buffer = buffer;
    this->next = next; 
    
}

template <class T>
Node<T>::~Node(){
    if(buffer != NULL){
        delete buffer;
    }
}

template <class T>
Node* Node<T>::GetNext(){
    return next; 
}

template <class T>
void Node<T>::SetNext(const Node* next){
    this->next = next; 
}

template <class T>
T* Node<T>::GetBuffer(){

}


template <class T>
Queue<T>::Queue(){

}

template <class T>
Queue<T>::~Queue(){

}

template <class T>
unsigned int Queue<T>::GetLen(){

}

template <class T>
void Queue<T>::Push(const T* t) const {

}

template <class T>
T* Queue<T>::Pop(){

}
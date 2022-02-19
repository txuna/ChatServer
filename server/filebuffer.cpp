#include "main.h"

FileBuffer::FileBuffer(std::string path) : 
    ready(false),
    io_thread(&FileBuffer::io_worker, this),
    timer_thread(&FileBuffer::timer_worker, this)
{
    buffer = NULL;
    this->path = path;
    file.open(this->path, std::ios_base::app); 
    if(!file.is_open()){
        throw std::runtime_error(string_format("Unrecognised Error : %d", errno));
    }
    // buffer_init
    buffer_init();
}

// overflow 됐을때 overflow된 나머지는? 
// overflow되면 해당 버퍼 sync 
/*
버퍼가 꽉차면 해당 버퍼를 늘리지 않고 그냥 큐로 전송? 
타이머로와 소멸자로만 버퍼 sync? 
*/
std::char_traits<char>::int_type FileBuffer::overflow(std::char_traits<char>::int_type c){
    // 해당 문자가 EOF인지 확인 
    //std::cout<<this->pptr() - this->pbase()<<std::endl;
    if(c != std::char_traits<char>::eof()){
        *this->pptr() = std::char_traits<char>::to_char_type(c);  //overflow 된 한문자 넣기 위한 임시공간
        this->pbump(1);       
    }
    //std::cout<<buffer<<std::endl;
    // buffer into queue; queue is shallow copy and do lock and unlock 
    {
        std::unique_lock<std::mutex> guard(this->mutx); // 해당 객체가 소멸될때 unlock이 됨 
        buffer_queue.push(buffer); 
    }
    // buffer init 
    sync();
    buffer_init();
    
}

void FileBuffer::buffer_init(){
    // NULL이라면 이전에 할당 된것 해제
    if(buffer != NULL){
        //delete[] buffer;  queue에 넣어서 관리할거니 따로 delete X 
        buffer = NULL;
    }
    buffer = new std::char_traits<char>::char_type[BUF_SIZE]; 
    memset(buffer, 0x0, BUF_SIZE); 
    setp(buffer, buffer+BUF_SIZE-2); // -2하는 이유가 초과되는 ch를 넣기위한 임시 공간
    
}

/*
    buffer를 Queue에 넣고 buffer 초기화 
    Queue에 넣을때는 thread lock 
*/
int FileBuffer::sync(){
    std::unique_lock<std::mutex> guard(this->mutx);
    ready = true; 
    this->condition.notify_one();
    return 1; 
}


/*
 condition wait대기 -> wake up -> get value from queue -> write FILE and flush -> condition wait
*/
void FileBuffer::io_worker(){
    std::char_traits<char>::char_type* tmp_buf;
    while(true){
        std::unique_lock<std::mutex> guard(this->mutx);
        this->condition.wait(guard);     
        if(!ready){
            continue;
        }
        
        while(!buffer_queue.empty()){
            tmp_buf = buffer_queue.front(); 
            buffer_queue.pop(); 
            file.write(tmp_buf, std::streamsize(std::char_traits<char>::length(tmp_buf)));
            delete[] tmp_buf;
        }
        break;
    }
    ready = false;
    file.flush();
}


/*
    10초마다 sync 
*/
void FileBuffer::timer_worker(){
    const int time = 10;
    return;
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(time)); 
        sync();
    }
}

// queue 정리 
void FileBuffer::queue_init(){
    while(!buffer_queue.empty()){
        std::char_traits<char>::char_type* tmp = buffer_queue.front();
        buffer_queue.pop();
        delete[] tmp; 
    }
}

void FileBuffer::stop_thread(){
    io_thread.std::thread::~thread(); 
    timer_thread.std::thread::~thread();
}

// buffer에 있는것도 다 queue로 밀어넣어야 함
FileBuffer::~FileBuffer(){
    /*
        buffer 체크 및 queue로 push 
    */
    if(buffer != NULL){
        std::unique_lock<std::mutex> guard(this->mutx); // 해당 객체가 소멸될때 unlock이 됨 
        buffer_queue.push(buffer);  
    }
    sync();
    io_thread.join(); 
    timer_thread.join();
    queue_init();
}


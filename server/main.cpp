#include "main.h"

/*
reference 
https://gamedev.stackexchange.com/questions/50931/packet-handling-system-architecture 
https://gamedev.stackexchange.com/questions/43458/networking-client-server-packet-logic-how-they-communicate?rq=1
*/

/*
    해야할것 
    [] SIGINT 시그널을 받았을 때 Logging Destructor 호출하기 
    [] 클라이언트의 EOF 및 연결 끊어짐 감지했을 때 정리 (socket close, client_list queue 제거)
    [] 클라이언트에게 받은 메시지 모든 클라이언트에게 뿌리기

*/

std::atomic_bool sig_int(false);    

// overflow 됐을때 Booting까지만 출력되고 . 은 overflow ch 그럼 .. System은 어디로간걸까 
// 다음 overflow떄 출력된거 보면 어디에 저장된거같은데 커스텀한 buffer인가 그럼 어디 구간에서 확인 가능한가... 
int main(int argc, char** argv){
    Handler* handler = new Handler(); 
    // Client 의 요처을 대기 한다. 
    handler->ManageRequestFromClient();
    delete handler; 
    
    return 0; 
} 

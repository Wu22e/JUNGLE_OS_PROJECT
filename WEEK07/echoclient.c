/**
 *	@program		echoclient.c
 *
 *	@brief          서버와의 연결을 수립한 이후에 클라이언트는 표준 입력에서
 *                  텍스트 줄을 반복해서 읽는 루프에 진입하고, 서버에 텍스트 줄을 전송하고,
 *                  서버에서 echo 줄을 읽어서 그 결과를 표준 출력으로 인쇄한다.
 *                  루프틑 fgets가 EOF 표준 입력을 만나면 종료하는데, 그 이유는 사용자가
 *                  Ctrl + D를 눌렀거나 파일로 텍스트 줄을 모두 소진했기 때문이다.
 *                  
 *                  루프가 종료한 후에 클라이언트는 식별자를 닫는다. 이렇게 하면 서버로 EOF라는
 *                  통지가 전송되며, 서버는 rio_readlineb함수에서 리턴 코드 0을 받으면 이 사실을 감지한다.
 *                  자신의 식별자를 닫은 후에 클라이언트는 종료한다. 클라이언트의 커널이 프로세스가 종료할 때
 *                  자동으로 열었던 모든 식별자들을 닫아주기 때문에 이 소스코드의 마지막 부분에 close는 불필요하다. 
 *                  그러나 열었던 모든 식별자들을 명시적으로 닫아주는 것이 올바른 프로그래밍 습관이다.               
 */

#include "csapp.h"

int main(int argc, char ** argv){
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc!=3){
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd= Open_clientfd(host, port);
    /**
     *  @fn                 open_clientfd 
     *      
     *  @brief              클라이언트는 open_clientfd를 호출해서 서버와 연결을 설정한다.
     *                                           
     * 
     *  @param    host
     *  @param    port
     * 
     *  @return   int       Returns: descriptor if OK, -1 on error
     *   
     */
    
    Rio_readinitb(&rio, clientfd);

    while(Fgets(buf, MAXLINE, stdin)!=NULL){
        Rio_writen(clientfd, buf, strlen(buf));
        Rio_readlineb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);

    }
    Close(clientfd);
    exit(0);

}
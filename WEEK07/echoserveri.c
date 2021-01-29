/**
 *	@program		echoserveri.c
 *
 *	@brief          echo 서버의 메인루틴이다. 듣기 식별자를 오픈한 후에 무한 루프에 진입한다.
 *                  각각의 반복실행은 클라이언트로부터 연결 요청을 기다리며, 도메인이름과
 *                  연결된 클라이언트의 포트를 출력하고, 클라이언트를 서비스하는 echo 함수를
 *                  호출한다. echo 루틴이 리턴한 후에 메인 루틴은 연결 식별자를 닫아준다.
 *                  일단 클라이언트와 서버가 자신들의 식별자를 닫은 후에 연결은 종료된다.
 * 
 *                  이 소스코드는 간단한 echo 서버로, 한번에 한 개의 클라이언트만 처리할 수 있다는 점을
 *                  유의해야 한다. 한 번에 한 개씩의 클라이언트를 반복해서 실행하는 이런 종류의 서버를
 *                  반복 서버(iterative server) 라고 부른다.
 */

#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */ 
    // accept로 보내지는 소켓 주소 구조체다. accept가 리턴하기 전에 clientaddr에는 
    // 연결의 다른쪽 끝의 클라이언트의 소켓 주소로 채워진다.

    // 왜 clientaddr을 struct sockaddr_in이 아니라 struct sockaddr_storage형으로 선언했을까?
    // 정의에 의해 sockaddr_storage 구조체는 모든 형태의 소켓 주소를 저장하기에 충분히 크며, 
    // 이것은 코드를 프로토콜-독립적으로 유지해준다.
    
                                                                           
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if(argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);
        Close(connfd);
    }
    exit(0);
}

/**
 *	@fn		echo
 *
 *	@brief                      echo 루틴을 보여줌
 *
 *  @param  int connfd
 *
 *	@return	void
 */
void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE))!= 0){
        printf("server received %d bytes\n", (int)n); // rio_readlineb함수가 EOF를 만날 때까지 텍스트 줄을 반복해서 읽고 써준다.
        Rio_writen(connfd, buf, n);
    }
}
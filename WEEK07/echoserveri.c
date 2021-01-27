/**
 *	@program		echoserveri.c
 *
 *	@brief          echo ������ ���η�ƾ�̴�. ��� �ĺ��ڸ� ������ �Ŀ� ���� ������ �����Ѵ�.
 *                  ������ �ݺ������� Ŭ���̾�Ʈ�κ��� ���� ��û�� ��ٸ���, �������̸���
 *                  ����� Ŭ���̾�Ʈ�� ��Ʈ�� ����ϰ�, Ŭ���̾�Ʈ�� �����ϴ� echo �Լ���
 *                  ȣ���Ѵ�. echo ��ƾ�� ������ �Ŀ� ���� ��ƾ�� ���� �ĺ��ڸ� �ݾ��ش�.
 *                  �ϴ� Ŭ���̾�Ʈ�� ������ �ڽŵ��� �ĺ��ڸ� ���� �Ŀ� ������ ����ȴ�.
 * 
 *                  �� �ҽ��ڵ�� ������ echo ������, �ѹ��� �� ���� Ŭ���̾�Ʈ�� ó���� �� �ִٴ� ����
 *                  �����ؾ� �Ѵ�. �� ���� �� ������ Ŭ���̾�Ʈ�� �ݺ��ؼ� �����ϴ� �̷� ������ ������
 *                  �ݺ� ����(iterative server) ��� �θ���.
 */

#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */ 
    // accept�� �������� ���� �ּ� ����ü��. accept�� �����ϱ� ���� clientaddr���� 
    // ������ �ٸ��� ���� Ŭ���̾�Ʈ�� ���� �ּҷ� ä������.

    // �� clientaddr�� struct sockaddr_in�� �ƴ϶� struct sockaddr_storage������ ����������?
    // ���ǿ� ���� sockaddr_storage ����ü�� ��� ������ ���� �ּҸ� �����ϱ⿡ ����� ũ��, 
    // �̰��� �ڵ带 ��������-���������� �������ش�.
    
                                                                           
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
 *	@brief                      echo ��ƾ�� ������
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
        printf("server received %d bytes\n", (int)n); // rio_readlineb�Լ��� EOF�� ���� ������ �ؽ�Ʈ ���� �ݺ��ؼ� �а� ���ش�.
        Rio_writen(connfd, buf, n);
    }
}
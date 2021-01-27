/**
 *	@program		echoclient.c
 *
 *	@brief          �������� ������ ������ ���Ŀ� Ŭ���̾�Ʈ�� ǥ�� �Է¿���
 *                  �ؽ�Ʈ ���� �ݺ��ؼ� �д� ������ �����ϰ�, ������ �ؽ�Ʈ ���� �����ϰ�,
 *                  �������� echo ���� �о �� ����� ǥ�� ������� �μ��Ѵ�.
 *                  �����z fgets�� EOF ǥ�� �Է��� ������ �����ϴµ�, �� ������ ����ڰ�
 *                  Ctrl + D�� �����ų� ���Ϸ� �ؽ�Ʈ ���� ��� �����߱� �����̴�.
 *                  
 *                  ������ ������ �Ŀ� Ŭ���̾�Ʈ�� �ĺ��ڸ� �ݴ´�. �̷��� �ϸ� ������ EOF���
 *                  ������ ���۵Ǹ�, ������ rio_readlineb�Լ����� ���� �ڵ� 0�� ������ �� ����� �����Ѵ�.
 *                  �ڽ��� �ĺ��ڸ� ���� �Ŀ� Ŭ���̾�Ʈ�� �����Ѵ�. Ŭ���̾�Ʈ�� Ŀ���� ���μ����� ������ ��
 *                  �ڵ����� ������ ��� �ĺ��ڵ��� �ݾ��ֱ� ������ �� �ҽ��ڵ��� ������ �κп� close�� ���ʿ��ϴ�. 
 *                  �׷��� ������ ��� �ĺ��ڵ��� ��������� �ݾ��ִ� ���� �ùٸ� ���α׷��� �����̴�.               
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
     *  @brief              Ŭ���̾�Ʈ�� open_clientfd�� ȣ���ؼ� ������ ������ �����Ѵ�.
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
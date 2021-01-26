/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content
 */
#include "csapp.h"
void doit(int fd);
void read_requesthdrs(rio_t* rp);
int parse_uri(char* uri, char* filename, char* cgiargs);
void serve_static(int fd, char* filename, int filesize);
void get_filetype(char* filename, char* filetype);
void serve_dynamic(int fd, char* filenmae, char* cgiargs);
void clienterror(int fd, char* cause, char* errnum,
                 char* shortmsg, char* longmsg);
/**
 *	@fn		main
 *
 *	@brief                      Tiny main 猷⑦떞, TINY�뒗 諛섎났�떎�뻾 �꽌踰꾨줈 紐낅졊以꾩뿉�꽌 �꽆寃⑤컺���
 *                              �룷�듃濡쒖쓽 �뿰寃� �슂泥��쓣 �뱽�뒗�떎. open_listenfd �븿�닔瑜� �샇異쒗빐�꽌
 *                              �뱽湲� �냼耳볦쓣 �삤�뵂�븳 �썑�뿉, TINY�뒗 �쟾�삎�쟻�씤 臾댄븳 �꽌踰� 猷⑦봽瑜�
 *                              �떎�뻾�븯怨�, 諛섎났�쟻�쑝濡� �뿰寃� �슂泥��쓣 �젒�닔�븯怨�, �듃�옓�옲�뀡�쓣
 *                              �닔�뻾�븯怨�, �옄�떊 履쎌쓽 �뿰寃� �걹�쓣 �떕�뒗�떎.
 *
 *  @param  int argc
 *  @param  char **argv
 *
 *	@return	int
 */
// fd : �뙆�씪 �삉�뒗 �냼耳볦쓣 吏�移��븯湲� �쐞�빐 遺��뿬�븳 �닽�옄
// socket
int main(int argc, char **argv) //안녕하세요 기모띠
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    /* Check command-line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    listenfd = Open_listenfd(argv[1]);
    while (1) { // 臾댄븳 �꽌踰� 猷⑦봽

        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen); // 諛섎났�쟻�쑝濡� �뿰寃� �슂泥��쓣 �젒�닔
        Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd); // �듃�옖�옲�뀡 �닔�뻾   kjkjlkjsdasd
        Close(connfd); // �옄�떊 履쎌쓽 �뿰寃� �걹�쓣 �떕�뒗�떎.
        printf("===============================================\n\n");

    }
}
/**
 *	@fn		doit
 *
 *	@brief                      Tiny main 猷⑦떞, TINY�뒗 諛섎났�떎�뻾 �꽌踰꾨줈 紐낅졊以꾩뿉�꽌 �꽆寃⑤컺���
 *                              �룷�듃濡쒖쓽 �뿰寃� �슂泥��쓣 �뱽�뒗�떎. open_listenfd �븿�닔瑜� �샇異쒗빐�꽌
 *                              �뱽湲� �냼耳볦쓣 �삤�뵂�븳 �썑�뿉, TINY�뒗 �쟾�삎�쟻�씤 臾댄븳 �꽌踰� 猷⑦봽瑜�
 *                              �떎�뻾�븯怨�, 諛섎났�쟻�쑝濡� �뿰寃� �슂泥��쓣 �젒�닔�븯怨�, �듃�옓�옲�뀡�쓣
 *                              �닔�뻾�븯怨�, �옄�떊 履쎌쓽 �뿰寃� �걹�쓣 �떕�뒗�떎.
 *
 *  @param  int argc
 *  @param  char **argv
 *
 *	@return	void
 */
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
    /* Read request line and headers */
    Rio_readinitb(&rio, fd); // &rio 二쇱냼瑜� 媛�吏��뒗 踰꾪띁瑜� 留뚮뱺�떎.
    Rio_readlineb(&rio, buf, MAXLINE); // 踰꾪띁�뿉�꽌 �씫��� 寃껋씠 �떞寃⑥엳�떎.
    printf("Request headers:\n");
    printf("%s", buf); // "GET / HTTP/1.1"
    sscanf(buf, "%s %s %s", method, uri, version); // 踰꾪띁�뿉�꽌 �옄猷뚰삎�쓣 �씫�뒗�떎, 遺꾩꽍�븳�떎.

    //硫붿냼�뱶媛� get�씠 �븘�땲硫� �뿉�윭瑜� 諭됯퀬 醫낅즺�븳�떎
    if (strcasecmp(method, "GET")) { //臾몄옄�뿴�쓽 ����냼瑜� 臾댁떆�븯 鍮꾧탳�븳�떎.
        clienterror(fd, method, "501", "Not implemented",
                    "Tiny does not implement this method");
        return;
    }
    // get�씤寃쎌슦 �떎瑜� �슂泥� �뿤�뜑瑜� 臾댁떆�븳�떎.

    read_requesthdrs(&rio);

    /* Parse URI from GET request */
    //uri瑜� 遺꾩꽍�븳�떎.
    // �뙆�씪�씠 �뾾�뒗 寃쎌슦 �뿉�윭瑜� �쓣�슫�떎/
    //parse_uri瑜� �뱾�뼱媛�湲� �쟾�뿉 filename怨� cgiargs�뒗 �뾾�떎.
    is_static = parse_uri(uri, filename, cgiargs);
    printf("uri : %s, filename : %s, cgiargs : %s \n", uri, filename, cgiargs);

    if (stat(filename, &sbuf) < 0) { //stat�뒗 �뙆�씪 �젙蹂대�� 遺덈윭�삤怨� sbuf�뿉 �궡�슜�쓣 �쟻�뼱以��떎. ok 0, errer -1
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file");
        return;
    }

    // �젙�쟻 肄섑뀗痢좎씪寃쎌슦
    if (is_static) { /* Serve static content */
        //�뙆�씪 �씫湲� 沅뚰븳�씠 �엳�뒗吏� �솗�씤�븯湲�
        //S_ISREG : �씪諛� �뙆�씪�씤媛�? , S_IRUSR: �씫湲� 沅뚰븳�씠 �엳�뒗吏�? S_IXUSR �떎�뻾沅뚰븳�씠 �엳�뒗媛�?
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            //沅뚰븳�씠 �뾾�떎硫� �겢�씪�씠�뼵�듃�뿉寃� �뿉�윭瑜� �쟾�떖
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        //洹몃젃�떎硫� �겢�씪�씠�뼵�듃�뿉寃� �뙆�씪 �젣怨�
        serve_static(fd, filename, sbuf.st_size);
    }//�젙�쟻 而⑦뀗痢좉�� �븘�땺寃쎌슦
   else { /* Serve dynamic content */
        // �뙆�씪�씠 �떎�뻾媛��뒫�븳 寃껋씤吏�
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            //�떎�뻾�씠 遺덇���뒫�븯�떎硫� �뿉�윭瑜� �쟾
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't run the CGI program");
            return;
        }
        //洹몃젃�떎硫� �겢�씪�씠�뼵�듃�뿉寃� �뙆�씪 �젣怨�.
        serve_dynamic(fd, filename, cgiargs);
    }
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    //cgi-bin�씠 �뾾�떎硫�
    if (!strstr(uri, "cgi-bin")) { /* Static content*/
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri); // ./uri/home.html �씠 �맂�떎
        if (uri[strlen(uri)-1] == '/')
            strcat(filename, "home.html");
        return 1;
    }
    else { /* Dynamic content*/
        //http://52.79.55.247:1024/cgi-bin/adder?123&456
        ptr = index(uri, '?');
        //CGI�씤�옄 異붿텧
        if (ptr) {
            // 臾쇱쓬�몴 �뮘�뿉 �엳�뒗 �씤�옄 �떎 媛뽯떎 遺숈씤�떎.
            strcpy(cgiargs, ptr+1);
            //�룷�씤�꽣�뒗 臾몄옄�뿴 留덉��留됱쑝濡� 諛붽씔�떎.
            *ptr = '\0'; // uri臾쇱쓬�몴 �뮘 �떎 �뾾�븷湲�
        }
        else
            strcpy(cgiargs, ""); // 臾쇱쓬�몴 �뮘 �씤�옄�뱾 �쟾遺� �꽔湲�
        strcpy(filename, "."); //�굹癒몄�� 遺�遺� �긽���  uri濡� 諛붽퓞,
        strcat(filename, uri); // ./uri 媛� �맂�떎.
        return 0;
    }
}

//�겢�씪�씠�뼵�듃�뿉寃� �삤瑜� 蹂닿퀬 �븳�떎
void clienterror(int fd, char* cause, char* errnum,char* shortmsg, char* longmsg)
{
    char buf[MAXLINE], body[MAXBUF];
    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);
    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

//�슂泥� �뿤�뜑 �씫湲�
void read_requesthdrs(rio_t* rp)
{
    char buf[MAXLINE];
    Rio_readlineb(rp, buf, MAXLINE); // 踰꾪꽣�뿉�꽌 MAXLINE 源뚯�� �씫湲�
    while (strcmp(buf, "\r\n")) {  //�걹以� �굹�삱 �븣 源뚯�� �씫�뒗�떎
        Rio_readlineb(rp, buf, MAXLINE);
    }
    return;
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response*/
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf)); //�솢 �몢踰� �벐�굹�슂?
    sprintf(buf,"Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    //fd�뒗 �젙蹂대�� 諛쏆옄留덉옄 �쟾�넚�븯�굹�슂???
    //�겢�씪�씠�뼵�듃�뒗 �꽦怨듭쓣 �븣�젮二쇰뒗 �쓳�떟�씪�씤�쓣 蹂대궡�뒗 寃껋쑝濡� �떆�옉�븳�떎.
    if (Fork() == 0) { //����씠�땲�뒗 �옄�떇�봽濡쒖꽭�뒪瑜� �룷�겕�븯怨� �룞�쟻 而⑦뀗痢좊�� �젣怨듯븳�떎.
        setenv("QUERY_STRING", cgiargs, 1);
        //�옄�떇��� QUERY_STRING �솚寃쎈���닔瑜� �슂泥� uri�쓽 cgi�씤�옄濡� 珥덇린�솕 �븳�떎.  (15000 & 213)
        Dup2(fd, STDOUT_FILENO); //�옄�떇��� �옄�떇�쓽 �몴以� 異쒕젰�쓣 �뿰寃� �뙆�씪 �떇蹂꾩옄濡� �옱吏��젙�븯怨�,

        Execve(filename, emptylist, environ);
        // 洹� �썑�뿉 cgi�봽濡쒓렇�옩�쓣 濡쒕뱶�븯怨� �떎�뻾�븳�떎.
        // �옄�떇��� 蹂몄씤 �떎�뻾 �뙆�씪�쓣 �샇異쒗븯湲� �쟾 議댁옱�븯�뜕 �뙆�씪怨�, �솚寃쎈���닔�뱾�뿉�룄 �젒洹쇳븷 �닔 �엳�떎.

    }
    Wait(NULL); //遺�紐⑤뒗 �옄�떇�씠 醫낅즺�릺�뼱 �젙由щ릺�뒗 寃껋쓣 湲곕떎由곕떎.
}

// fd �쓳�떟諛쏅뒗 �냼耳�(�뿰寃곗떇蹂꾩옄), �뙆�씪 �씠由�, �뙆�씪 �궗�씠利�
void serve_static(int fd, char *filename, int filesize){
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
    /* send response headers to client*/
    //�뙆�씪 �젒誘몄뼱 寃��궗�빐�꽌 �뙆�씪�씠由꾩뿉�꽌 ����엯 媛�吏�怨�
    get_filetype(filename, filetype);  // �젒誘몄뼱瑜� �넻�빐 �뙆�씪 ����엯 寃곗젙�븳�떎.
    // �겢�씪�씠�뼵�듃�뿉寃� �쓳�떟 以꾧낵 �쓳�떟 �뿤�뜑 蹂대궦�떎湲�
    // �겢�씪�씠�뼵�듃�뿉寃� �쓳�떟 蹂대궡湲�
    // �뜲�씠�꽣瑜� �겢�씪�씠�뼵�듃濡� 蹂대궡湲� �쟾�뿉 踰꾪띁濡� �엫�떆濡� 媛�吏�怨� �엳�뒗�떎.
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer : Tiny Web Server \r\n", buf);
    sprintf(buf, "%sConnection : close \r\n", buf);
    sprintf(buf, "%sConnect-length : %d \r\n", buf, filesize);
    sprintf(buf, "%sContent-type : %s \r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));

    //rio_readn��� fd�쓽 �쁽�옱 �뙆�씪 �쐞移섏뿉�꽌 硫붾え由� �쐞移� usrbuf濡� 理쒕�� n諛붿씠�듃瑜� �쟾�넚�븳�떎.
    //rio_writen��� usrfd�뿉�꽌 �떇蹂꾩옄 fd濡� n諛붿씠�듃瑜� �쟾�넚�븳�떎.
    //�꽌踰꾩뿉 異쒕젰
    printf("Response headers : \n");
    printf("%s", buf);

    //�씫�쓣 �닔 �엳�뒗 �뙆�씪濡� �뿴湲�
    srcfd = Open(filename, O_RDONLY, 0); //open read only �씫怨�
    //PROT_READ -> �럹�씠吏��뒗 �씫�쓣 �닔留� �엳�떎.
    // �뙆�씪�쓣 �뼱�뼡 硫붾え由� 怨듦컙�뿉 ����쓳�떆�궎怨� 泥レ＜�냼瑜� 由ы꽩
    // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); //硫붾え由щ줈 �꽆湲곌퀬

    srcp = (char*)Malloc(filesize);
    Rio_readn(srcfd, srcp, filesize); 

    //留ㅽ븨�쐞移�, 留ㅽ븨�떆�궗 �뙆�씪�쓽 湲몄씠, 硫붾え由� 蹂댄샇�젙梨�, �뙆�씪怨듭쑀�젙梨�,srcfd ,留ㅽ븨�븷�븣 硫붾え由� �삤�봽�뀑
//    srcp = malloc(sizeof(srcfd));

    Close(srcfd);//�떕湲�


    Rio_writen(fd, srcp, filesize);
    // mmap() �쑝濡� 留뚮뱾�뼱吏� -留듯븨�쓣 �젣嫄고븯湲� �쐞�븳 �떆�뒪�뀥 �샇異쒖씠�떎
    // ����쓳�떆�궓 ����꽍�쓣 ����뼱以��떎. �쑀�슚�븯吏� �븡��� 硫붾え由щ줈 留뚮벀
    // void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
    // int munmap(void *start, size_t length);
    // Munmap(srcp, filesize); //硫붾え由� �빐�젣
    free(srcp);
}

void get_filetype(char *filename, char *filetype){
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filename, "image/.png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mp4"))
        strcpy(filetype, "video/mp4");
    else
        strcpy(filetype, "text/plain");

}


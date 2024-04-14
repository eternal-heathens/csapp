#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400


void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char * host, char *filename, char *port) ;
void get_filetype(char *filename, char *filetype);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void sigpipe_handler(int sig);

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int main(int argc, char **argv)
{
    
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    char servename[MAXLINE], serveport[MAXLINE];
    char buf[MAXLINE],rbuf[MAXLINE],wbuf[MAXBUF];
    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    char filetype[MAXLINE],filename[MAXLINE], cgiargs[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    int clientfd;
    rio_t rios,rioc;
    

    Signal(SIGPIPE,sigpipe_handler);

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);

        /* Read request line and headers */
        Rio_readinitb(&rios, connfd);
        if (!Rio_readlineb(&rios, buf, MAXLINE))  //line:netp:doit:readrequest
            return;
        printf("%s", buf);
        sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
 
        if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
            clienterror(connfd, method, "501", "Not Implemented",
                        "Tiny does not implement this method");
            return;
        }        
         
        /* Parse URI from GET request */
        if(parse_uri(uri, servename,filename,serveport) < 0){
            return;
        }       
        
        // 构建对服务器的请求
        sprintf(wbuf, "%s %s %s\r\n", method, filename, version); 
        sprintf(wbuf, "%sHost：%s\r\n",wbuf,servename); //line:netp:servestatic:beginserve
        sprintf(wbuf, "%s%s",wbuf,user_agent_hdr); //line:netp:servestatic:beginserve
        sprintf(wbuf, "%sConnection: close \r\n",wbuf); //line:netp:servestatic:beginserve
        sprintf(wbuf, "%sProxy-Connection: close\r\n",wbuf); //line:netp:servestatic:beginserve
        sprintf(wbuf, "%s\r\n",wbuf); //line:netp:servestatic:beginserve
        //打开与服务器的链接
        clientfd = Open_clientfd(servename,serveport);

        
        Rio_writen(clientfd,wbuf,strlen(wbuf));
        //读取服务器返回，再返回给客户端
        Rio_readinitb(&rioc, clientfd);
        int n;
        while((n= Rio_readlineb(&rioc,rbuf,MAXLINE)) !=0){
            printf("proxy received %d bytes,then send\n", (int)n);
            //不能用strlen() 每次比较一个机器字（4字节）直到机器字中有0内容字节就停止比较
            // printf("%d \n",strlen(rbuf));
            // printf("%d \n",n);
            Rio_writen(connfd, rbuf,n);
        }//line:netp:servestatic:endserve
        Close(clientfd);
        Close(connfd);                                            //line:netp:tiny:close
    }
}

void sigpipe_handler(int sig){
    printf("sigpipe_handler");
    return;
}

/*
 * read_requesthdrs - read HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
	Rio_readlineb(rp, buf, MAXLINE);
	printf("%s", buf);
    }
    return;
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
	strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else
	strcpy(filetype, "text/plain");
}  


/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char * host, char *filename, char *port) 
{
    char * temphost[MAXLINE];
    if(sscanf(uri, "HTTP://%[^/]%s", temphost, filename ) < 2){
        if(sscanf(uri, "HTTPS://%[^/]%s", temphost, filename ) < 2){
            if(sscanf(uri, "http://%[^/]%s", temphost, filename ) < 2){
                if(sscanf(uri, "https://%[^/]%s", temphost, filename ) < 2){
                        return -1;
                }
            }
        }
    }
    int snum = sscanf(temphost, "%[^:]:%s", host, port);
    if(snum == 1){
        port = "80";
    }else if(snum == 0){
        return -1;
    }
	return 0;
}

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */
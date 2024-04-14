#include <stdio.h>
#include "csapp.h"
#include "sbuf.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define NTHREADS 4
#define SBUFSIZE 16
#define MAX_CACHE_LENGTH 10

typedef struct
{
    // map 的形式
    char url[MAXLINE];
    char obj[MAX_OBJECT_SIZE];

    int lru;
    size_t read_cnt;

    sem_t mutex; //读者读互斥锁
    sem_t w; // 读写互斥锁

} block;


typedef struct
{
    block data[MAX_CACHE_LENGTH];
} Cache;


Cache cache;


void read_requesthdrs(rio_t *rp);
int parse_uri(char *url, char * host, char *filename, char *port) ;
void get_filetype(char *filename, char *filetype);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void sigpipe_handler(int sig);
void *proxy_consume_thread(void *vargp);
void proxy_consume(int connfd);
void init_cache();
int get_block_index(char * url);
void write_cache(char* url,char* cache_data );
void update_LRU(int index);
int get_write_index(char* url);
sbuf_t sbuf;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int main(int argc, char **argv)
{
    
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];

    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    pthread_t tid;
    

    Signal(SIGPIPE,sigpipe_handler);

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }


    listenfd = Open_listenfd(argv[1]);
    init_cache();   
    
    sbuf_init(&sbuf,SBUFSIZE);
    for(int i = 0; i < NTHREADS; i++){
        Pthread_create(&tid,NULL,proxy_consume_thread,NULL);
    }

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        sbuf_insert(&sbuf,connfd);            
    }
}

/**
 * 具体消费逻辑
*/
void proxy_consume(int connfd){
    char servename[MAXLINE], serveport[MAXLINE];
    char buf[MAXLINE],rbuf[MAXLINE],wbuf[MAXBUF];
    char method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char filetype[MAXLINE],filename[MAXLINE], cgiargs[MAXLINE];
    int clientfd;
    rio_t rios,rioc;
    /* Read request line and headers */
    Rio_readinitb(&rios, connfd);
    if (!Rio_readlineb(&rios, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, url, version);       //line:netp:doit:parserequest

    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(connfd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }  

        /* Parse url from GET request */
    if(parse_uri(url, servename,filename,serveport) < 0){
        return;
    }    


    //获取对应的block id，有空闲读者位置则加锁读取
    int block_id = get_block_index(url);
    if(block_id != -1){
        // P(&cache.data[block_id].mutex);
        //     cache.data[block_id].read_cnt++;
        //     if(cache.data[block_id].read_cnt == 1){
        //         P(&cache.data[block_id].w);
        //     }
        // V(&cache.data[block_id].mutex);

        Rio_writen(connfd, cache.data[block_id].obj, strlen(cache.data[block_id].obj));

        //读取完，更新lru，减少读者数量
        
        P(&cache.data[block_id].mutex);
            cache.data[block_id].read_cnt--;
            if(cache.data[block_id].read_cnt == 0){
                V(&cache.data[block_id].w);
            }
        V(&cache.data[block_id].mutex);
        update_LRU(block_id);
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
    size_t tem_size = 0;
    char cache_data[MAX_OBJECT_SIZE]; 
    while((n= Rio_readlineb(&rioc,rbuf,MAXLINE)) !=0){
        //Rio_writen 不能用strlen() 每次比较一个机器字（4字节）直到机器字中有0内容字节就停止比较
        // printf("%d \n",strlen(rbuf));
        // printf("%d \n",n);
        tem_size +=n;
        if(tem_size < MAX_OBJECT_SIZE){
            strcat(cache_data,rbuf);
        }
        Rio_writen(connfd, rbuf,n);
    }//line:netp:servestatic:endserve
    Close(clientfd);

    if(tem_size < MAX_OBJECT_SIZE){
        write_cache(url,cache_data);
    }
}

//更新LRU
void update_LRU(int index)
{
    P(&cache.data[index].w);
    cache.data[index].lru--;
    V(&cache.data[index].w);
   
}

void write_cache(char* url,char* cache_data ){
    int i = get_write_index(url);

    P(&cache.data[i].mutex);
        cache.data[i].read_cnt--;
        if(cache.data[i].read_cnt == 0){
            V(&cache.data[i].w);
        }
    V(&cache.data[i].mutex);
    
    printf("write_cache[%d]:%s \n",i,url);
    printf("write_cache read_cnt:%d\n",cache.data[i].read_cnt);
    P(&cache.data[i].w);
    //写入内容
    printf("write_cache read_cnt:%d\n",cache.data[i].read_cnt);
    strcpy(cache.data[i].obj, cache_data);
    strcpy(cache.data[i].url, url);
    cache.data[i].lru = __INT_MAX__-1;
    printf("[%d]lru:%d \n",i,cache.data[i].lru);
    V(&cache.data[i].w);
    
}

// 获取写缓存索引
int get_write_index(char* url){

    int max = 0;
    int maxindex = 0;
    for(int i = 0; i < MAX_CACHE_LENGTH; i++){
        //加锁，防止遍历的过程中缓存被修改
        P(&cache.data[i].mutex);
        cache.data[i].read_cnt++;
        if(cache.data[i].read_cnt == 1){
            P(&cache.data[i].w);
        }
        V(&cache.data[i].mutex);

        //只初始化完还没用的
        // printf("[%d]lru:%d \n",i,cache.data[i].lru);
        if(cache.data[i].lru == __INT_MAX__){

            maxindex = i;
            if(maxindex != 0){
                P(&cache.data[i].mutex);
                cache.data[i].read_cnt--;
                if(cache.data[i].read_cnt == 0){
                    V(&cache.data[i].w);
                }
                V(&cache.data[i].mutex);
            }
            break;
        }

        if(cache.data[i].lru > max){
            if(maxindex != 0){
                P(&cache.data[maxindex].mutex);
                cache.data[maxindex].read_cnt--;
                if(cache.data[maxindex].read_cnt == 0){
                    V(&cache.data[maxindex].w);
                }
                V(&cache.data[maxindex].mutex);
            }
            maxindex = i;
            max = cache.data[i].lru;
            continue;
        }
        P(&cache.data[i].mutex);
            cache.data[i].read_cnt--;
            if(cache.data[i].read_cnt == 0){
                V(&cache.data[i].w);
            }
        V(&cache.data[i].mutex);
    }
    return maxindex;
}

void init_cache(){
    for (int i = 0; i < MAX_CACHE_LENGTH; i++){
        cache.data[i].read_cnt = 0;
        cache.data[i].lru = __INT_MAX__;
        Sem_init(&cache.data[i].mutex,0,1);
        Sem_init(&cache.data[i].w,0,1);
    }
}

//获取读缓存索引
int get_block_index(char* url){
    //加锁，防止我匹配到的时候，有人在修改，在读取缓存后解锁
    printf("url: %s   \n",url);
    for(int i = 0; i < MAX_CACHE_LENGTH; i++){
        
        P(&cache.data[i].mutex);
        cache.data[i].read_cnt++;
        if(cache.data[i].read_cnt == 1){
            P(&cache.data[i].w);
        }
        V(&cache.data[i].mutex);
        if(strcmp(cache.data[i].url ,url) == 0){

            printf("accept [%d]url:%s \n",i,url);
            return i;
        } 

        P(&cache.data[i].mutex);
            cache.data[i].read_cnt--;
            if(cache.data[i].read_cnt == 0){
                V(&cache.data[i].w);
            }
        V(&cache.data[i].mutex);
    }
    return -1;
}


void *proxy_consume_thread(void *vargp){
    Pthread_detach(pthread_self());
    while (1)
    {
        int connfd = sbuf_remove(&sbuf);
        proxy_consume(connfd);
        Close(connfd); 
    }
    
}

void sigpipe_handler(int sig){
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
 * parse_uri - parse url into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *url, char * host, char *filename, char *port) 
{
    char  temphost[MAXLINE];
    if(sscanf(url, "HTTP://%[^/]%s", temphost, filename ) < 2){
        if(sscanf(url, "HTTPS://%[^/]%s", temphost, filename ) < 2){
            if(sscanf(url, "http://%[^/]%s", temphost, filename ) < 2){
                if(sscanf(url, "https://%[^/]%s", temphost, filename ) < 2){
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
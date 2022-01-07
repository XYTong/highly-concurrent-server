# Introduction

## Simple Echo Server-Client Code Example
![client_server](https://xiangyutong.eu/wp-content/uploads/2021/12/server_client.png "client_server")
Figure from [Source](http://obeki.com/notes.php?iid=150894002-linux+socket&cid=6)
### Client
```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define SERVER_PORT 3030
#define SERVER_IP 127.0.0.1

int main(int argc, char* argv[]){
    int sockfd;
    char* message;
    int n;
    char buffer[64];
    struct sockaddr_in servaddr;
    if(argc != 2){
        printf("error input\n");
        exit(1);
    }
    message = argv[1];
    printf("message: %s\n", message);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	//clear up
    memset(&servaddr, '\0', sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(SERVER_PORT);
	//connect to server
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    write(sockfd, message, strlen(message));
    n = read(sockfd, buffer, sizeof(buffer)-1);
    if(n > 0){
        buffer[n] = '\0';
        printf("receive: %s\n", buffer);
    }
    else {
        perror("error!");
    }
    printf("finished. \n");
	// tell server to close connection
    close(sockfd);
    return 0;
}
```

### Server
```cpp
#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

#define SERVER_PORT 3030 

int main(void){
    int sock;
    struct sockaddr_in server_addr;
    //creat socket
    sock = socket(AF_INET, SOCK_STREAM, 0); //AF_INET: IPv4
    //clear upp
    bzero(&server_addr, sizeof(server_addr));
    //select protocol family
    server_addr.sin_family = AF_INET;
    //litsen all local IP address
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //bind port
    server_addr.sin_port = htons(SERVER_PORT);
    //bind address in socket
    bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    //define max number of conn
    listen(sock, 128);
    printf("wait conn from client \n");
    int done = 1;
    while(done){
        struct sockaddr_in client;
        int client_sock, len;
        char client_ip[64];
        char buffer[256];
        socklen_t client_addr_len;
        client_sock = accept(sock, (struct sockaddr *)&client, &client_addr_len);
    //print IP address and port of client
        printf("client IP: %s\t port: %d\n", inet_ntop(AF_INET, &client.sin_addr.s_addr, client_ip, sizeof(client_ip)),ntohs(client.sin_port));
        len = read(client_sock, buffer, sizeof(buffer)-1);
        buffer[len] = '\0';
        printf("receive[%d]: %s\n", len, buffer);
		// transform of upper case of letters
        for(int i = 0; i < len; i++) if(buffer[i] >= 'a' && buffer[i] <='z') buffer[i] -= 32;
        len = write(client_sock, buffer, len);
        printf("write finished. Len: %d\n", len);
		// tell client to close connection
        close(client_sock);
    }
    return 0;
}
```
### Bit Order
TCP/IP stream: Big Endian.  
**H**ost **TO** **N**etwork translation. It makes sure the endian of a 32 bit data value is correct (Big endian) for network transport. **N**etwork **TO** **H**ost -- is used by the receiver to ensure that the endian is correct for the receiver's CPU.
```cpp
#include <arpa/inet.h>
uint32_t htonl(uint32_t hstlong);
uint16_t htonl(uint32_t hstshort);
uint32_t ntonl(uint32_t netlong);
uint16_t ntonl(uint32_t netshort);
```
If host is little endian, these function will then translate to Big endian and return. Unless they will return the original value.
### Structure of Socket
**struct sockaddr_in**
Forming Internet (IPv4) Socket Addresses

The most commonly used address family under Linux is the AF_INET family. This gives a socket an IPv4 socket address to allow it to communicate with other hosts over a TCP/IP network. The include file that defines the structure sockaddr_in is defined by the C language statement:
```cpp
#include <netinet/in.h>
```
`struct sockaddr` is used for many network programming function, which were born before IPv4. Now sockaddr has turned into (void* )-like function, which passes by a address to function. struct sockaddr and struct sockaddr_in are two structures used to handle the addresses of network traffic.
In various system calls or functions, these two structures are used whenever dealing with network addresses.
![Conparision between sockaddr and sockaddr_in](https://xiangyutong.eu/wp-content/uploads/posts_image/socket.jpg "Conparision between sockaddr and sockaddr_in")
Following are the data structures of sockaddr:
```cpp
struct sockaddr{
	sa_family_t sa_family; /* address family, AF_xxx */
	char sa_data[14];      /* 14 bytes of protocol address */
};

struct sockaddr_in{
	sa_family_t sin_family;   /* address family: AF_INET */
	in_port_t sin_port;       /* port in network byte order */
	struct in_addr sin_addr;  /* internet address */
}
/* internet address */
struct in_addr{
	uint32_t s_addr;          /* address in network byte order */
}
```
type cast: `(struct sockaddr* )&server_addr`
### IP address conversion
```cpp
#include <arpa/inet.h>
int inet_pton(int af, const char* src, void* dst);
const char* inet_ntop(int af, const void* src, char* dst, socklen_t, size);
```
where `af` can be assigned as `AF_INET` and `AF_INET6` that correspond to IPv4 and IPv6. 
### Functions
1. **socket**
socket() creates an endpoint for communication and returns a file
descriptor that refers to that endpoint.  The file descriptor
returned by a successful call will be the lowest-numbered file
descriptor not currently open for the process.
```cpp
#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
/* domain
Name         Purpose                                    Man page
       AF_UNIX      Local communication                        unix(7)
       AF_LOCAL     Synonym for AF_UNIX
       AF_INET      IPv4 Internet protocols                    ip(7)
       AF_AX25      Amateur radio AX.25 protocol               ax25(4)
       AF_IPX       IPX - Novell protocols
       AF_APPLETALK AppleTalk                                  ddp(7)
       AF_X25       ITU-T X.25 / ISO-8208 protocol             x25(7)
       AF_INET6     IPv6 Internet protocols                    ipv6(7)
       AF_DECnet    DECet protocol sockets
       AF_KEY       Key management protocol, originally
                    developed for usage with IPsec
       AF_NETLINK   Kernel user interface device               netlink(7)
       AF_PACKET    Low-level packet interface                 packet(7)
       AF_RDS       Reliable Datagram Sockets (RDS) protocol   rds(7)
                                                               rds-rdma(7)
       AF_PPPOX     Generic PPP transport layer, for setting
                    up L2 tunnels (L2TP and PPPoE)
       AF_LLC       Logical link control (IEEE 802.2 LLC)
                    protocol
       AF_IB        InfiniBand native addressing
       AF_MPLS      Multiprotocol Label Switching
       AF_CAN       Controller Area Network automotive bus
                    protocol
       AF_TIPC      TIPC, "cluster domain sockets" protocol
       AF_BLUETOOTH Bluetooth low-level socket protocol
       AF_ALG       Interface to kernel crypto API
       AF_VSOCK     VSOCK (originally "VMWare VSockets")       vsock(7)
                    protocol for hypervisor-guest
                    communication
       AF_KCM       KCM (kernel connection multiplexer)
                    interface
       AF_XDP       XDP (express data path) interface
type:
       SOCK_STREAM
              Provides sequenced, reliable, two-way, connection-based
              byte streams.  An out-of-band data transmission mechanism
              may be supported.

       SOCK_DGRAM
              Supports datagrams (connectionless, unreliable messages of
              a fixed maximum length).

       SOCK_SEQPACKET
              Provides a sequenced, reliable, two-way connection-based
              data transmission path for datagrams of fixed maximum
              length; a consumer is required to read an entire packet
              with each input system call.
       SOCK_RAW
              Provides raw network protocol access.

       SOCK_RDM
              Provides a reliable datagram layer that does not guarantee
              ordering.

       SOCK_PACKET
              Obsolete and should not be used in new programs; see
              packet(7).
protocol:
       The protocol specifies a particular protocol to be used with the
       socket.  Normally only a single protocol exists to support a
       particular socket type within a given protocol family, in which
       case protocol can be specified as 0.  However, it is possible
       that many protocols may exist, in which case a particular
       protocol must be specified in this manner.  The protocol number
       to use is specific to the “communication domain” in which
       communication is to take place
return:
       On success, a file descriptor for the new socket is returned.  On
       error, -1 is returned, and errno is set to indicate the error.
```
see more in [manual](https://man7.org/linux/man-pages/man2/socket.2.html)
2. **bind**
```cpp
#include <sys/type.h>
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
```
bind() assigns the address specified by `addr` to the socket referred to by the file descriptor `sockfd`.  `addrlen` specifies the size, in bytes, of the address structure pointed to by addr. Traditionally, this operation is called “assigning a name to a socket”.
3. **listen**
listen() marks the socket referred to by sockfd as a passive socket, that is, as a socket that will be used to accept incoming connection requests using accept(2).
```cpp
#include <sys/types.h>
#include <sys/socket.h>
int listen(int sockfd, int backlog);
```
The `sockfd` argument is a file descriptor that refers to a socket of type SOCK_STREAM or SOCK_SEQPACKET.
The `backlog` argument defines the maximum length to which the queue of pending connections for sockfd may grow.  If a connection request arrives when the queue is full, the client may receive an error with an indication of ECONNREFUSED or, if the underlying protocol supports retransmission, the request may be ignored so that a later reattempt at connection succeeds.
4. **accept**
The `accept()`` system call is used with connection-based socket types (SOCK_STREAM, SOCK_SEQPACKET).  It extracts the first connection request on the queue of pending connections for the listening socket, sockfd, creates a new connected socket, and returns a new file descriptor referring to that socket.  The newly created socket is not in the listening state.  The original socket sockfd is unaffected by this call.
```cpp
#include <sys/socket.h>
#include <sys/types.h>
int accept(int sockfd, struct sockaddr *restrict addr, 
		   socklen_t *restrict addrlen);
```
```cpp
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sys/socket.h>
int accept4(int sockfd, struct sockaddr *restrict addr, 
			socklen_t *restrict addrlen, int flags);
```
The argument `sockfd` is a socket that has been created with socket(2), bound to a local address with bind(2), and is listening for connections after a listen(2). The argument `addr` is a pointer to a sockaddr structure.  This structure is filled in with the address of the peer socket, as known to the communications layer.  The exact format of the address returned `addr` is determined by the socket's address family (see socket(2) and the respective protocol man pages). When addr is NULL, nothing is filled in; in this case, `addrlen` is not used, and should also be NULL. The `addrlen` argument is a value-result argument: the caller must initialize it to contain the size (in bytes) of the structure pointed to by addr; on return it will contain the actual size of the peer address. The returned address is truncated if the buffer provided is too small; in this case, addrlen will return a value greater than was supplied to the call.
5. **connect**
```cpp
#include <sys/socket.h>
int connect(int sockfd, const struct sockaddr *addr,
			socklen_t addrlen);
```
The `connect()` system call connects the socket referred to by the file descriptor `sockfd` to the address specified `by addr`.  The `addrlen` argument specifies the size of `addr`.  The format of the address in `addr` is determined by the address space of the socket `sockfd`; see socket(2) for further details.  
If the socket `sockfd` is of type `SOCK_DGRAM`, then `addr` is the address to which datagrams are sent by default, and the only address from which datagrams are received.  If the socket is of type `SOCK_STREAM` or `SOCK_SEQPACKET`, this call attempts to make a connection to the socket that is bound to the address specified
by `addr`.
## Mini HTTP SERVER
![HTTP Protocol](https://xiangyutong.eu/wp-content/uploads/posts_image/http.png "HTTP Protocol")
> The Hypertext Transfer Protocol (HTTP) is an application layer protocol in the Internet protocol suite model for distributed, collaborative, hypermedia information systems. HTTP is the foundation of data communication for the World Wide Web, where hypertext documents include hyperlinks to other resources that the user can easily access, for example by a mouse click or by tapping the screen in a web browser. (WiKi)
**HTTP Request**
![structure of http request](https://xiangyutong.eu/wp-content/uploads/posts_image/http_s.png "structure of http request")
**HTTP Response**
![HTTP Response](https://xiangyutong.eu/wp-content/uploads/posts_image/http-response.png "HTTP Response")
**stat, fstat, lstat - get file status**
```cpp
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *path, struct stat *buf);
/*These functions return information about a file. No permissions are required on the file itself, but-in the case of stat() and lstat() - execute (search) permission is required on all of the directories in path that lead to the file.
stat() stats the file pointed to by path and fills in buf.
*/
struct stat {
    dev_t     st_dev;     /* ID of device containing file */
    ino_t     st_ino;     /* inode number */
    mode_t    st_mode;    /* protection */
    nlink_t   st_nlink;   /* number of hard links */
    uid_t     st_uid;     /* user ID of owner */
    gid_t     st_gid;     /* group ID of owner */
    dev_t     st_rdev;    /* device ID (if special file) */
    off_t     st_size;    /* total size, in bytes */
    blksize_t st_blksize; /* blocksize for file system I/O */
    blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
    time_t    st_atime;   /* time of last access */
    time_t    st_mtime;   /* time of last modification */
    time_t    st_ctime;   /* time of last status change */
}; 
```
**pthread_create - create a new thread**
The pthread_create() function starts a new thread in the calling process. The new thread starts execution by invoking start_routine(); arg is passed as the sole argument of start_routine().
```cpp
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg);
```
**main file**
```cpp
#include <stdio.h> 
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

#define SERVER_PORT 80 // port 80 for http 

static int debug = 1;

int get_line(int sock, char* buffer, int size);

void* handle_http_request(void* pclient_sock);
void do_http_response(int client_sock, const char* path);
int headers(int client_sock, FILE* resource);
void cat(int client_sock, FILE* resource);
void not_found(int client_sock); // 404
void inner_error(int client_sock); // 500
void unimplemented(int client_sock); //501
void bad_request(int client_sock); //400


int main(void){
    int sock;
    struct sockaddr_in server_addr;
    //creat socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    //clear upp
    bzero(&server_addr, sizeof(server_addr));
    //select protocol family
    server_addr.sin_family = AF_INET;
    //litsen all local IP address
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //bind port
    server_addr.sin_port = htons(80);
    //bind address in socket
    if(bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) perror("bind failded. Error");
    //define max number of conn
    listen(sock, 128);
    printf("wait conn from client \n");
    int done = 1;
    while(done){
        struct sockaddr_in client;
        int client_sock, len;
        char client_ip[64];
        char buffer[256];
        pthread_t id;
        int* pclient_sock = NULL;
        socklen_t client_addr_len;
        client_sock = accept(sock, (struct sockaddr *)&client, &client_addr_len);
        //print IP address and port of client
        printf("client IP: %s\t port: %d\n", inet_ntop(AF_INET, &client.sin_addr.s_addr, client_ip, sizeof(client_ip)),ntohs(client.sin_port));
        // create new thread
        pclient_sock = (int*)malloc(sizeof(int));
        *pclient_sock = client_sock;
        //handle http request
        pthread_create(&id, NULL, handle_http_request, (void*)pclient_sock);
    }
    close(sock);
    return 0;
}

// **** http parses request ****
void* handle_http_request(void* pclient_sock){
    int len = 0;
    char buffer[256];
    char method[64];
    char url[256];
    char path[512];
    int client_sock = *(int*)pclient_sock;
    struct stat st;
    // read http request from client
    // 1. read request line
    len = get_line(client_sock, buffer, sizeof(buffer));
    if(len > 0){ // read request line successfully
        int i = 0;
        int j = 0;
        while(!isspace(buffer[j]) && (i < sizeof(method)-2)){
            method[i++] = buffer[j++];
        }
        method[i] = '\0';
        printf("request method: %s\n", method);
        if(strncasecmp(method, "GET", i) == 0){
            if(debug) printf("method = GET\n");
            // obtain url
            while(isspace(buffer[j++]));
            i = 0;
            while(!isspace(buffer[j]) && i < sizeof(url)-1){
                url[i++] = buffer[j++];
            }
            url[i] = '\0';
            if(debug) printf("url: %s\n",url);
            // continue reading request headers
            do{
                len = get_line(client_sock, buffer, sizeof(buffer));
                if(debug) printf("read: %s\n", buffer);
            }while(len > 0);
            // locate html file
            // handle ? of url
            {
                char* pos = strchr(url, '?');
                if (pos) {
                    *pos = '\0';
                    //printf("real url: %s\n", url);
                }
            }
            sprintf(path, "./html_docs/%s", url);
            if(debug) printf("path: %s\n", path);

            // do http response
            // check if corresponding html file exists. If yes, response 200 OK then send the file, else response 404 NOT FOUND.
            if(stat(path, &st) == -1){
                fprintf(stderr, "stat %s failed. reason: %s\n", path, strerror(errno));
                not_found(client_sock);
            }else{
                if(S_ISDIR(st.st_mode)){
                    // append default html file if path is a folder
                    strcat(path, "/index.html");
                }
                do_http_response(client_sock, path);
            }
        }else{
            // not GET request, read http headers and response client of 501 Method Not Implemented
            fprintf(stderr, "Warning! Other_request [%s]\n", method);
            do{
                len = get_line(client_sock, buffer, sizeof(buffer));
                if(debug) printf("read: %s\n", buffer);
            }while(len > 0);
            // request not implemented
            unimplemented(client_sock);
        }

    }else{
        // bad request 400
        bad_request(client_sock);
    }
    close(client_sock);
    if(pclient_sock) free(pclient_sock);
    return NULL;
}

// **** http response ****
void do_http_response(int client_sock, const char* path){
    int ret = 0;
    FILE* resource = NULL;
    resource = fopen(path, "r");
    if(resource == NULL){
        not_found(client_sock);
        return;
    }
    // 1. send http headers
    headers(client_sock, resource);
    // 2. send http body
    if(!ret) cat(client_sock, resource);
    fclose(resource);
}
// send html content
void cat(int client_sock, FILE* resource){
    char buffer[1024];
    fgets(buffer, sizeof(buffer), resource);
    while(!feof(resource)){
        int len = write(client_sock, buffer, strlen(buffer));
        // error occurs when sending body
        if(len < 0){
            fprintf(stderr, "send body error. reason: %s,\n", strerror(errno));
            break;
        }
        if(debug) fprintf(stdout, "%s", buffer);
        fgets(buffer, sizeof(buffer), resource);
    }
}
/*******************************************
 * return http headers of resonsed file
 * enter:
 *      client_sock
 *      resource
 *return:
        0, success
        -1, failed
 * ****************************************/
int headers(int client_sock, FILE* resource){
    printf("========================================================================\n");
    struct stat st;
    int file_id = 0;
    char tmp[64];
    char buffer[1024];
    strcpy(buffer, "HTTP/1.0 200 OK\r\n");
    strcat(buffer, "SERVER: Your Server Name\r\n");
    strcat(buffer, "Comtemt-Type:text/html\r\n");
    strcat(buffer, "Connection: Close\r\n");

    file_id = fileno(resource);
    if(fstat(file_id, &st) == -1){
        inner_error(client_sock);
        return -1;
    }
    snprintf(tmp, 64, "Content-Length: %ld\r\n\r\n", st.st_size);
    strcat(buffer, tmp);
    if(debug) fprintf(stdout, "header: %s\n", buffer);
    if(send(client_sock, buffer, strlen(buffer), 0) < 0){
        fprintf(stderr, "send failed. data: %s, reason: %s\n", buffer, strerror(errno));
        return -1;
    }
    return 0;
}
/*
void do_http_response(int client_sock){
    const char* main_header = "HTTP/1.0 200 OK\r\nSERVER: Your Server Name\r\nCpmtemt-Type:text/html\r\nConnection: Close\r\n";
    const char* content = "\
        <html lang = \"en-US\">\n\
        <head>\n\
        <meta content = \"text/html; charset = utf-8\" http-equiv = \"Content-Type\">\n\
        <title> This is a title </title>\n\
        </head>\n\
        <body>\n\
        <div align =  center height = \"500px\">\n\
        <br/><br/><br/>\n\
        <h2>welcome!</h2></br></br></br>\n\
        <form action = \"commit\" method = \"post\">\n\
        Name: <input type = \"text\" name = \"name\"/>\n\
        <br/> Age: <input type = \"password\" name = \"age\"/>\n\
        <br/><br/><br/> <input type = \"submit\" value = \"submit\" />\n\
        <input type = \"reset\" value = \"reset\"/>\n\
        </form>\n\
        </div>\n\
        </body>\n\
        </html>\n";
    // 1. send main_header
    int len = write(client_sock, main_header, strlen(main_header));
    if(debug) fprintf(stdout, "... do_http_response...\n");
    if(debug) fprintf(stdout, "write[%d] %s", len, main_header);
    // 2. generate Content-Length line and send
    char send_buffer[64];
    int content_len = strlen(content);
    len = snprintf(send_buffer, 64, "Content-Length: %d\r\n\r\n", content_len);
    len = write(client_sock, send_buffer, len);
    if(debug) fprintf(stdout, "write[%d]: %s", len, send_buffer);
    // 3. send html file content
    len = write(client_sock, content, content_len);
    if(debug) fprintf(stdout, "write[%d]: %s", len, content);
}
*/

int get_line(int sock, char* buffer, int size){
    int cnt = 0;
    char ch = '\0';
    int len = 0;
    while((cnt < size-1) && ch != '\n'){
        len = read(sock, &ch, 1);
        if(len == 1){
            if(ch == '\r') continue;
            else if (ch == '\n'){
                buffer[cnt] = '\0';
                break;
            }
            //handle for normal chars
            buffer[cnt] = ch;
            cnt++;
        }
        else if (len == -1){ //read error
            cnt = -1;
            perror("read failed");
            break;
        }else{ // return 0, close sock
            cnt = -1;
            fprintf(stderr, "client close.\n");
        }
    }
    if(cnt >= 0) buffer[cnt] = '\0';
    return cnt;
}

void not_found(int client_sock){
    const char* reply = "HTTP/1.0 404 NOT FOUND\r\n\
    Content-Type: text/html\r\n\
    \r\n\
    <HTML lang = \"en-US\">\r\n\
    <meta content = \"text/html; charset = utf-8\" http-equiv = \"Content-Type\">\r\n\
    <HEAD>\r\n\
    <TITLE> NOT FOUND </TITLE>\r\n\
    </HEAD>\r\n\
    <BODY>\r\n\
    <P> File not exists!\r\n\
    <P> The server could not fulfill your request because the resource specified is unavailable or nonexistent. \r\n\
    </BODY>\r\n\
    </HTML>";
    int len = write(client_sock, reply, strlen(reply));
    if(debug) fprintf(stdout, reply);
    if(len <= 0){
        fprintf(stderr, "send reply failed. reason: %s\n", strerror(errno));
    }
}

void inner_error(int client_sock){
    const char* reply = "HTTP/1.0 500 Internal Server Error\r\n\
    Content-Type: text/html\r\n\
    \r\n\
    <HTML>\r\n\
    <HEAD>\r\n\
    <TITLE> Inner Error </TITLE>\r\n\
    </HEAD>\r\n\
    <BODY>\r\n\
    <P> Internal Server Error. \r\n\
    </BODY>\r\n\
    </HTML>";
    int len = write(client_sock, reply, strlen(reply));
    if(debug) fprintf(stdout, reply);
    if(len <= 0){
        fprintf(stderr, "send reply failed. reason: %s\n", strerror(errno));
    }
}

void unimplemented(int client_sock){
    const char* reply = "HTTP/1.0 501 Method Not Implemented\r\n\
    Content-Type: text/html\r\n\
    \r\n\
    <HTML lang = \"en-US\">\r\n\
    <meta content = \"text/html; charset = utf-8\" http-equiv = \"Content-Type\">\r\n\
    <HEAD>\r\n\
    <TITLE> Mehtod not Implemented </TITLE>\r\n\
    </HEAD>\r\n\
    <BODY>\r\n\
    <P> HTTP request method not supported.\r\n\
    </BODY>\r\n\
    </HTML>";
    int len = write(client_sock, reply, strlen(reply));
    if(debug) fprintf(stdout, reply);
    if(len <= 0){
        fprintf(stderr, "send reply failed. reason: %s\n", strerror(errno));
    }
}

void bad_request(int client_sock){
    const char* reply = "HTTP/1.0 400 BAD REQUEST\r\n\
    Content-Type: text/html\r\n\
    \r\n\
    <HTML lang = \"en-US\">\r\n\
    <meta content = \"text/html; charset = utf-8\" http-equiv = \"Content-Type\">\r\n\
    <HEAD>\r\n\
    <TITLE> BAD REQUEST </TITLE>\r\n\
    </HEAD>\r\n\
    <BODY>\r\n\
    <P> Your browser sent a bad request.\r\n\
    </BODY>\r\n\
    </HTML>";
    int len = write(client_sock, reply, strlen(reply));
    if(debug) fprintf(stdout, reply);
    if(len <= 0){
        fprintf(stderr, "send reply failed. reason: %s\n", strerror(errno));
    }
}
```
## Tiny Web Server
1. thread pool + non-blocking socket + epoll (both ET and LT are implemented) + event processing (both Reactor and analog Proactor are implemented) concurrency model
2. Use the state machine to parse HTTP request messages, support parsing GET and POST requests
3. Access the server database to realize the user registration and login functions on the web side, and can request server pictures and video files
4. Realize a synchronous/asynchronous log system to record the running status of the server
5. The Webbench stress test can achieve tens of thousands of concurrent connection data exchanges

### Threads, Locks and Synchronization
to be continue...

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
    sock = socket(AF_INET, SOCK_STREAM, 0);
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
        for(int i = 0; i < len; i++) if(buffer[i] >= 'a' && buffer[i] <='z') buffer[i] -= 32;
        len = write(client_sock, buffer, len);
        printf("write finished. Len: %d\n", len);
        close(client_sock);
    }
    return 0;
}

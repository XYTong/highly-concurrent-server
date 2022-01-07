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
    memset(&servaddr, '\0', sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(SERVER_PORT);
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
    close(sockfd);
    return 0;
}


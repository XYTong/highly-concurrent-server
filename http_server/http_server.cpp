#include <stdio.h> 
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>

#define SERVER_PORT 80 // port 80 for http 

static int debug = 1;

int get_line(int sock, char* buffer, int size);

void handle_http_request(int client_sock);
void do_http_response(int client_sock, const char* path);
void headers(int client_sock, FILE* resource);
void cat(int client_sock, FILE* resource);
void not_found(int client_sock);
void inner_error(int client_sock);


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
        socklen_t client_addr_len;
        client_sock = accept(sock, (struct sockaddr *)&client, &client_addr_len);
        //print IP address and port of client
        printf("client IP: %s\t port: %d\n", inet_ntop(AF_INET, &client.sin_addr.s_addr, client_ip, sizeof(client_ip)),ntohs(client.sin_port));
        //handle http request
        handle_http_request(client_sock);
        close(client_sock);
    }
    close(sock);
    return 0;
}

// **** http parses request ****
void handle_http_request(int client_sock){
    int len = 0;
    char buffer[256];
    char method[64];
    char url[256];
    char path[512];
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
            // unimplemented(client_sock);
        }

    }else{
        // bad request 400
    }
}

// **** http response ****
void do_http_response(int client_sock, const char* path){
    FILE* resource = NULL;
    resource = fopen(path, "r");
    if(resource == NULL){
        not_found(client_sock);
        return;
    }
    // 1. send http headers
    headers(client_sock, resource);
    // 2. send http body
    cat(client_sock, resource);

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
void headers(int client_sock, FILE* resource){
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
    }
    snprintf(tmp, 64, "Content-Length: %ld\r\n\r\n", st.st_size);
    strcat(buffer, tmp);
    if(debug) fprintf(stdout, "header: %s\n", buffer);
    if(send(client_sock, buffer, strlen(buffer), 0) < 0){
        fprintf(stderr, "send failed. data: %s, reason: %s\n", buffer, strerror(errno));
    }
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

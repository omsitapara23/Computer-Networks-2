#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <netinet/in.h>
#include <sys/types.h> 
#include <string.h>
int main()
{
    char buffer[2048] = {0};
    int read_val;
    char input[1024];
    struct sockaddr_in client_address;
    struct sockaddr_in server_addr;
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    char *welcome = "hello client here";
    if(socket_id  == 0)
    {
        printf("Socket Error\n");
    }

    int result = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    if(result < 0)
        printf("error for inet_pton");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3542);

    int connection = connect(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(connection < 0)
        printf("Connection error\n");

    send(socket_id, welcome, strlen(welcome), 0 );
    bzero(buffer, 2048);
    read_val = recv(socket_id, buffer, 2048, 0);
    printf("server : %s\n", buffer);
    while(strcmp(input, "exit") != 0)
    {
        printf("client : ");
        scanf("%s", input);
        send(socket_id, input, strlen(input), 0 );
        bzero(buffer, 2048);
        read_val = recv(socket_id, buffer, 2048, 0);
        printf("server : %s\n", buffer);
    
    
    }

    close(socket_id);


}
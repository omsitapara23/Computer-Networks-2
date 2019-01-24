#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <netinet/in.h>
#include <sys/types.h> 
#include <string.h>
int main()
{
    char buffer[2048] = {0};
    int read_val;
    struct sockaddr_in server_address;
    char* welcome = "hello server here";
    char input[1024];
    int length = sizeof(server_address);
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_id  == 0)
    {
        printf("Socket Error\n");
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(3542);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(socket_id, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
    {
        printf("bind failed \n");
    }

    int status = listen(socket_id, 2);
    if(status < 0)
    {
        printf("listen error");
    }
    printf("Waiting for clients\n");
    int incoming_socket = accept(socket_id, (struct sockaddr *)&server_address,  
                       (socklen_t*)&length);
    if ((incoming_socket  <0) )
    { 
        printf("error in accept\n");
    } 
    bzero(buffer, 2048);
    read_val = recv(incoming_socket, buffer, 2048,0);
    printf("client : %s\n", buffer);
    printf("server : %s\n", welcome);
    send(incoming_socket, welcome, strlen(welcome), 0);
    while(strcmp(input, "exit") != 0)
    {
        bzero(buffer, 2048);
        read_val = recv(incoming_socket, buffer, 2048,0);
        printf("client : %s\n", buffer);
        printf("server : ");
        scanf("%s", input);
        send(incoming_socket, input, strlen(input), 0);
    }
    close(incoming_socket);
    close(socket_id);
    



}
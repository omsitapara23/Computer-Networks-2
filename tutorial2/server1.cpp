#include <bits/stdc++.h>
#include <string>   //strlen 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <thread>
#include <atomic>
using namespace std;

atomic<int> total_Conn{0};

void runner(int client_id, vector<int> client, int n)
{
    int curr = 1;
    char buffer[1024];
    while(curr)
    {
        bzero(buffer, 1024);
        int read_val = recv(client[client_id], buffer, 1024,0);
        if(read_val == 0)
        {
            cout << "Client " << client_id << " disconnected" << endl;
            close(client[client_id]);
            client[client_id] = 0;
            total_Conn--;
            curr = 0;
        }
        else
        {
            if(strlen(buffer) != 0)
            {
                cout << "Broadcasting : " << buffer << endl;
                for(int i = 0; i < n; i++)
                {
                    if(i != client_id && client[i] != 0)
                    {
                        send(client[i], buffer, strlen(buffer), 0);
                    }
                }
            }
            
        }
        
    }
}
void accepter(vector<int> client, int socket_id, struct sockaddr_in server_address, int length, int n)
{
    int flag  = 1;
    while(total_Conn || flag)
    {
        flag = 0;
        for(int i = 0; i < n; i++)
        {
            if(client[i] == 0)
                client[i] = accept(socket_id, (struct sockaddr *)&server_address,  
                       (socklen_t*)&length);
            
        }
    }
}

int main()
{
    int n;
    cout << "Enter max clients : ";
    cin >> n;
    int server_socket;
    int option = 1;
    int client_socket[n];
    int max_clients = n;
    int activity, i , string_length , curr_soc;
    int sever_address_length , new_socket, max_sd;
    int sender, receiver;
    int channels = 0;
    int marker = 0;


    struct sockaddr_in sever_address;
    char buffer[1025];

    fd_set scoket_descriptor;

    string message;

    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0; 
    } 

    //creating the server socket
    if( (server_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket creation failed");  
        exit(EXIT_FAILURE);  
    } 


    // making the server
    if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&option, 
          sizeof(option)) < 0 )  
    {  
        perror("setsockopt failed");  
        exit(EXIT_FAILURE);  
    }  

    //data structure for the server
    sever_address.sin_family = AF_INET;  
    sever_address.sin_addr.s_addr = INADDR_ANY;  
    sever_address.sin_port = htons( 3542 );  

    //binding the server to listen to respective port
    if (bind(server_socket, (struct sockaddr *)&sever_address, sizeof(sever_address))<0)  
    {  
        perror("bind for socket failed");  
        exit(EXIT_FAILURE);  
    }  

    //lsiten to that socket and 5 is the waiting queue of clients
    if (listen(server_socket, n + 1) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  

    sever_address_length = sizeof(sever_address); 
    int flag = 1;
    //run server loop
    while(total_Conn || flag )  
    {  
        flag = 0;
        //clearing the socket 
        FD_ZERO(&scoket_descriptor);  
    
        //setting the server socket
        FD_SET(server_socket, &scoket_descriptor);  
        max_sd = server_socket;  
            
        //for loop for the client connected
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket for each client
            curr_soc = client_socket[i];  
                
            //if the scoket is readable than add to read list
            if(curr_soc > 0)  
                FD_SET( curr_soc , &scoket_descriptor);  
                
            //we need highest number of fd for select function
            if(curr_soc > max_sd)  
                max_sd = curr_soc;  
        }  
    
        /*
        waiting for activity on the socket,
        here the timeout is set to null so it waits infinetly
        The purpose of this method is to wake up the server if 
        something happens to its socket
        */
        activity = select( max_sd + 1 , &scoket_descriptor , NULL , NULL , NULL);  
        
      
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }  
            
        //checking new connection
        if (FD_ISSET(server_socket, &scoket_descriptor))  
        {  
            if ((new_socket = accept(server_socket, (struct sockaddr *)&sever_address, (socklen_t*)&sever_address_length))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
            
            
            int isget = 0;
            //adding new connection
            for (i = 0; i < max_clients; i++)  
            {  
                //adding to first non-empty position
                if( client_socket[i] == 0 )  
                {  
                    cout << "new connection " << endl;
                    client_socket[i] = new_socket;   
                    total_Conn++;  
                    isget = 1 ;
                    break;  
                }  
            } 
            if(isget == 0)
            {
                string s = "buffer full";
                send(new_socket, s.c_str(), s.length(), 0);
            } 
        }  
            
        //checking each client for IO
        for (i = 0; i < max_clients; i++)  
        {                          // for(int i = 0; i < conns.size(); i++)
                        //     channels[i] = 0;
            curr_soc = client_socket[i];  
                
            if (FD_ISSET( curr_soc , &scoket_descriptor))  
            { 
                //checking if some one disconnected
                
                if ((string_length = read( curr_soc , buffer, 4096)) == 0)  
                {  

                    getpeername(curr_soc , (struct sockaddr*)&sever_address ,(socklen_t*)&sever_address_length);  
                    close(curr_soc);  
                    client_socket[i] = 0; 
                    cout << "Client : " << i << "disconnected " << endl;
                    total_Conn--;
                }  
                    
                //receviving the message came in
                else
                {  
                    cout << "Broadcasting : " << buffer << endl;

                   for(int i = 0; i < n; i++)
                    {
                        buffer[string_length] = '\0';
                        if(client_socket[i] != 0 && curr_soc != client_socket[i])
                            send(client_socket[i], buffer, strlen(buffer), 0);
                    }
                }  
            }  
        }  
    } 

    //closing the server
    close(server_socket);

}




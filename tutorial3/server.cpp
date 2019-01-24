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
#include <chrono>
using namespace std;

void square(int client_socket, float x)
{
    float res = x*x;
    string to_send = to_string(res);
    // this_thread::sleep_for(chrono::seconds(10)); 

    send(client_socket, to_send.c_str(), to_send.length(), 0);   

}

void sum(int client_socket, float array[], int numbers)
{
    float res = 0;
    for(int i = 0; i < numbers; i++)
    {
        res += array[i];
    }
    string to_send = to_string(res);
    send(client_socket, to_send.c_str(), to_send.length(), 0);   
}


void multipleReturn(int client_socket, float array[], int number)
{
    float res1 = array[0] + array[1];
    float res2 = array[0] - array[1];
    string to_send1 = to_string(res1);
    string to_send2 = to_string(res2);
    string to_send = to_send1 + " "  + to_send2;
    send(client_socket, to_send.c_str(), to_send.length(), 0);   

}

atomic<int> total_Conn{0};

struct functions
{
    char fuction[1024];
    int numberoFparams;
    float params[1024];
};

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
    while(1)  
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
            struct functions* func = new functions;
                
            if (FD_ISSET( curr_soc , &scoket_descriptor))  
            { 
                //checking if some one disconnected
                
                if ((string_length = read( curr_soc , func, sizeof(*func))) == 0)  
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
                    
                    if(string(func->fuction).compare("square") == 0)
                    {
                        thread th = thread(square, client_socket[i], func->params[0]);
                        th.detach();
                        
                    }
                    else if(string(func->fuction).compare("sum") == 0)
                    {
                        thread th = thread(sum, client_socket[i], func->params, func->numberoFparams);
                        th.detach();
                    }
                    else
                    {
                        thread th = thread(multipleReturn, client_socket[i], func->params, func->numberoFparams);
                        th.detach();
                    }
                }  
            }  
        }  
    } 

    //closing the server
    close(server_socket);

}




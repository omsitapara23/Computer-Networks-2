#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <mutex>
#include <chrono>
using namespace std;

atomic<int> total_Conn{0};
int id = 0;

void download_send(string website, string object, int client_socket)
{
    cout << "Website : " << website << endl;
    cout << "Object: " << object << endl;
    struct addrinfo info, *address;
    memset(&info, 0, sizeof(info));
    info.ai_family = AF_INET;
    info.ai_socktype = SOCK_STREAM;
    int errcode =  getaddrinfo(website.c_str(), "80", &info, &address); //www.bjp.org
    char buffer[2048];
    int socketid = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    errcode = connect(socketid, address->ai_addr, address->ai_addrlen);
    if(errcode != 0)
    {
        cout << "Error in getaddrinfo" << endl;
        exit(0);
    }
    else
    {
        cout << "Connection Established!!!" << endl;
    }
    if (send(socketid, object.c_str(), object.length(), 0)<0)
    {
        cout << "GET req send fail!!" << endl;
        exit(0);
    }
    else
    {
        int tot_size = 1;
        int rec_bytes = 0;
        int act = 0;
        int curr_bytes;
        // fstream outfile;
        // string extension = ".pdf";
        // outfile.open(to_string(id) + extension, ios::binary | ios::out);
        id++;
        string sends = "";
        while(act < tot_size)
        {
            curr_bytes = recv(socketid, buffer, sizeof(buffer), 0);
            buffer[curr_bytes] = '\0';
            string data = string(buffer);
            cout << "DATA : " << data.length() << endl;
            rec_bytes += curr_bytes;
            if(curr_bytes < 0)
            {
                cout << "recv error!!!" << endl;
                exit(0);
            }

            if(curr_bytes == 0)
                break;

            cout << "Recived bytes :: " <<  curr_bytes << endl;
            if(rec_bytes == curr_bytes)
            {
                cout << data << endl;
                int isthree = data.find("Content-Length: ");
                if(isthree != -1)
                {
                    int index_size = data.find("Content-Length: ") + string("Content-Length: ").length();
                    string size_object = data.substr(index_size, data.find("\n", index_size) - index_size);
                    tot_size = stoi(size_object);

                }
                else
                {
                    cout << "NO contenet length " << endl;
                }
                
                cout<< tot_size << endl;

            }
            int index = data.find("\r\n\r\n") + 4;
            if(index == 3) {
				index = 0;
			}

            for(int i = index; i < curr_bytes; i++)
            {
                sends += buffer[i];
                act++;
            }
            cout << "ACTUAL: " << act << endl;
            char newbuff[curr_bytes];
            for(int i = 0; i < curr_bytes; i++)
            {
                newbuff[i] = buffer[i];
            }
            if(send(client_socket, newbuff, curr_bytes, 0) < 0)
            {
                cout << "Error" << endl;
                exit(0);
            }
            // string bff = "buffer full";
            // if(act >= tot_size)
            // {
            //     if(send(client_socket, bff.c_str(),bff.length(), 0) < 0)
            //     {
            //         cout << "ERRORR" << endl;
            //     }
            //     // break;
            // }
            sends = "";

        }
    }
    close(socketid);

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
    char buffer[2048];

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
    while(flag )  
    {  
        flag = 1;
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
                
                if ((string_length = read( curr_soc , buffer, 2048)) == 0)  
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
                    cout << "Client: requested : " << buffer << endl;
                    cout << "length : " << string_length << endl;

                   string site = "", request = "";
                   int switcher = 0;
                   int k = 0;
                   buffer[string_length] = '\0';
                   while(k<string_length)
                   {
                    //    cout << "gg" << endl;
                        if(buffer[k] == '|')
                        {
                            switcher = 1;
                        }
                        else if(switcher == 0)
                        {
                            site += buffer[k];
                        }
                        else
                        {
                            request += buffer[k];
                        }
                        k++;
                        
                   }
                    cout << "Going to download" << endl;
                   thread downloader_sender = thread(download_send, site, request, client_socket[i]);
                   downloader_sender.detach();
                }  
            }  
        }  
    } 

    //closing the server
    close(server_socket);

}




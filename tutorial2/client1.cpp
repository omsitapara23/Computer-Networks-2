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
atomic<int> flag{0};

void reader(int socket_id)
{
    char buffer[1024];
    while(flag)
    {
        bzero(buffer, 1024);
        int read_val = recv(socket_id, buffer, 1024, 0);
        cout << buffer << endl;
        string s = buffer;
        if(s.compare("buffer full") == 0)
            exit(1);

    }
}


void writer(int socket_id)
{
    string input;
    while(flag)
    {
        cin >> input;
        if(input.compare("exit") == 0)
        {
            cout << "Exiting " << endl;
            close(socket_id);
            exit(1);
        }
        else
        {
             int val = send(socket_id, input.c_str(), input.length(), 0 );
            if(val  < 0)
                cout << "send eroor" << endl;
        }
    }
    
}

int main()
{
    char buffer[1024] = {0};
    int read_val;
    flag  = 1;
    string input;
    struct sockaddr_in client_address;
    struct sockaddr_in server_addr;
    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
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

    thread readerth(reader, socket_id);
    thread writerth(writer, socket_id);

    readerth.join();
    writerth.join();

   
    return 0;

}
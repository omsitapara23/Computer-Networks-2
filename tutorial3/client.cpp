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
atomic<int> flag{0};

struct functions
{
    char funciton[1024];
    int numberoFparams;
    float params[1024];
};

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
    int number;
    float param;
    while(flag)
    {
        struct functions* func = new functions;
        cout << "Enter function name : ";
        cin >> input;
        for(int i = 0; i < input.length(); i++)
        {
            func->funciton[i] = input[i];
        }
        cout << "Enter number of args :";
        cin >> number;
        func->numberoFparams = number;
        for(int i = 0; i < number; i++)
        {
            cin >> param;
            func->params[i] = param;
        }
        if(input.compare("exit") == 0)
        {
            cout << "Exiting " << endl;
            close(socket_id);
            exit(1);
        }
        else
        {
            int val = send(socket_id, func, sizeof(*func), 0 );
            if(val  < 0)
                cout << "send eroor" << endl;
        }
    }
    
}

int main()
{
    char buffer[1024] = {0};
    

    while(1)
    {
        flag  = 1;
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
        string input;
        int number;
        float param;
        struct functions* func = new functions;
        cout << "Enter function name : ";
        cin >> input;
        for(int i = 0; i < input.length(); i++)
        {
            func->funciton[i] = input[i];
        }
        cout << "Enter number of args : ";
        cin >> number;
        func->numberoFparams = number;
        for(int i = 0; i < number; i++)
        {
            cin >> param;
            func->params[i] = param;
        }
        if(input.compare("exit") == 0)
        {
            cout << "Exiting " << endl;
            close(socket_id);
            exit(1);
        }
        else
        {
            int val = send(socket_id, func, sizeof(*func), 0 );
            if(val  < 0)
                cout << "send eroor" << endl;
        }

        bzero(buffer, 1024);
        int read_val = recv(socket_id, buffer, 1024, 0);
        cout << " Answer : " << buffer << endl;
        close(socket_id);
        
        cout << "Going to Sleep" << endl;
        this_thread::sleep_for(chrono::seconds(2)); 


    }


   
    return 0;

}
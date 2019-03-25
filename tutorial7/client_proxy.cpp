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
fstream outfile;
string extension = ".pdf";
void reader(int socket_id)
{
    char buffer[2048];
    int tot_size = 1;
    int rec_bytes = 0;
    int act = 0;
    int curr_bytes = 0;
    // fstream outfile;
    // string extension = ".pdf";
    // outfile.open(to_string(id) + extension, ios::binary | ios::out);
    string sends = "";
    while(act < tot_size)
    {
        curr_bytes = recv(socket_id, buffer, sizeof(buffer), 0);
        buffer[curr_bytes] = '\0';
        string data = string(buffer);
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
            outfile << buffer[i];
            act++;
        }

        cout << "ACTUAL :" << act << endl;
    }

    cout << "REC" << endl;
}


void writer(int socket_id, string object, string website)
{
    std::this_thread::sleep_for (std::chrono::seconds(1 ));
    string input;
    int t = 1;

        string to_send = website + "|" + object;
        int val = send(socket_id, to_send.c_str(), to_send.length(), 0 );
        if(val  < 0)
            cout << "send eroor" << endl;
    
    
}

int main()
{
    char buffer[2048] = {0};
    int read_val;
    flag  = 1;
    string input;
    string name;
    cin >> name;
    outfile.open((name) + extension, ios::binary | ios::out);
    string header = "GET /files/course/MA_2110_Aug_4_2017_Assgn_1.pdf HTTP/1.1\r\nHost: intranet.iith.ac.in\r\n\r\n";
    string website = "intranet.iith.ac.in";

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
    thread writerth(writer, socket_id, header, website);

    readerth.join();
    writerth.join();

   
    return 0;

}
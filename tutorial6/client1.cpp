#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
using namespace std;

void getObject(string header, int socketid)
{
    char buffer[2048];
    if (send(socketid, header.c_str(), header.length(), 0)<0)
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
        fstream outfile;
        outfile.open("file.pdf", ios::binary | ios::out);
        while(act < tot_size)
        {
            curr_bytes = recv(socketid, buffer, sizeof(buffer), 0);
            string data = string(buffer);
            rec_bytes += curr_bytes;
            if(curr_bytes < 0)
            {
                cout << "recv error!!!" << endl;
                exit(0);
            }

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
                    break;
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

        }
    }
}

int main()
{
    struct addrinfo info, *address;
    int socketid;
    memset(&info, 0, sizeof(info));
    info.ai_family = AF_INET;
    info.ai_socktype = SOCK_STREAM;
    int errcode =  getaddrinfo("intranet.iith.ac.in", "80", &info, &address); //www.bjp.org
    if(errcode != 0)
    {
        cout << "Error in getaddrinfo" << endl;
        exit(0);
    }

    socketid = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    errcode = connect(socketid, address->ai_addr, address->ai_addrlen);
    if(errcode < 0)
    {
        cout << "Unable to establish connection !!" << endl;
        exit(0);
    }
    else
    {
        cout << "Connection Established!!!" << endl;
    }

    string header = "GET /files/course/MA_2110_Aug_4_2017_Assgn_1.pdf HTTP/1.1\r\nHost: intranet.iith.ac.in\r\n\r\n";
    thread downloader = thread(getObject, header, socketid);
    downloader.join();
    return 0;
    
    
    

}
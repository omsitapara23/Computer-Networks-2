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
#include <sys/stat.h>
using namespace std;

#define PAYLOAD 1046


struct DataPack
{
	int sequenceNo;
	int checkSum;
	bool packetTypeLast;
	int length;
	char payload[PAYLOAD];
	DataPack () {
		sequenceNo = 0;
		checkSum = 0;
		packetTypeLast = false;
		length = 0;
		for (int i = 0; i < PAYLOAD; ++i)
		{
			payload[i] = '\0';
		}
	}
};
struct Ack {
	bool packetType;
	int checksum;
	int ackNo;
	Ack()
	{
		packetType = false;
		checksum = 0;
		ackNo = 0;
	}
} ;

bool mysort(DataPack* packet1, DataPack* packet2)
{
	return packet1->sequenceNo < packet2->sequenceNo;
}
int computeCheckData(DataPack* packet) {
	int checkSum = 0;
	checkSum ^= packet->sequenceNo;
	checkSum ^= packet->length;
	char* payload = packet->payload;
	if (payload) {
		for (int i = 0; i < PAYLOAD; ++i)
		{
			checkSum ^= payload[i];
		}
	}
	return checkSum;
}

int computeCheckAck(Ack* packet)
{
	int checkSum = 0;
	checkSum ^= packet->packetType;
	checkSum ^= packet->ackNo;
	return checkSum;
}

bool checkSum(DataPack* packet) {
	return (packet->checkSum == computeCheckData(packet));
}


int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in recvAddr, req_addr;

	char buffer[PAYLOAD];
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		cout << "ERROR opening socket" << endl;
		return 0;
	}

	// binding port to client
	memset((char *)&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	// recvAddr.sin_addr.s_addr = INADDR_ANY;
	recvAddr.sin_port = htons(4000);

	// req_addr.sin_family = AF_INET;
	// req_addr.sin_addr.s_addr = recvAddr.sin_addr.s_addr = inet_addr("192.168.224.222");
	// req_addr.sin


	if (bind(sockfd, (struct sockaddr *)&recvAddr, sizeof(recvAddr)) < 0) {
		perror("cannot bind receiver");	
		return 0;
	}
	string s = "output.txt";

	
	ofstream outfile;
	outfile.open(s, ios::binary | ios::out);
	int expected = 0;
	
	
	int total = 0;
	int total_to_rec = 0;
	struct sockaddr senderAddr;
	vector<DataPack*> out_order_buffer;
	socklen_t serverlen =  sizeof(senderAddr);
	int total_rec = 0;
	set<int> recived_packs;
		while(true)
		{
			DataPack* packet = new DataPack;
			if (recvfrom(sockfd, packet, sizeof(*packet), 0, &senderAddr, &serverlen) < 0) {
				cout << "Error in receiving UDP segment Reason " << std::strerror(errno) << endl;
			}
			else if (!checkSum(packet)) {
				cout << "Incorrect checkSum for UDP segment with seq no: " << packet->sequenceNo << endl;
			}
			// else if (packet->sequenceNo != expected) {
			// 	// dup ACK
			// 	cout << "Error: Incorrect segment number for UDP segment Expected: Buffering---" << expected << " recevied: " << packet->sequenceNo  << endl;
			// 	Ack* ack = new Ack;
			// 	ack->ackNo = packet->sequenceNo;
			// 	ack->checksum = computeCheckAck(ack);
			// 	sendto(sockfd, ack, sizeof(*ack), 0, &senderAddr, sizeof(senderAddr));
			// 	if(recived_packs.find(packet->sequenceNo) == recived_packs.end())
			// 	{
			// 		cout << "Buffering the packet" << endl;
			// 		out_order_buffer.push_back(packet);
			// 		recived_packs.insert(packet->sequenceNo);
			// 	}
				
			// }
			else {
				total_rec++;
				Ack* ack = new Ack;
				ack->ackNo = packet->sequenceNo;
				ack->checksum = computeCheckAck(ack);
				if (packet->packetTypeLast == true)
				{
					total_to_rec = packet->sequenceNo + 1;
				}
				if(recived_packs.find(packet->sequenceNo) == recived_packs.end())
				{
					recived_packs.insert(packet->sequenceNo);
					out_order_buffer.push_back(packet);
					total++;
				}
				cout << "aCKING " << ack->ackNo << endl;
				// cout << "Accepting : seq no " << packet->header.sequenceNo << " checksum " << packet->header.checkSum << " length : " << packet->header.length << endl; 
				sendto(sockfd, ack, sizeof(*ack), 0, &senderAddr, sizeof(senderAddr));
				// sort(out_order_buffer.begin(), out_order_buffer.end(), mysort);
				// expected = out_order_buffer[out_order_buffer.size() - 1]->sequenceNo + 1;
				if (total == total_to_rec && out_order_buffer.size() == total_to_rec) {
					sort(out_order_buffer.begin(), out_order_buffer.end(), mysort);
					for(int i = 0; i < out_order_buffer.size(); i++)
					{
						int length = out_order_buffer[i]->length;
						for (int j = 0; j < length; j++)
						{
							outfile << out_order_buffer[i]->payload[j];
						}
					}
					cout << "Last packet file transfered yaay...:-)" << endl;
					break;
				}
			}
		}

		

		
	cout << "Total packets recieved : " << total_rec << endl;
	cout << "Total actual packets : " << total << endl;
	close(sockfd);
	return 0;
}
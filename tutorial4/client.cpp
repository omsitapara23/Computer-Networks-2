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

struct Header
{
	int sequenceNo;
	int ackNo;
	int checkSum;
	bool lastPacket;
	int length;
	Header () {
		sequenceNo = 0;
		ackNo = 0;
		checkSum = 0;
		lastPacket = true;
		length = 0;
	}
};

struct segment {
	Header header;
	char payload[PAYLOAD];
	segment () {
		for (int i = 0; i < PAYLOAD; ++i)
		{
			payload[i] = '\0';
		}
	}
} ;

int computeCheck(segment* packet) {
	int checkSum = 0;
	checkSum ^= packet->header.sequenceNo;
	checkSum ^= packet->header.ackNo;
	checkSum ^= packet->header.lastPacket;
	checkSum ^= packet->header.length;
	char* payload = packet->payload;
	if (payload) {
		for (int i = 0; i < PAYLOAD; ++i)
		{
			checkSum ^= payload[i];
		}
	}
	return checkSum;
}

bool checkSum(segment* packet) {
	return (packet->header.checkSum == computeCheck(packet));
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
	recvAddr.sin_addr.s_addr = inet_addr("192.168.224.222");
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
	segment* packet = new segment;
	segment* ack = new segment;
	int total = 0;
	struct sockaddr senderAddr;
	socklen_t serverlen =  sizeof(senderAddr);
	while (true) {
		while(true)
		{
			if (recvfrom(sockfd, packet, sizeof(*packet), 0, &senderAddr, &serverlen) < 0) {
				cout << "Error in receiving UDP segment Reason " << std::strerror(errno) << endl;
			}
			else if (! checkSum(packet)) {
				cout << "Incorrect checkSum for UDP segment with seq no: " << packet->header.sequenceNo << endl;
			}
			else if (packet->header.sequenceNo != expected) {
				// dup ACK
				cout << "Error: Incorrect segment number for UDP segment Expected: " << expected << " recevied: " << packet->header.sequenceNo  << endl;
				ack->header.ackNo = abs(expected);
				ack->header.checkSum = computeCheck(ack);
				sendto(sockfd, ack, sizeof(*ack), 0, &senderAddr, sizeof(senderAddr));
			}
			else {
				ack->header.ackNo = abs(expected  - 1);
				ack->header.checkSum = computeCheck(ack);
				cout << "Accepting : seq no " << packet->header.sequenceNo << " checksum " << packet->header.checkSum << " length : " << packet->header.length << endl; 
				sendto(sockfd, ack, sizeof(*ack), 0, &senderAddr, sizeof(senderAddr));
				break;
			}
		}

		expected = abs(expected - 1);
		total++;
		int length = packet->header.length;
		for (int i = 0; i < length; i++)
		{
			outfile << packet->payload[i];
		}
		if (packet->header.lastPacket) {
			cout << "Last packet file transfered yaay...:-)" << endl;
			break;
		}

		
	}
	cout << "Total actual packets : " << total << endl;
	close(sockfd);
	return 0;
}
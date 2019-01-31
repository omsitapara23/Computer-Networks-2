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


int main() {
	int sockfd;
	struct sockaddr_in senderAddr;
	struct sockaddr_in reciver_addr;

	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		cout << "ERROR opening socket" << endl;
		return 0;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 50000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		cout << "Error in creating timeout for UDP connection " << endl;
		return 0;
	}

	// binding port to socket
	senderAddr.sin_family = AF_INET;
	senderAddr.sin_addr.s_addr = inet_addr("192.168.224.222");
	senderAddr.sin_port = htons(5000);

	// binding port to client
	reciver_addr.sin_family = AF_INET;
	reciver_addr.sin_addr.s_addr = inet_addr("192.168.224.222");
	reciver_addr.sin_port = htons(4000);

	if (bind(sockfd, (struct sockaddr *)&senderAddr, sizeof(senderAddr)) < 0) {
		perror("cannot bind sender");
		return 0;
	}

	int already_sent = 0;
	fstream sendFile;
	string s;
	cout << "Enter file path :";
	cin >> s;
	sendFile.open(s, ios::binary | ios::in);

	struct stat fileToSend;
	stat(s.c_str(), &fileToSend);
	int fileSize = fileToSend.st_size;
	int last_seq = 0;
	cout << "Sending : " << s << " size : " << fileSize << endl;

	segment* packet = new segment;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	char buffer[PAYLOAD];
	int total = 0;
	while (!sendFile.eof()) {

		sendFile.read(packet->payload, PAYLOAD);
		already_sent += PAYLOAD;

		packet->header.sequenceNo = last_seq;
		packet->header.ackNo = 0;
		
		if (already_sent >= fileSize) {
			packet->header.lastPacket = true;
			packet->header.length = fileSize - already_sent + PAYLOAD;
		}
		else {
			packet->header.lastPacket = false;
			packet->header.length = PAYLOAD;
		}

		packet->header.checkSum = computeCheck(packet);
		while(true)
		{
			if (sendto(sockfd, packet, sizeof(*packet), 0, (struct sockaddr*) &reciver_addr, sizeof(reciver_addr)) < 0) {
			
				cout << "Error in sending UDP frame\tReason :: " << std::strerror(errno) << endl;
			}
			else {
				segment* ack = new segment;
				struct sockaddr senderAddr;
				socklen_t senderLen = sizeof(senderAddr);

				if (recvfrom(sockfd, ack, sizeof(*ack), 0, &senderAddr, &senderLen) < 0) {
					cout << "Error in receiving UDP ACK Reason :: " << std::strerror(errno) << endl;
					cout << "Time out... " << endl;
				}
				else if (! checkSum(ack)) {
					cout << "Incorrect checkSum for UDP ACK " << ack->header.ackNo << endl;
					cout << "Retransmission... for " << packet->header.sequenceNo << endl;
				}
				else if(ack->header.ackNo == last_seq)
				{
					cout << "Invalid ack no " << ack->header.ackNo << endl;
					cout << "Restransmission... for " << packet->header.sequenceNo << endl;
				}
				else
				{
					cout << "Packet sent succesfully with seq no : " << last_seq << endl;
					total++;
					break;
				}
				

				

				
			}
			total++;
		}
		
		last_seq = abs(last_seq - 1);
	}
	cout << "Total packets sent: " << total << endl;
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

	close(sockfd);

    return 0;
}
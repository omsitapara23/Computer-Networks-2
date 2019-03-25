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

int base = 0;
int seqnumber = 0;
int window = 10;
std::vector<DataPack*> total_file;
int total_packets = 0;
int* acks;

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

bool checkSum(Ack* packet) {
	return (packet->checksum == computeCheckAck(packet));
}

void reciver_thread(int socket_id, struct sockaddr* reciver_addr)
{
	Ack* ack;
	DataPack* packet;
	struct sockaddr senderAddr;
	socklen_t serverlen =  sizeof(senderAddr);
	while(1)
	{
		if (recvfrom(socket_id, ack, sizeof(*ack), 0, &senderAddr, &serverlen) < 0) {
			// timeout
			// int bs = base;
			// int seq = seqnumber;
			// std::cout << "Failed Transmission - base " << bs << " - nextseqnum " << seq << endl;
			// for (int i = bs; i < seq ; i++) {
			// 	packet = total_file[i];
			// 	std::cout << "-- sending " << packet->header.sequenceNo << endl;
			// 	if (sendto(socket_id, packet, sizeof(*packet), 0, reciver_addr, sizeof(*reciver_addr)) < 0) {
			// 		std::cout << "Error in sending UDP frame\tReason :: " << std::strerror(errno) << endl;
			// 	}
			// }
			// packet = total_file[base];
			// if (sendto(socket_id, packet, sizeof(*packet), 0, reciver_addr, sizeof(*reciver_addr)) < 0) {
			// 	std::cout << "Error in sending UDP frame\tReason :: " << std::strerror(errno) << endl;
			// }
		}
		else if (! checkSum(ack)) {
			std::cout << "Incorrect Checksum for UDP segment " << endl;
		}
		else {
			std::cout << "--> received " << ack->ackNo << endl;
			acks[ack->ackNo] = 1;
			if (base == ack->ackNo)
			{
				while(acks[base]==1)
				{
					base++;
				}
			}
			//std::cout << "** received " << base << " -- " << file_data[base]->header.sequenceNo  << endl;
			if (base >= (total_packets)) {
				break;
			}
		}
	}
}

void each_packet_thread(int socket_id, struct sockaddr* reciver_addr, DataPack* packet)
{
	int counter = 0;
	while(acks[packet->sequenceNo] == 0 && counter < 100)
	{
		if(acks[packet->sequenceNo] == 0)
		{
			cout << "Sending " << packet->sequenceNo  << endl;
			if(sendto(socket_id, packet, sizeof(*packet), 0, reciver_addr, sizeof(*reciver_addr)) < 0) {
				std::cout << "Error in sending UDP frame\tReason :: " << std::strerror(errno) << endl;
			}
		}
		counter++;
		this_thread::sleep_for(chrono::milliseconds(50));
	}

	if(counter >= 100)
	{
		cout << "Reciver unreachable or closed..." << endl;
		exit(1);
	}
}

void sender_thread(int socket_id, struct sockaddr* reciver_addr)
{
	while(base < total_packets)
	{
		if(seqnumber < base + window && seqnumber < total_packets)
		// if(seqnumber == 0 || seqnumber == 1)
		{
			DataPack* packet = new DataPack;
			packet = total_file[seqnumber];
			thread th = thread(each_packet_thread, socket_id, reciver_addr, packet);
			th.detach();
			seqnumber++;	
		}
	}
}


int main() {
	int sockfd;
	struct sockaddr_in senderAddr;
	struct sockaddr_in reciver_addr;

	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		std::cout << "ERROR opening socket" << endl;
		return 0;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 50000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		std::cout << "Error in creating timeout for UDP connection " << endl;
		return 0;
	}

	// binding port to socket
	senderAddr.sin_family = AF_INET;
	senderAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	senderAddr.sin_port = htons(5000);

	// binding port to client
	reciver_addr.sin_family = AF_INET;
	reciver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	reciver_addr.sin_port = htons(4000);

	if (bind(sockfd, (struct sockaddr *)&senderAddr, sizeof(senderAddr)) < 0) {
		perror("cannot bind sender");
		return 0;
	}

	int already_sent = 0;
	fstream sendFile;
	string s;
	std::cout << "Enter file path :";
	cin >> s;
	sendFile.open(s, ios::binary | ios::in);

	struct stat fileToSend;
	stat(s.c_str(), &fileToSend);
	int fileSize = fileToSend.st_size;
	int last_seq = 0;
	std::cout << "Sending : " << s << " size : " << fileSize << endl;


	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	char buffer[PAYLOAD];
	int total = 0;
	int id = 0;
	while (!sendFile.eof()) {
		DataPack* packet = new DataPack;
		packet->sequenceNo = id;
		id++;
		sendFile.read(packet->payload, PAYLOAD);
		already_sent += PAYLOAD;
		
		if (already_sent >= fileSize) {
			packet->packetTypeLast = true;
			packet->length = fileSize - already_sent + PAYLOAD;
		}
		else {
			packet->packetTypeLast = false;
			packet->length = PAYLOAD;
		}

		packet->checkSum = computeCheckData(packet);
		total_file.push_back(packet);
	}
	cout << "File loaded" << endl;
	total_packets = total_file.size();
	cout << "Total to transfer : " << total_packets << endl;
	acks = new int[total_packets];
	for(int i = 0; i < total_packets; i++)
	{
		acks[i] = 0;
	}
	struct sockaddr* to_send = (struct sockaddr*) (&reciver_addr);
	thread recc = thread(reciver_thread, sockfd, to_send);
	thread serv = thread(sender_thread, sockfd, to_send);
	recc.join();
	serv.join();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
	// 	while(true)
	// 	{
	// 		if (sendto(sockfd, packet, sizeof(*packet), 0, (struct sockaddr*) &reciver_addr, sizeof(reciver_addr)) < 0) {
			
	// 			std::cout << "Error in sending UDP frame\tReason :: " << std::strerror(errno) << endl;
	// 		}
	// 		else {
	// 			segment* ack = new segment;
	// 			struct sockaddr senderAddr;
	// 			socklen_t senderLen = sizeof(senderAddr);

	// 			if (recvfrom(sockfd, ack, sizeof(*ack), 0, &senderAddr, &senderLen) < 0) {
	// 				std::cout << "Error in receiving UDP ACK Reason :: " << std::strerror(errno) << endl;
	// 				std::cout << "Time out... " << endl;
	// 			}
	// 			else if (! checkSum(ack)) {
	// 				std::cout << "Incorrect checkSum for UDP ACK " << ack->header.ackNo << endl;
	// 				std::cout << "Retransmission... for " << packet->header.sequenceNo << endl;
	// 			}
	// 			else if(ack->header.ackNo == last_seq)
	// 			{
	// 				std::cout << "Invalid ack no " << ack->header.ackNo << endl;
	// 				std::cout << "Restransmission... for " << packet->header.sequenceNo << endl;
	// 			}
	// 			else
	// 			{
	// 				std::cout << "Packet sent succesfully with seq no : " << last_seq << endl;
	// 				total++;
	// 				break;
	// 			}
				

				

				
	// 		}
	// 		total++;
	// 	}
		
	// 	last_seq = abs(last_seq - 1);
	// }
	// std::cout << "Total packets sent: " << total << endl;
	// std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	// std::std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

	close(sockfd);

    return 0;
}
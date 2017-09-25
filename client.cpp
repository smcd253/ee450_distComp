/*****************************************************************************************
Title: Client
Author: Spencer McDonough
Date Created: 04/03/2017
Last Modified: 04/07/2017
*****************************************************************************************/
#include "config.h"

using namespace std;

int main (int argc, char* argv[]){
/*****************************************************************************************
STEP 1: Initialize TCP socket and connect to server
source: http://www.thegeekstuff.com/2011/12/c-socket-programming/?utm_source=feedburner
*****************************************************************************************/	
	int socket_fd; //  socket file descriptor
	struct sockaddr_in edge_addr; // edge server address
	
	// check client function input for errors
	if(argc != 2){
        cout << "Missing parameter\n" << endl;
        return 0;
    }
	
	// create socket
	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Create socket error: %s(errno: %d)\n", strerror(errno),errno);
        return 0;
    }
	
	// create TCP socket address 
	memset(&edge_addr, 0, sizeof(edge_addr));
    edge_addr.sin_family = AF_INET;
    edge_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    edge_addr.sin_port = htons(EDGE_PORT_TCP);
	
	// connect with socket address 
	cout << "Client connecting to edge server..." << endl;
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if(connect(socket_fd, (struct sockaddr*)&edge_addr, sizeof(edge_addr)) < 0){
        cout << "Connect error" << endl;
        return 0;
    }
	
	cout << "The client is up and running." << endl;
// STEP 2: read data from .txt file
	// open file
	ifstream input;
	input.open(argv[1]);
	
	// check for input errors
	if(!input){
		cerr << "File could not be read." << endl; 
		exit(1);
	}
	
	// grab txt --> strings by line
	char *rawdata = new char[BUFF_SIZE];
	int packet_length  = 0;
	int linenum = 0;
	int i = 0;
	
	while (!input.eof()){
		input >> rawdata[i];
		// cout strings to test
		// cout << "rawdata[" << i << "]" << " = " << rawdata[i] << endl;
		//cout << "packet_length of rawdata[" << i << "]" << " = " << packet_length << endl;
		if((strncmp(&rawdata[i],"a",1) == 0) || (strncmp(&rawdata[i],"o",1) == 0)){linenum++;}
		i++;
	}
	packet_length = strlen(rawdata);
	//cout << rawdata << endl;
/*****************************************************************************************	
STEP 3: send data to edge server
Source: http://stackoverflow.com/questions/6798967/sending-data-in-socket-programming-using-c
*****************************************************************************************/		
	// send packet
	if (send(socket_fd, rawdata, packet_length,0) < 0){
		cout << "Client --> Server Data transmission error" << endl;
	}
	else{
		printf("The client has successfully finished sending %i lines to the edge server.\n",linenum);
		printf ("Packet of size %i sent from client to edge server.\n", packet_length);
	}
	// close input file
	input.close();
	delete [] rawdata;
/*****************************************************************************************	
STEP 4: receive results from edge server via TCP
Sources: - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
*****************************************************************************************/	
	int connect_fd, respktlen; // tcp constants
	char results[BUFF_SIZE];
	
	connect_fd = 0;
	// accept socket and check for errors
	if((connect_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1){
		cout << "accept socket error detected" << endl;
	}
	//cout << "connect_fd = " << connect_fd << endl;
	
	// grab results
	// receive data into packet
	respktlen = recv(socket_fd, results, BUFF_SIZE, 0);
	//cout << *packet << endl;
	
	if (respktlen > 0){
		//cout << "pktlen = " << respktlen << endl;
		//cout << "packet = " << *packet << endl;
		printf("The client has successfully finished receiving all computation results from the edge server.\n");
		cout << "Edge server received a TCP packet of length " << respktlen << " from client at port " << EDGE_PORT_TCP << endl;
		printf("The computation results are:\n");
/*****************************************************************************************	
STEP 5: process and display results
Sources: 
*****************************************************************************************/	
		int r = 0;
		printf("results = %s\n",results);
		while (strncmp(&results[r],"o",1) != 0){
			printf("AND Result: ");
			while (strncmp(&results[r],";",1) != 0){
				printf("%c",results[r]);
				r++;
			}
			printf("\n");
			r++;
		}
		r++;
		while(strncmp(&results[r],"\0",1) != 0){
			printf("OR Result: ");
			while (strncmp(&results[r],";",1) != 0){
				printf("%c",results[r]);
				if(strncmp(&results[r],"\0",1) == 0){break;}
				r++;
			}
			printf("\n");
			if(strncmp(&results[r],"\0",1) == 0){break;}
			r++;
		}
	}
	//clear results
	for (int i = 0; i < BUFF_SIZE; i++){results[i] = '\0';}
	// close connection with client
	if (close(socket_fd) < 0){
		cout << "Error closing connection with edge server" << endl;
	}	

return 0;
}
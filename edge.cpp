/*****************************************************************************************
Title: Edge Server
Author: Spencer McDonough
Date Created: 04/03/2017
Last Modified: 04/07/2017
*****************************************************************************************/
#include "config.h"

using namespace std;
int main (int argc, char* argv[]){
	// initialize constants
	int sockfd_tcp, connect_fd, pktlen; // tcp constants
	struct sockaddr_in edge_addr;
/*****************************************************************************************
STEP 1a: Initialize TCP socket and listen for client
Source: http://stackoverflow.com/questions/6798967/sending-data-in-socket-programming-using-c
*****************************************************************************************/
 	// create socket
 	sockfd_tcp =  socket(AF_INET, SOCK_STREAM, 0);
 	if(sockfd_tcp < 0){
        cout << "socket creation error detected" << endl;
        return 0;
    }
    
    memset(&edge_addr, 0, sizeof(edge_addr));
    edge_addr.sin_family = AF_INET;
    edge_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    edge_addr.sin_port = htons(EDGE_PORT_TCP);

	
 	// bind socket 
 	if (bind(sockfd_tcp, (struct sockaddr *) &edge_addr, sizeof(edge_addr)) < 0){
 		cout << "binding error detected" << endl;
 	}
 	
	// listen to socket 
	if (listen(sockfd_tcp, 10) < 0){
		cout << "socket listen error detected" << endl;
	}
	
	printf("The edge server is up and running.");
	
	// data buffers
	char buff_and[BUFF_SIZE];
	char buff_or[BUFF_SIZE];
	char *packet = new char[BUFF_SIZE]; // dynamically allocate a packet of type char
	
	// while loop indices
	int and_i, or_i, n, m;
	
/*****************************************************************************************
STEP 1b: Initialize UDP sockets
Source: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
*****************************************************************************************/	
	struct sockaddr_in andserver_addr, orserver_addr;

	// boot message
	cout << "UDP network booting up." << endl;
	
    // create UDP socket
    int sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);

    // initialize backend server addresses
    memset(&andserver_addr, 0, sizeof(andserver_addr));
    andserver_addr.sin_family = AF_INET; // force IPv4
    andserver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    andserver_addr.sin_port = htons(AND_PORT_UDP);

    memset(&orserver_addr, 0, sizeof(orserver_addr));
    orserver_addr.sin_family = AF_INET; // force IPv4
    orserver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    orserver_addr.sin_port = htons(OR_PORT_UDP);
    
    cout << "The UDP network is running." << endl;
    
	// udp receipt constants
	socklen_t length_and_addr = sizeof(andserver_addr); // must be of type socklen_t, NOT int
	socklen_t length_or_addr = sizeof(orserver_addr);
	int andpktlen, orpktlen, sendreslen, andlinenum, orlinenum, totlinenum;
	andlinenum = 0;
	orlinenum = 0;
	totlinenum = 0;
	char *andresults = new char[BUFF_SIZE];
	char *orresults = new char[BUFF_SIZE];
	char *sendresults = new char[BUFF_SIZE];
	
	while (1){
	connect_fd = 0;
	
/*****************************************************************************************	
STEP 2: receive TCP raw data from client
Sources: - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
*****************************************************************************************/
		// accept socket and check for errors
		if((connect_fd = accept(sockfd_tcp, (struct sockaddr*)NULL, NULL)) == -1){
				cout << "accept socket error detected" << endl; 
				continue;
			}
	  	//cout << "connect_fd = " << connect_fd << endl;
		// receive data into packet
		pktlen = recv(connect_fd, packet, BUFF_SIZE, 0);
		//cout << *packet << endl;
		/*if (pktlen > 0){
			cout << "pktlen = " << pktlen << endl;
			//cout << "packet = " << *packet << endl;
			for (int i = 0; i < pktlen; i++){
				cout << "packet[" << i << "] = " << packet[i] << endl;
				//cout << "packet_buff[" << i << "] = " << packet_buff[i] << endl;
				//cout << "packet[" << i << "] = " << packet[i] << endl;
				//cout << "buff[" << i << "] = " << buff[i] << endl;
			}
			//cout << "packet = " << packet_buff << endl;
		}*/
        
/*****************************************************************************************	
STEP 3: build backend server buffers 
source: 
*****************************************************************************************/		
		for (int i = 0; i < pktlen; i++){
			//cout << "we have entered the next for loop" << endl;
			//cout << "packet[" << i << "] = " << packet[i] << endl;
			
			if(strncmp(&packet[i],"a",1) == 0){
				andlinenum++;
				totlinenum++;
				n = i + 4; // jump to first binary diget 
				while ((strncmp(&packet[n],"a",1) != 0) && (strncmp(&packet[n],"o",1) != 0)){
					if(strncmp(&packet[n],"\0",1) == 0){break;}
					buff_and[and_i] = packet[n];
					//printf("buff_and[%i] = %c\n", and_i, buff_and[and_i]);
					and_i++;
					n++;
				}
				buff_and[and_i] = ';';
				//printf("buff_and[%i] = %c\n", and_i, buff_and[and_i]);
				and_i++;
				i = n-1;
			}
			if (strncmp(&packet[i],"o",1) == 0){
				orlinenum++;
				totlinenum++;
				m = i + 3; // jump to first binary diget
				while ((strncmp(&packet[m],"a",1) != 0) && (strncmp(&packet[m],"o",1) != 0)){
					if(strncmp(&packet[m],"\0",1) == 0){break;}
					buff_or[or_i] = packet[m];
					//printf("buff_or[%i] = %c\n", or_i, buff_or[or_i]);
					or_i++;
					m++;
				}
				buff_or[or_i] = ';';
				//printf("buff_or[%i] = %c\n", or_i, buff_or[or_i]);
				or_i++;
				i = m-1;
			}
			//cout << "buff_and[" << i << "] = " << buff[i] << endl;
		}
		or_i = 0;
		and_i = 0;
		printf("The edge server has received %i lines from the client using TCP over port %i.\n", totlinenum, EDGE_PORT_TCP);
/*****************************************************************************************	
STEP 4a: send UPD packet to server_and 
Sources: - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
		 - http://www.cplusplus.com/forum/unices/116919/
*****************************************************************************************/		
		if(sendto(sockfd_udp, buff_and, strlen(buff_and),0,(struct sockaddr*)&andserver_addr,sizeof(andserver_addr)) == -1){
            printf("Error: could not send UDP packet 'buff_and' to server_and.\n");
            return 1;
        }
        printf("The edge server has successfully sent %i lines to Backend-Server AND.\n", andlinenum);
/*****************************************************************************************	
STEP 4b: send UPD packet to server_or 
Sources: - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
		 - http://www.cplusplus.com/forum/unices/116919/
*****************************************************************************************/		
		if(sendto(sockfd_udp, buff_or, strlen(buff_or),0,(struct sockaddr*)&orserver_addr,sizeof(orserver_addr)) == -1){
            printf("Error: could not send UDP packet 'buff_and' to server_or.\n");
            return 1;
        }
        printf("The edge server has successfully sent %i lines to Backend-Server OR.\n", orlinenum);
/*****************************************************************************************	
STEP 5a: receive results from server_and via UPD
Sources: - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
		 - http://www.cplusplus.com/forum/unices/116919/
*****************************************************************************************/
		// read incoming packet
		andpktlen = recvfrom(sockfd_udp, andresults, BUFF_SIZE,0,(struct sockaddr*)&andserver_addr, &length_and_addr);
        if(andpktlen == -1){
            printf("Error: server_and could not receive edge server's UDP packet.\n");
            return 1;
        }
        //printf("andresults received from server_and.\n");
        /*for (int i = 0; i < andpktlen; i++){
        	printf("andresults[%i] = %c\n", i, andresults[i]);
        }*/
        printf("The edge server received computation results from Backend-Server AND using UDP port %i.\n", AND_PORT_UDP);
/*****************************************************************************************	
STEP 5b: receive results from server_or via UPD
Sources: - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
		 - http://www.cplusplus.com/forum/unices/116919/
*****************************************************************************************/
		// read incoming packet
		orpktlen = recvfrom(sockfd_udp, orresults, BUFF_SIZE,0,(struct sockaddr*)&orserver_addr, &length_or_addr);
        if(andpktlen == -1){
            printf("Error: server_and could not receive edge server's UDP packet.\n");
            return 1;
        }
        /*printf("orresults received from server_or.\n");
        for (int i = 0; i < orpktlen; i++){
        	printf("orresults[%i] = %c\n", i, orresults[i]);
        }*/
        printf("The edge server received computation results from Backend-Server OR using UDP port %i\n", OR_PORT_UDP);
/*****************************************************************************************	
STEP 6: print out results, build results packet to send back to client
Sources: 
*****************************************************************************************/ 
		printf("The computation results are:\n");       
		int res_i = 0;
		int ores_i = 0;
		int abuff_i = 0;
		int adisp_i = 0;
		int obuff_i = 0;
		int odisp_i = 0;

		//printf("buff_and length = %lu\n", strlen(buff_and));
		//printf("andpktlen = %i\n", andpktlen);
		//printf("orpktlen = %i\n",orpktlen);
		
		//build result packet (to send to client) and display results on edge server terminal
    	while (res_i < andpktlen){
			if(strncmp(&andresults[adisp_i],"\0",1) != 0){
				// print first bit string
				while(strncmp(&buff_and[abuff_i],",",1) != 0){
					printf("%c", buff_and[abuff_i]);
					abuff_i++;
				}
				abuff_i++;
				printf(" AND ");
				// print second bit string
				while(strncmp(&buff_and[abuff_i],";",1) != 0){
					printf("%c", buff_and[abuff_i]);
					abuff_i++;
				}
				abuff_i++;
				printf(" = ");
				// print result string
				while(strncmp(&andresults[adisp_i],";",1) != 0){
					printf("%c", andresults[adisp_i]);
					if(strncmp(&andresults[adisp_i],"\0",1) == 0){break;}
					adisp_i++;
				}
				adisp_i++;
				printf("\n");
			}
    		
    		//build results array
    		sendresults[res_i] = andresults[res_i];
    		//printf("sendresults[%i] = %c\n", res_i, sendresults[res_i]);
    		res_i++;
    		//printf("res_i = %i\nores_i = %i\nabuff_i = %i\nadisp_i = %i\nobuff_i = %i\nodisp_i = %i\n",res_i,ores_i,abuff_i,adisp_i,obuff_i,odisp_i);
    	}
    	sendresults[res_i] = 'o';
    	res_i++;
    	//printf("sendresults[%i] = %c\n", res_i, sendresults[res_i]);
    	while (ores_i < orpktlen){
    		if(strncmp(&orresults[odisp_i],"\0",1) != 0){
				// print first bit string
				while(strncmp(&buff_or[obuff_i],",",1) != 0){
					printf("%c", buff_or[obuff_i]);
					obuff_i++;
				}
				obuff_i++;
				printf(" OR ");
				// print second bit string
				while(strncmp(&buff_or[obuff_i],";",1) != 0){
					printf("%c", buff_or[obuff_i]);
					obuff_i++;
				}
				obuff_i++;
				printf(" = ");
				// print result string
				while(strncmp(&orresults[odisp_i],";",1) != 0){
					printf("%c", orresults[odisp_i]);
					if(strncmp(&orresults[odisp_i],"\0",1) == 0){break;}
					odisp_i++;
				}
				odisp_i++;
				printf("\n");
			}
    		//build results array
    		sendresults[res_i] = orresults[ores_i];
    		//printf("sendresults[%i] = %c\n", res_i, sendresults[res_i]);
    		res_i++;
    		ores_i++;
    		
    		//printf("res_i = %i\nores_i = %i\nabuff_i = %i\nadisp_i = %i\nobuff_i = %i\nodisp_i = %i\n",res_i,ores_i,abuff_i,adisp_i,obuff_i,odisp_i);
    	}
    	printf("The edge server has successfully finished receiving all computation results from the Backend-Server AND and Backend-Server OR.\n");
/*****************************************************************************************	
STEP 7: send and results back to client via TCP
Sources: - http://stackoverflow.com/questions/6798967/sending-data-in-socket-programming-using-c  
*****************************************************************************************/ 
		sendreslen = strlen(sendresults);
		//re printf("sendresults = %s", sendresults);
		// send packet
		if (send(connect_fd, sendresults, sendreslen,0) < 0){
			cout << "Client --> Server Data transmission error" << endl;
		}
		else{
			printf("The edge server has successfully finished sending all computation results to the client.\n");
			printf ("Packet of size %i sent from edge server to client.\n", sendreslen);
		}
	} // end infinite while
	// close connection with client
	if (close(connect_fd) < 0){
		cout << "Error closing connection with client" << endl;
	}
	close(sockfd_udp);
	delete [] sendresults;
	delete [] andresults;
	delete [] orresults;
// STEP 5: send TCP results back to client
	return 0;
}
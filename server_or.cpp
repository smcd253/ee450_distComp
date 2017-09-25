/*****************************************************************************************
Title: Server OR
Author: Spencer McDonough
Date Created: 04/03/2017
Last Modified: 04/07/2017
*****************************************************************************************/
#include "config.h"

using namespace std;

int main(){
/*****************************************************************************************
STEP 1: Initialize UDP sockets
Sources: - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
		 - http://www.cplusplus.com/forum/unices/116919/
*****************************************************************************************/

	// define udp address
    struct sockaddr_in edge_addr;
    memset(&edge_addr, 0, sizeof(edge_addr));
    edge_addr.sin_family = AF_INET; // force IPv4
    edge_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    edge_addr.sin_port = htons(OR_PORT_UDP);

    // create udp socket 
    int sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd_udp == -1){
        printf("Socket Creation Error");
        return 1;
    }

    // bind socket 
    if((bind(sockfd_udp,(struct sockaddr*)&edge_addr,sizeof(edge_addr))) == -1){
        printf("Socket Bind Failure");
        return 1;
    }
    printf("The Server OR is up and running using UDP on port %i.", OR_PORT_UDP);
    
        // necessary constants
    socklen_t length_edge_addr = sizeof(edge_addr); // must be of type socklen_t, NOT int
    char *orpacket = new char[BUFF_SIZE];
    char *bstring1 = new char[BUFF_SIZE];
    char *bstring2 = new char[BUFF_SIZE];
    char *bstring1pad = new char[BUFF_SIZE];
    char *bstring2pad = new char[BUFF_SIZE];
    char *orresults = new char[BUFF_SIZE];
    
    int pktlen, bin_i, res_i, length1, length2, prevreslen, diff, linecount, j;
    j = 0;
    bin_i = 0;
    res_i = 0;
    linecount = 0;
    prevreslen = 0;
    // begin processing
    while (1){ 
/*****************************************************************************************
STEP 2: receive incoming UDP packet
Sources: - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
 		 - http://www.cplusplus.com/forum/unices/116919/
*****************************************************************************************/
		// read incoming packet
		pktlen = recvfrom(sockfd_udp, orpacket, BUFF_SIZE,0,(struct sockaddr*)&edge_addr, &length_edge_addr);
        if(pktlen == -1){
            printf("Error: server_or could not receive edge server's UDP packet");
            return 1;
        }
        printf("The Server OR start receiving lines from the edge server for OR computation.\n");
        printf("The computation results are:\n");
/*****************************************************************************************
STEP 3: perform bitwise OR, build resulting packet
Sources: self...
*****************************************************************************************/
		/*// loop to check input packet
		for(int i = 0; i < pktlen; i++){
			printf("orpacket[%i] = %c\n", i, orpacket[i]); // test packet
		}*/
		
		res_i = 0;// reset result index
		for(int i = 0; i < pktlen; i++){ 
			//printf("orpacket[%i] = %c\n", i, orpacket[i]); // test packet
			//build bstring1
			//printf("strncmp[%i] = %i\n",i, strncmp(&orpacket[i],",",1));
			while(strncmp(&orpacket[i],",",1) != 0){
				bstring1[bin_i] = orpacket[i];
				//printf("bstring1[%i] = %c\n", bin_i, bstring1[bin_i]);
				bin_i++;
				i++;
			}
			bin_i = 0;
			//once we reach a comma, we iterate to the next index and start building bstring2
			i++; // force iteration to prevent running through next loop
			//build bstring2
			while(strncmp(&orpacket[i],";",1) != 0){
				bstring2[bin_i] = orpacket[i];
				//printf("bstring2[%i] = %c\n", bin_i, bstring2[bin_i]);
				bin_i++;
				i++;
			}
			bin_i = 0;
			//once we reach a semicolon, we perform the OR operation and add the result to orresults
			//find length of longest bstring and pad the other with 0's
			length1 = strlen(bstring1);
			length2 = strlen(bstring2);
			//printf("length1 = %i\nlength2 = %i\n",length1,length2);
			if(length1 > length2){ // bstring1 is bigger
				diff = length1 - length2;
				for(int n = 0; n < length1; n++){
					if(n < diff){
						bstring2pad[n] = '0'; // pad bstring2
					}
					else{ 
						bstring2pad[n] = bstring2[n-diff]; // fill in rest with data
					}
					//printf("bstring2pad[%i] = %c\n", n, bstring2pad[n]);
				}
				for(int n = 0; n < length1; n++){
					//printf("bstring1[%i] = %c\n", n, bstring1[n]);
				}
				// perform OR operation --> store into orresults
				for (int n = 0; n < length1; n++){
					if ((strncmp(&bstring1[n],"1",1) == 0) || (strncmp(&bstring2pad[n],"1",1) == 0)){
						orresults[res_i] = '1';
					}
					else{orresults[res_i] = '0';}
					//printf("orresults[%i] = %c\n", res_i, orresults[res_i]);
					res_i++;
				}
				orresults[res_i] = ';';
				//printf("operation done - string1 longer\n");
				//printf("orresults[%i] = %c\n", res_i, orresults[res_i]);
				res_i++;
			}
			else if(length1 < length2){ // bstring2 is bigger
				diff = length2 - length1;
				for(int n = 0; n < length2; n++){
					if(n < diff){
						bstring1pad[n] = '0'; // pad bstring2
					}
					else{ 
						bstring1pad[n] = bstring1[n-diff]; // fill in rest with data
					}
					//printf("bstring1pad[%i] = %c\n", n, bstring1pad[n]);
				}
				// test bstring2
				for(int n = 0; n < length2; n++){
					//printf("bstring2[%i] = %c\n", n, bstring2[n]);
				}
				// perform OR operation --> store into orresults
				for (int n = 0; n < length2; n++){
					if ((strncmp(&bstring1pad[n],"1",1) == 0) || (strncmp(&bstring2[n],"1",1) == 0)){
						orresults[res_i] = '1';
					}
					else{orresults[res_i] = '0';}
					//printf("orresults[%i] = %c\n", res_i, orresults[res_i]);
					res_i++;
				}
				orresults[res_i] = ';';
				//printf("operation done - string2 longer\n");
				//printf("orresults[%i] = %c\n", res_i, orresults[res_i]);
				res_i++;
			}
			else{
				// perform OR operation --> store into orresults
				for (int n = 0; n < length2; n++){
					if ((strncmp(&bstring1[n],"1",1) == 0) || (strncmp(&bstring2[n],"1",1) == 0)){
						orresults[res_i] = '1';
					}
					else{orresults[res_i] = '0';}
					//printf("orresults[%i] = %c\n", res_i, orresults[res_i]);
					res_i++;
				}
				orresults[res_i] = ';';
				//printf("operation done - same size\n");
				//printf("orresults[%i] = %c\n", res_i, orresults[res_i]);
				res_i++;
			}
			
			// print results
			for (int i = 0; i < length1; i++){
				printf("%c",bstring1[i]);
			}
			printf(" OR ");
			for (int i = 0; i < length2; i++){
				printf("%c",bstring2[i]);
			}
			printf(" = ");
			
			//SEGMENTATION FAULT GENERATED HERE AFTER 2ND EXECUTION OF ./client
			//maybe memory leakage
			if (j < strlen(orresults)){
				while(strncmp(&orresults[j],";",1) != 0){
					printf("%c", orresults[j]);
					j++;
				}
				j++;
			}
			/*for (int i = 0; i < (strlen(orresults) - 1 - prevreslen); i++){
				printf("%c", orresults[i]);
			}*/
			//prevreslen = strlen(orresults);
			printf("\n");
			linecount++;
			// clear bstring1 and 2
			for (int n = 0; n < max(length1,length2); n++){
				bstring1[n] = '\0';
				bstring2[n] = '\0';
			}
		} // end for
		printf("orresults = %s\n",orresults);
		printf("The Server OR has successfully received %i lines from the edge server and finished all OR computations.", linecount);
/*****************************************************************************************	
STEP 4: send results back to edge server via UPD packet 
Sources: - http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
		 - http://www.cplusplus.com/forum/unices/116919/
*****************************************************************************************/		
		if(sendto(sockfd_udp, orresults, strlen(orresults),0,(struct sockaddr*)&edge_addr,sizeof(edge_addr)) == -1){
            printf("Error: could not send UDP packet 'buff_or' to server_or.\n");
            return 1;
        }
        printf("The Server OR has successfully finished sending all computation results to the edge server.");
        //clear results
        for (int n = 0; n < max(pktlen,int(strlen(orresults))); n++){
				orpacket[n] = '\0';
				orresults[n] = '\0';
		}
	} // end infinite while
	close(sockfd_udp);
	delete [] bstring1;
	delete [] bstring2;
	delete [] bstring1pad;
	delete [] bstring2pad;
	delete [] orpacket;
    delete [] orresults;
	 
}
    
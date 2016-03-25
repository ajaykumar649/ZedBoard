// ots_udp_hw_emulator.cpp
//    by rrivera at fnal dot gov
//	  created Feb 2016
//
// This is a simple emulator of a "data gen" front-end (hardware) interface
// using the otsdaq UDP protocol.
//
//compile with:
//g++ ots_udp_hw_emulator.cpp -o hw.o
//
//if developing, consider appending -D_GLIBCXX_DEBUG to get more 
//descriptive error messages
//
//run with:
//./hw.o
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


using namespace std;


#define MYPORT "4987"    // the port users will be connecting to
#define MAXBUFLEN 1492

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int makeSocket(const char * ip, int port, struct addrinfo*& p)
{
	int sockfd;
	struct addrinfo hints, *servinfo;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	char portStr[10];
	sprintf(portStr,"%d",port);
	if ((rv = getaddrinfo(ip, portStr, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("sw: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "sw: failed to create socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	return sockfd;
}

int main(void)
{
    int sockfd;
    int sendSockfd=0;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buff[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    //////////////////////////////////////////////////////////////////////
    ////////////// ready to go //////////////
    //////////////////////////////////////////////////////////////////////

    //hardware "registers"
	uint64_t 	data_gen_cnt = 0;
	uint64_t 	data_gen_rate = 1<<16;
	uint8_t		data_enable = 0;

    const unsigned int RX_ADDR_OFFSET = 2;
    const unsigned int RX_DATA_OFFSET = 10;
    const unsigned int TX_DATA_OFFSET = 2;

    bool wasDataEnable = false;
    unsigned char sequence = 0;
    unsigned int packetSz;

    //for timeout/select
    struct timeval tv;
	fd_set readfds, masterfds;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	FD_ZERO(&masterfds);
	FD_SET(sockfd, &masterfds);

	time_t count = 0;

    while(1)
    {
    	readfds = masterfds; //copy to reset timeout select
		select(sockfd+1, &readfds, NULL, NULL, &tv);

	    if (FD_ISSET(sockfd, &readfds))
	    {
	    	//packet received
	    	cout << "hw: Line " << __LINE__ << ":::" << "Packet Received!" << endl;

			addr_len = sizeof their_addr;
			if ((numbytes = recvfrom(sockfd, buff, MAXBUFLEN-1 , 0,
				(struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}

			printf("hw: got packet from %s\n",
				inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *)&their_addr),
					s, sizeof s));
			printf("hw: packet is %d bytes long\n", numbytes);
			printf("packet contents: ");

			for(int i=0;i<numbytes;++i)
			{
				if((i-RX_ADDR_OFFSET)%8==0) printf("\n");
				printf("%2.2X", (unsigned char)buff[i]);
			}
			printf("\n");


			//handle packet
			if(numbytes == 10 && 			//size is valid (type, size, 8-byte address)
					buff[0] == 0) //read
			{
				uint64_t addr;
	    		memcpy((void *)&addr,(void *)&buff[RX_ADDR_OFFSET],8);

		    	cout << "hw: Line " << __LINE__ << ":::" << "Read address: 0x" << hex << addr;
				printf(" 0x%16.16lX", addr);
				cout << endl;

			 	//setup response packet based on address
				buff[0] = 0; //read type
				buff[1] = sequence++; //1-byte sequence id increments and wraps

				switch(addr) //define address space
				{
				case 0x1001:
		    		memcpy((void *)&buff[TX_DATA_OFFSET],(void *)&data_gen_cnt,8);
			    	cout << "hw: Line " << __LINE__ << ":::" << "Read data count: " <<  data_gen_cnt << endl;
					break;
				case 0x1002:
		    		memcpy((void *)&buff[TX_DATA_OFFSET],(void *)&data_gen_rate,8);
			    	cout << "hw: Line " << __LINE__ << ":::" << "Read data rate: " <<  data_gen_rate << endl;
					break;
				case 0x0000000100000009:
		    		memset((void *)&buff[TX_DATA_OFFSET+1],0,7);
		    		memcpy((void *)&buff[TX_DATA_OFFSET],(void *)&data_enable,1);
			    	cout << "hw: Line " << __LINE__ << ":::" << "Read data enable: " <<  data_enable << endl;
					break;
				default:
			    	cout << "hw: Line " << __LINE__ << ":::" << "Unknown read address received." << endl;
				}

				packetSz = TX_DATA_OFFSET + 8; //only response with 1 QW
				if ((numbytes = sendto(sockfd, buff, packetSz, 0,
						(struct sockaddr *)&their_addr, sizeof(struct sockaddr_storage))) == -1) {
					perror("hw: sendto");
					exit(1);
				}
				printf("hw: sent %d bytes back\n", numbytes);


				++sequence; //increment sequence counter
			}
			else if(numbytes >= 18 && (numbytes-2)%8 == 0 && //size is valid (multiple of 8 data)
					buff[0] == 1) //write
			{
				uint64_t addr;
				memcpy((void *)&addr,(void *)&buff[RX_ADDR_OFFSET],8);
		    	cout << "hw: Line " << __LINE__ << ":::" << "Write address: 0x" << hex << addr;
				printf(" 0x%16.16lX", addr);
				cout << endl;

				switch(addr) //define address space
				{
				case 0x1001:
					memcpy((void *)&data_gen_cnt,(void *)&buff[RX_DATA_OFFSET],8);
			    	cout << "hw: Line " << __LINE__ << ":::" << "Write data count: " << data_gen_cnt << endl;
			    	count = 0; //reset count
					break;
				case 0x1002:
					memcpy((void *)&data_gen_rate,(void *)&buff[RX_DATA_OFFSET],8);
			    	cout << "hw: Line " << __LINE__ << ":::" << "Write data rate: " << data_gen_rate << endl;
					break;
				//case 0x0000000100000006:
				//case 0x0000000100000007:
				case 0x0000000100000008:
					{
						int myport;
						memcpy((void *)&myport,(void *)&buff[RX_DATA_OFFSET],2);

					    close(sendSockfd); sendSockfd = 0;
						sendSockfd = makeSocket(string("localhost").c_str(),myport,p);
				    	cout << "hw: Line " << __LINE__ << ":::" << "New dest socket at port: " <<
				    			myport << dec << " " << myport << endl;
					}

					break;
				case 0x0000000100000009:
		    		memcpy((void *)&data_enable,(void *)&buff[RX_DATA_OFFSET],1);
			    	cout << "hw: Line " << __LINE__ << ":::" << "Write data enable: " <<  (int)data_enable << endl;
					break;
				default:
					cout << "hw: Line " << __LINE__ << ":::" << "Unknown write address received." << endl;
				}
			}


	    }
	    else
	    {
	    	//no packet received (timeout)

	    	if(data_enable)
	    	{
	    		//generate data
	    		if(count%data_gen_rate == 0 && //if delayed enough for proper rate
	    				data_gen_cnt != 0)	//still packets to send
	    		{
		    		cout << "hw: Line " << __LINE__ << ":::" << "Send Burst at count:" <<
		    				count << endl;
	    			//send a packet
	    			buff[0] = wasDataEnable?2:1; //type := burst middle (2) or first (1)
	    			buff[1] = sequence++; //1-byte sequence id increments and wraps
	    			memcpy((void *)&buff[TX_DATA_OFFSET],(void *)&count,8); //make data counter

	    			packetSz = TX_DATA_OFFSET + 8; //only response with 1 QW

					if ((numbytes = sendto(sockfd, buff, packetSz, 0,
							 p->ai_addr, p->ai_addrlen)) == -1) {
						perror("hw: sendto");
						exit(1);
					}
					printf("hw: sent %d bytes back\n", numbytes);

					if(data_gen_cnt != (uint64_t)-1)
						--data_gen_cnt;
	    		}

	    		wasDataEnable = true;
	    	}
	    	else if(wasDataEnable) //send last in burst packet
	    	{
	    		wasDataEnable = false;
	    		cout << "hw: Line " << __LINE__ << ":::" << "Send Last in Burst at count:" <<
	    				count << endl;
				//send a packet
				buff[0] = 3; //type := burst last (3)
				buff[1] = sequence++; //1-byte sequence id increments and wraps
				memcpy((void *)&buff[TX_DATA_OFFSET],(void *)&count,8); //make data counter

				packetSz = TX_DATA_OFFSET + 8; //only response with 1 QW

				if(sendSockfd)
				{
					if ((numbytes = sendto(sendSockfd, buff, packetSz, 0,
							 p->ai_addr, p->ai_addrlen)) == -1) {
						perror("hw: sendto");
						exit(1);
					}
					printf("hw: sent %d bytes back\n", numbytes);
				}
				else
					cout << "hw: Line " << __LINE__ << ":::" << "Send socket not defined." << endl;
			}

	    	++count;
	    }
    }

    close(sockfd);

    return 0;
}


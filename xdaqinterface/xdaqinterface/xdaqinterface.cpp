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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <ctype.h>
#include "hgc_util.h"


using namespace std;


//#define ZED_IP   "192.168.133.6"    // the ZED IP users will be connecting to
//#define ZED_IP             "192.168.133.5"    // the ZED IP users will be connecting to
#define COMMUNICATION_PORT "2035"             // the port users will be connecting to
#define STREAMING_PORT     "2036"             // the port users will be connecting to
#define DESTINATION_IP     "192.168.133.5"    // the port users will be connecting to
#define DESTINATION_PORT   2039            // the port users will be connecting to
#define MAXBUFLEN 1492

//========================================================================================================================
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//========================================================================================================================
int makeSocket(const char* ip, const char* port, struct addrinfo*& addressInfo)
{
    int socketId = 0;
	struct addrinfo hints, *servinfo, *p;
    int sendSockfd=0;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buff[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    if(ip == NULL)
    	hints.ai_flags    = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((socketId = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(socketId, p->ai_addr, p->ai_addrlen) == -1) {
            close(socketId);
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
    return socketId;

}

//========================================================================================================================
struct sockaddr_in setupSocketAddress(const char* ip, unsigned int port)
{
	//std::cout << __PRETTY_FUNCTION__ << std::endl;
	//network stuff
	struct sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;// use IPv4 host byte order
    socketAddress.sin_port   = htons(port);// short, network byte order

    if(inet_aton(ip, &socketAddress.sin_addr) == 0)
    {
        std::cout << "FATAL: Invalid IP address " <<  ip << std::endl;
        exit(0);
    }

    memset(&(socketAddress.sin_zero), '\0', 8);// zero the rest of the struct
    return socketAddress;
}

//========================================================================================================================
int send(int fromSocket, struct sockaddr_in& toSocketAddress, const std::string& buffer)
{
    std::cout << "Socket Descriptor #: " << fromSocket
    		<< " ip: " << std::hex << toSocketAddress.sin_addr.s_addr << std::dec
    		<< " port: " << toSocketAddress.sin_port
			<< std::endl;
    if(sendto(fromSocket, buffer.c_str(), buffer.size(), 0, (struct sockaddr *)&(toSocketAddress), sizeof(sockaddr_in)) < (int)(buffer.size()))
    {
        std::cout << "Error writing buffer" << std::endl;
        return -1;
    }
    return 0;
}

//========================================================================================================================
int receive(int socketNumber, std::string& buffer, struct sockaddr_in& fromAddress)
{
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0; //set timeout period for select()
	fd_set fileDescriptor;  //setup set for select()
	FD_ZERO(&fileDescriptor);
	FD_SET(socketNumber,&fileDescriptor);
	select(socketNumber+1, &fileDescriptor, 0, 0, &tv);

	if(FD_ISSET(socketNumber,&fileDescriptor))
	{
		std::string bufferS;
		//struct sockaddr_in fromAddress;
		socklen_t addressLength = sizeof(fromAddress);
		int nOfBytes;
		buffer.resize(MAXBUFLEN);
		if ((nOfBytes=recvfrom(socketNumber, &buffer[0], MAXBUFLEN, 0, (struct sockaddr *)&fromAddress, &addressLength)) == -1)
			return -1;

		buffer.resize(nOfBytes);
		//send(socketNumber, fromAddress, buffer);
//		char address[INET_ADDRSTRLEN];
//		inet_ntop(AF_INET, &(fromAddress.sin_addr), address, INET_ADDRSTRLEN);
//		unsigned long  fromIPAddress = fromAddress.sin_addr.s_addr;
//		unsigned short fromPort      = fromAddress.sin_port;

	}
	else
		return -1;

	return 0;
}

//========================================================================================================================
int main(void)
{

    /////////////////////
    // Bind UDP socket //
    /////////////////////
      
    //sendSockfd = makeSocket(string("localhost").c_str(),myport,p);

    struct addrinfo hints, *servinfo, *p;

    int communicationSocket = makeSocket(0,COMMUNICATION_PORT,p);
    struct sockaddr_in messageSender;
    int streamingSocket     = makeSocket(0,STREAMING_PORT,p);
    struct sockaddr_in streamingReceiver = setupSocketAddress(DESTINATION_IP, DESTINATION_PORT);

    std::string communicationBuffer;

    ///////////////// PAUL'S CODE //////////////////////////////////////////////
    //////////////////////////////
    // Put aside page of memory //
    //////////////////////////////

    int memfd;
    char str[255];
    volatile void * zed_ch0;
    unsigned int Data32Write;
    unsigned int Data32Read;
    //---------------------------------------------------------------------
    // Open /dev/mem file
    //---------------------------------------------------------------------
    memfd = open("/dev/mem", O_RDWR | O_SYNC);
    //memfd = open("/tmp/mmapped.bin", O_RDWR | O_SYNC);
    if (memfd == -1)
    {
        printf("Can't open /dev/mem\r\n");
        //printf("Can't open /tmp/mmapped.bin\r\n");
        exit(0);
    }

    //result = lseek(fd, FILESIZE-1, SEEK_SET);
    //if (result == -1) {
    //    close(fd);
    //    perror("Error calling lseek() to 'stretch' the file");
    //    exit(EXIT_FAILURE);
    //}
    
    printf("/dev/mem opened\r\n");
    //printf("/tmp/mmapped.bin opened\r\n");
    printf("===============\r\n");
    printf("Test loop\r\n");
    printf("===============\r\n");

    //---------------------------------------------------------------------
    // Map the device into memory.
    // Map one page of memory into user space such that the device is
    // in that page, but it may not be at the start of the page.
    //---------------------------------------------------------------------
    unsigned long page_size = sysconf(_SC_PAGESIZE);
    printf("page_size=0x%.8lX\r\n", page_size);

    zed_ch0 = mmap(0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, (BASE_ZED_CH) & ~page_size);
    if (zed_ch0 == (void *) -1)
    {
        printf("Can't map the memory=0x%.8lX to user space\r\n",
                (long unsigned int) (BASE_ZED_CH));
        exit(0);
    }
    printf(
            "HW zed_ch0 Memory=0x%.8lX mapped to user space at mapped_base=%p\r\n",
            (long unsigned int) (BASE_ZED_CH), (void *) zed_ch0);

    //*((unsigned int) (zed_ch0 + CMD_AND_STATUS)) = 0x8000;
    //*((unsigned int) (zed_ch0 + CMD_AND_STATUS)) = 0;

    ///////////////// DONE PAUL'S CODE //////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////
                ////////////// ready to go //////////////
    //////////////////////////////////////////////////////////////////////

    while(1)
    {
    	if(receive(communicationSocket, communicationBuffer,messageSender) >= 0)
    	{

    		process_cmd(communicationBuffer.c_str(), (long int)zed_ch0);
    		while(1)
    		{
    			send(streamingSocket, streamingReceiver, communicationBuffer);
    			usleep(10000);
    		}
    	}

    }

    close(communicationSocket);

    return 0;
}


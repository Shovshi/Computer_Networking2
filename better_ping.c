// icmp.cpp
// Robert Iakobashvili for Ariel uni, license BSD/MIT/Apache
//
// Sending ICMP Echo Requests using Raw-sockets.
//

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <unistd.h>

#define SERVER_PORT 3000
#define SERVER_IP_ADDRESS "127.0.0.1"


// IPv4 header len without options
#define IP4_HDRLEN 20

// ICMP header len for echo req
#define ICMP_HDRLEN 8

    // Checksum algo
    unsigned short calculate_checksum(unsigned short *paddress, int len);

    // 1. Change SOURCE_IP and DESTINATION_IP to the relevant
    //     for your computer
    // 2. Compile it using MSVC compiler or g++
    // 3. Run it from the account with administrative permissions,
    //    since opening of a raw-socket requires elevated preveledges.
    //
    //    On Windows, right click the exe and select "Run as administrator"
    //    On Linux, run it as a root or with sudo.
    //
    // 4. For debugging and development, run MS Visual Studio (MSVS) as admin by
    //    right-clicking at the icon of MSVS and selecting from the right-click
    //    menu "Run as administrator"
    //
    //  Note. You can place another IP-source address that does not belong to your
    //  computer (IP-spoofing), i.e. just another IP from your subnet, and the ICMP
    //  still be sent, but do not expect to see ICMP_ECHO_REPLY in most such cases
    //  since anti-spoofing is wide-spread.

    #define SOURCE_IP "127.0.0.1"
    // i.e the gateway or ping to google.com for their ip-address
    #define DESTINATION_IP "8.8.8.8"

    int main(int argc, char **argv)
    {
        
        char *args[2];
        args[0]="./watchdog";
        args[1]=NULL;
        int status;
        int pid = fork();
        printf("%d\n", pid);

        if (pid == 0)
        {
          //  printf("%d\n", pid);
            execvp(args[0],args);
        }
        sleep(1);
    

        // creating a TCP socket using IPV4
        int sockTCP = socket(AF_INET, SOCK_STREAM, 0);

        if (sockTCP == -1)
        {
            printf("No socket was created\n");
            return -1;
        }
        else
        {
            printf("Socket Created\n");
        }

        struct sockaddr_in serverAddress;
        memset(&serverAddress, 0, sizeof(serverAddress)); // we initiliaze the memory at sin_zero

        serverAddress.sin_family = AF_INET;

        // we turn the server ip address from a binary sequence to a network representation
        // and then check if it worked

        int check = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr);
        if (check > 0)
        {
            printf("Successfully converted the address\n");
        }
        else
        {
            printf("Unsuccessfully converted the address\n");
            close(sockTCP);
            return -1;
        }

        // we convert the server port into a Network Byte Order representation
        serverAddress.sin_port = htons(SERVER_PORT);


        // we connect the socket to the server and check if we have succeeded
        int didConnect = connect(sockTCP, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        if (didConnect >= 0)
        {
            printf("Successfully connected\n");
        }
        else
        {
            printf("%d", didConnect);
            printf("Unsuccessfully connected\n");
            close(sockTCP);
            return -1;
        }


        //we get the ip address from the user
        char *ipDestination=argv[1];

        //we check if the ip input is valid
        struct in_addr addr;
        int check_ip = inet_pton(AF_INET,ipDestination, &addr);

        if (check_ip <= 0)
        {
            printf("Invalid IP address\n");
            return -1;
        }

        //initalized the sequence number
        int sNumber=0;

         struct sockaddr_in dest_in;
            memset(&dest_in, 0, sizeof(struct sockaddr_in));
            dest_in.sin_family = AF_INET;

            // The port is irrelant for Networking and therefore was zeroed.
            // dest_in.sin_addr.s_addr = iphdr.ip_dst.s_addr;
            dest_in.sin_addr.s_addr = inet_addr(ipDestination);
            // inet_pton(AF_INET, DESTINATION_IP, &(dest_in.sin_addr.s_addr));

            // Create raw socket for IP-RAW (make IP-header by yourself)
            int sock = -1;
            if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
            {
                fprintf(stderr, "socket() failed with error: %d", errno);
                fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
                return -1;
            }
        while (1)
        {
            struct icmp icmphdr; // ICMP-header

        
            char data[IP_MAXPACKET] = "This is the ping.\n";

            int datalen = strlen(data) + 1;

            //===================
            // ICMP header
            //===================

            // Message Type (8 bits): ICMP_ECHO_REQUEST
            icmphdr.icmp_type = ICMP_ECHO;

            // Message Code (8 bits): echo request
            icmphdr.icmp_code = 0;

            // Identifier (16 bits): some number to trace the response.
            // It will be copied to the response packet and used to map response to the request sent earlier.
            // Thus, it serves as a Transaction-ID when we need to make "ping"
            icmphdr.icmp_id = 18;

            // Sequence Number (16 bits): starts at 0
            icmphdr.icmp_seq = 0;

            // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation
            icmphdr.icmp_cksum = 0;

            // Combine the packet
            char packet[IP_MAXPACKET];

            // Next, ICMP header
            memcpy((packet), &icmphdr, ICMP_HDRLEN);

            // After ICMP header, add the ICMP data.
            memcpy(packet + ICMP_HDRLEN, data, datalen);

            //Calculate the ICMP header checksum
            icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + datalen);
            memcpy((packet), &icmphdr, ICMP_HDRLEN);


            struct timeval start, end;
            gettimeofday(&start, 0);


                // Send the packet using sendto() for sending datagrams.
                int bytes_sent = sendto(sock, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *)&dest_in, sizeof(dest_in));
                if (bytes_sent == -1)
                {
                    fprintf(stderr, "sendto() failed with error: %d", errno);
                    return -1;
                }
                printf("Successfuly sent one packet : ICMP HEADER : %d bytes, data length : %d , icmp header : %d \n", bytes_sent, datalen, ICMP_HDRLEN);

                // we send watchdogs a message saying we sent a ping
                char ok [] ={1};
                int length = strlen(ok) + 1;
                int checkSend = send(sockTCP, ok , length , 0);
                if(checkSend <= 0)
                {
                    printf("Error");
                }
                
                // Get the ping response
                bzero(packet, IP_MAXPACKET);
                socklen_t len = sizeof(dest_in);
                ssize_t bytes_received = -1;

                

                bytes_received = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_in, &len);
                
                    if (bytes_received > 0)
                    {
                        // Check the IP header
                        struct iphdr *iphdr = (struct iphdr *)packet;
                        struct icmphdr *icmphdr = (struct icmphdr *)(packet + (iphdr->ihl * 4));
                        printf("%ld bytes from %s\n", bytes_received, inet_ntoa(dest_in.sin_addr));
                        char sourceIPAddrReadable[32] = { '\0' };
                        inet_ntop(AF_INET, &iphdr->saddr, sourceIPAddrReadable, sizeof(sourceIPAddrReadable));
                        printf("Successfuly received one packet with %d bytes : data length : %d , icmp header : %d , ip header : %d \n", (int)bytes_received, datalen, ICMP_HDRLEN, IP4_HDRLEN);
                       
                        gettimeofday(&end, 0);
                        float milliseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
                        unsigned long microseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec);
                        printf("\nRTT: %f milliseconds (%ld microseconds)\n", milliseconds, microseconds);
                        printf ("Packet IP: %s --- Packet Sequence Number: %d --- Time between request and replay: %f milliseconds\n" , sourceIPAddrReadable, sNumber ,milliseconds );
                        sNumber++;                        
                    }


        

            

            char reply[IP_MAXPACKET];
            memcpy(reply, packet + ICMP_HDRLEN + IP4_HDRLEN, datalen);
            // printf("ICMP reply: %s \n", reply);
            


        } 

        // Close the raw socket descriptor.
        close(sock);
        return 0;
    }

    // Compute checksum (RFC 1071).
    unsigned short calculate_checksum(unsigned short *paddress, int len)
    {
        int nleft = len;
        int sum = 0;
        unsigned short *w = paddress;
        unsigned short answer = 0;

        while (nleft > 1)
        {
            sum += *w++;
            nleft -= 2;
        }

        if (nleft == 1)
        {
            *((unsigned char *)&answer) = *((unsigned char *)w);
            sum += answer;
        }

        // add back carry outs from top 16 bits to low 16 bits
        sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
        sum += (sum >> 16);                 // add carry
        answer = ~sum;                      // truncate to 16 bits

        return answer;
    }
    
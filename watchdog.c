#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#define SERVER_PORT 3000
#define SERVER_IP_ADDRESS "127.0.0.1"

int main()
{
    // we create a socket and then check for success
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("No socket was created");
        return -1;
    }
    else
    {
        printf("Socket Created\n");
    }

    // we build a struct holding the server address data and we initialize
    // the memory in this address
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    // we set the field to AF_INET
    serverAddress.sin_family = AF_INET;

    // we convert the server port into a Network Byte Order representation
    serverAddress.sin_port = htons(SERVER_PORT);

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
        return -1;
    }

    // in order to re-use the port we enter the following code
    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    // we bind the server address and port to the socket and check for success
    int didBind = bind(sock, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (didBind == -1)
    {
        printf("Unsuccessfully binded\n");
        close(sock);
        return -1;
    }
    else
    {
        printf("Successful bind\n");
    }

    // the server is going into " waiting " mode and we check for success
    int didListen = listen(sock, 500);
    if (didListen == -1)
    {
        printf("Unseccessful attempt to listen\n");
    }
    else
    {
        printf("---------Waiting for TCP connection---------\n");
    }

    // we build a struct for the client address data and initialize the memory
    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    memset(&client, 0, sizeof(client));
    clientLen = sizeof(client);

    // we request permission to connect from the queue of requests and check for succession
    int clientS = accept(sock, (struct sockaddr *)&client, &clientLen);
    if (clientS == -1)
    {
        printf("Unsuccessfull accept\n");
    }
    else
    {
        printf("New Connection\n");
    }

    // we check if we received a message from the pinger , if more then 10 seconds have passed
    // without a message coming in we kill the program
    char msg[1] = {0};

    // we setup our structs to use them as timers
    struct timeval startSeconds;
    struct timeval endSeconds;

    int seconds = 0;

    int length = strlen(msg) + 1;
    
    // we start the timer before the while loop so it only resets when we get a 
    // message
    gettimeofday(&startSeconds, 0);

    while(seconds < 10)
    {
        int bytes = recv(clientS , msg , length , MSG_DONTWAIT);
        //printf("%d THIS IS BYTES\n", bytes);
        if(bytes > 0)
        {
            gettimeofday(&startSeconds, 0);
        }
        else
        {
            gettimeofday(&endSeconds , 0);
            seconds = endSeconds.tv_sec - startSeconds.tv_sec;
        }
    }
    // if we reach here we need to close the sockets and kill the parent process
    close(sock);
    close(clientS);
    kill(getppid(), SIGKILL);

    return 0;
}
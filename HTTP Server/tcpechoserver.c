// Simple TCP echo server

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Max message to echo
#define MAX_MESSAGE 1000000

/* server main routine */

int main(int argc, char **argv) {
    // locals
    unsigned short port = 22222;  // default port
    int sock;                     // socket descriptor
    char* okMessage = "HTTP/1.1 200 OK\n\n";
    char* errMessage = "HTTP/1.1 404 Not Found \n\n";

    // Was help requested?  Print usage statement
    if (argc > 1 && ((!strcmp(argv[1], "-?")) || (!strcmp(argv[1], "-h")))) {
        printf(
            "\nUsage: tcpechoserver [-p port] port is the requested \
 port that the server monitors.  If no port is provided, the server \
 listens on port 22222.\n\n");
        exit(0);
    }

    // get the port from ARGV
    if (argc > 1 && !strcmp(argv[1], "-p")) {
        if (sscanf(argv[2], "%hu", &port) != 1) {
            perror("Error parsing port option");
            exit(0);
        }
    }

    // ready to go
    printf("tcp echo server configuring on port: %d\n", port);

    // for TCP, we want IP protocol domain (PF_INET)
    // and TCP transport type (SOCK_STREAM)
    // no alternate protocol - 0, since we have already specified IP

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error on socket creation");
        exit(1);
    }

    // lose the pesky "Address already in use" error message
    int yes = 1;

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    // establish address - this is the server and will
    // only be listening on the specified port
    struct sockaddr_in sock_address;

    // address family is AF_INET
    // our IP address is INADDR_ANY (any of our IP addresses)
    // the port number is per default or option above

    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_address.sin_port = htons(port);

    // we must now bind the socket descriptor to the address info
    if (bind(sock, (struct sockaddr *)&sock_address, sizeof(sock_address)) <
        0) {
        perror("Problem binding");
        exit(-1);
    }

    // extra step to TCP - listen on the port for a connection
    // willing to queue 5 connection requests
    if (listen(sock, 5) < 0) {
        perror("Error calling listen()");
        exit(-1);
    }

    // go into forever loop and echo whatever message is received
    // to console and back to source
    char *buffer = calloc(MAX_MESSAGE, sizeof(char));
    int bytes_read;
    int echoed;

    while (1) {
        // hang in accept and wait for connection
        printf("====Waiting====\n");
        int newsockfd = accept(sock, (struct sockaddr *)NULL, NULL);
        if (newsockfd < 0) {
            perror("Error on accept");
        }
        int pid = fork();
        if (pid < 0) {
            perror("Error on fork");
        }
        if (pid == 0) {
            close(sock);

            while (1) {
                buffer[0] = '\0';  // guarantee a null here to break out on a
                                   // disconnect

                // read message
                bytes_read = read(newsockfd, buffer, MAX_MESSAGE - 1);

                //Parse the buffer request
                char* filename = calloc(MAX_MESSAGE, sizeof(char));
                char* parsedName;

                //Returns Get
                parsedName = strtok(buffer, " ");
                //Returns request after GET
                parsedName = strtok(NULL, " ");
                //Remove / 
                for(int i = 0; i < strlen(parsedName); i++){
                    parsedName[i] = parsedName[i + 1];
                }
                filename = parsedName;

                //Open file with desired filename
                FILE* file = fopen(filename, "rb");
                char *data = calloc(MAX_MESSAGE, sizeof(char));
                memset(buffer, 0, MAX_MESSAGE);
                //Check to make sure file exists
                if (file != NULL){
                    fread(buffer, 1, MAX_MESSAGE, file);
                    memcpy(data, okMessage, strlen(okMessage));
                    memcpy(data + strlen(okMessage), buffer, MAX_MESSAGE);
                }else {
                    file = fopen("error404.html", "rb");
                    fread(buffer, 1, MAX_MESSAGE, file);
                    memcpy(data, errMessage, strlen(errMessage));
                    memcpy(data + strlen(errMessage), buffer, MAX_MESSAGE);
                }

                if (bytes_read == 0) {  // socket closed
                    printf("====Client Disconnected====\n");
                    close(newsockfd);
                    break;  // break the inner while loop
                }

                // make sure buffer has null temrinator
                buffer[bytes_read] = '\0';

                // see if client wants us to disconnect
                if (strncmp(buffer, "quit", 4) == 0) {
                    printf("====Server Disconnecting====\n");
                    close(newsockfd);
                    break;  // break the inner while loop
                }

                // print info to console
                printf("Received message\n");

                // put message to console
                printf("Message: %s\n", buffer);

                // send it back to client
                if ((echoed = write(newsockfd, data, MAX_MESSAGE)) < 0) {
                    perror("Error sending echo");
                    exit(-1);
                } else {
                    printf("Bytes echoed: %d\n", echoed);
                }
            }
            exit(0);
        } else {
            close(newsockfd);
        }
    }
}
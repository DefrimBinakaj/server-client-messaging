#include "server.h"

// for ip:
// hostname -i

// sources:
// https://www.linuxhowtos.org/C_C++/socket.htm?userrate=2
// https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
// https://www.youtube.com/watch?v=Y6pFtgRdUts&list=PL9IEJIKnBJjH_zM5LnovnoaKlXML5qh17&index=8&t=16s
// https://stackoverflow.com/questions/15673846/how-to-give-to-a-client-specific-ip-address-in-c
// https://lists.debian.org/debian-hurd/2012/03/msg00020.html


// error function
void error(const char *msg) {
    perror(msg);
    exit(1);
}


// function to get the current epoch time
double currentTime() {
    // https://www.epochconverter.com/
    // https://techoverflow.net/2021/09/06/how-to-get-time-since-epoch-in-milliseconds-in-c/
    uint64_t timeSinceEpochMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()
    ).count();
    double doubleTimeMilli = (double) timeSinceEpochMilliseconds;
    double doubleTime = doubleTimeMilli / 1000;
    // printf("%.2f", doubleTime);
    return doubleTime;
}



int main(int argc, char *argv[]) {

    // inits
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    int transactionNum = 0;


    // check input for port
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    // convert port input into int
    portno = atoi(argv[1]);

    // check input range(5k,64k)
    if (portno < 5000 || portno > 64000) {
        fprintf(stderr, "ERROR, port must be in range 5,000 to 64,000\n");
        exit(1);
    }



    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR making socket");
    }

    // set buffer values to zero
    bzero((char *) &serv_addr, sizeof(serv_addr));



    /* set serv_addr fields: */
    // set address family to AF_INET
    serv_addr.sin_family = AF_INET;
    // set IP address of the host either:
    // INADDR_ANY (IP of current machine)
    // <variable> from input
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // set port number to portno, after converting to network byte order using htons
    serv_addr.sin_port = htons(portno);
    // cout << INADDR_ANY << endl;

    // bind socket to an address and port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    // listen to socket for connections
    listen(sockfd,10);


    // block process until client connects
    clilen = sizeof(cli_addr);

    fd_set current_sockets, ready_sockets;


    // clear socket set
    FD_ZERO(&current_sockets);
    // add main socket to set
    FD_SET(sockfd, &current_sockets);




    
    // create and redirect to output file
    if (!freopen("Server Output.txt", "w", stdout)) {
        error("redirect failed");
    }
    
    
    
    // intro print
    printf("Using port %d\n", portno);
    fflush(stdout);
    
    // times
    double startingTime = currentTime();
    double now;

    // map for transNum of each client
    map <string, int> allClient;



    while (true) {

        // timeout set to 30 seconds
        // https://stackoverflow.com/questions/9494626/c-select-not-waiting-for-timeout-period
        struct timeval serverTimeout;
        serverTimeout.tv_sec = 30; // seconds
        serverTimeout.tv_usec = 0; // microseconds

        ready_sockets = current_sockets;

        int select_result = select(FD_SETSIZE, &ready_sockets, NULL, NULL, &serverTimeout);

        // this commented out section is used for 
        // breaking the server when there are no more clients
        /*
        if (select_result < 0){
            printf("out of fds");
            fflush(stdout);
            break;

        } 
        */



        if (select_result == 0){
            printf("Server has timed out -- no messages for 30 seconds\n\n");
            fflush(stdout);
            break;
        }
        

        for (int i = 0; i < FD_SETSIZE; i++){
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == sockfd){
                    if ((newsockfd = accept(sockfd, (struct sockaddr *) NULL, NULL)) < 0) {
                        error("ERROR, accept");
                    }
                    // client connected -> add them to current sockets
                    FD_SET(newsockfd, &current_sockets);
                }
                else {
                    // init buffer using bzero, then read from socket after connection
                    bzero(buffer,256);
                    n = read(i,buffer,255);



                    if (n < 0) {
                        error("Error, message");
                    }
                    else if (n > 0) {

                        transactionNum += 1;

                        char paramType;
                        int paramNum = 0;
                        char clientName[64];
                        

                        // scan in buffer and set values accordingly
                        sscanf(buffer, "%s %d %s", &paramType, &paramNum, clientName);


                        // print pre-transaction message
                        now = currentTime();
                        printf("%.2f: # %d  (%c%d)  from %s\n", now, transactionNum, paramType, paramNum, clientName);
                        fflush(stdout);

                        // client info trans init / incr
                        if (allClient.find(clientName) == allClient.end()) {
                            allClient.emplace(clientName, 1);
                        } 
                        else {
                            allClient.at(clientName) += 1;
                        }



                        // execute Trans
                        Trans(paramNum);

                        // print post-transaction message
                        now = currentTime();
                        printf("%.2f: # %d  (Done)  from %s\n", now, transactionNum, clientName);
                        fflush(stdout);


                        // write to the client
                        string writeMessage = "Recv  (D";
                        writeMessage += to_string(transactionNum) + ")";

                        n = write(i,writeMessage.c_str(),writeMessage.size());
                        if (n < 0) {
                            error("ERROR writing to socket");
                        }





                    }
                    else if (n == 0) {
                        // client disconnected
                        close (i);
                        FD_CLR(i, &current_sockets);
                    }



                }

            }
        }

    }


    // summary
    double endTime = currentTime();
    printf("SUMMARY\n");
    
    // print all client specific transNum
    // https://stackoverflow.com/questions/14555751/how-to-iterate-over-a-c-stl-map-data-structure-using-the-auto-keyword
    for (auto it: allClient) {
        printf("%d transactions from %s\n", it.second, it.first.c_str());
    }
    
    // total calculation and print
    double totalTransPerSec = transactionNum / (endTime - startingTime-30);
    printf("%.1f transaction/sec (%d/%.2f)\n", totalTransPerSec, transactionNum, (endTime - startingTime-30));
    fflush(stdout);


    FD_ZERO(&current_sockets);
    FD_ZERO(&ready_sockets);

    // terminate
    close(newsockfd);
    close(sockfd);
    return 0; 
}
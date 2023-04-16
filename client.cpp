#include "client.h"

// error function
void error(const char *msg) {
    perror(msg);
    exit(0);
}


// command function that returns command type
// this must be used, otherwise the while loop doesnt work
Command getCmdType(){

    Command command;
    char commandArg;

    if (scanf("%c%d", &commandArg, &command.parameter) != EOF){ 

        if (commandArg == 'T'){
            command.type = "T";
        } 
        else if (commandArg == 'S'){
            command.type = "S";
        }
    }
    else {
        command.type = "Z";
    }

    return command;

}

// function to get the current epoch time
double currentTime() {
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
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int transactionNum = 0;


    // check input for hostname and port
    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"ERROR, input must be: %s <port> <ip-address>\n", argv[0]);
       exit(0);
    }


    // convert port input into int
    portno = atoi(argv[1]);

    // check input range(5k,64k)
    if (portno < 5000 || portno > 64000) {
        fprintf(stderr, "ERROR, port must be in range 5,000 to 64,000\n");
        exit(0);
    }

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    
    // take host name as arg and return pointer to a hostent
    server = gethostbyname(argv[2]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }


    // init buffer using bzero, then connect
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }


    // enter string to write to the fd
    printf("Please enter the message: \n");
    bzero(buffer,256);


    // create and redirect to output file
    char hostname[50];
    if (gethostname(hostname, 50) < 0){
        error("Error, hostname");
        exit(-1);
    }
    // inits for names used for sending
    string ugName = hostname;
	char fileName[50];
    int pid = getpid();
    string entireName = ugName + "." + to_string(pid);

    // open output file
    sprintf(fileName, "%s.%d Output.txt",ugName.c_str(), pid);
    if (!freopen(fileName, "w", stdout)) {
        error("redirect failed");
    }

    // print necessary intro lines
    printf("Using port %d\n", portno);
    printf("Using server address %s\n", argv[2]);
    printf("Host %s.%d\n", ugName.c_str(), pid);


    // other buffer init
    char buffer2[256];

    // time init
    // double startingTime = currentTime();
    double now;


    // command init
    Command command;

    command = getCmdType();

    while (command.type != "Z"){

        if(command.type == "T"){
            
            // incr transNum
            transactionNum += 1;

            // set buffer to %s%d
            bzero(buffer,256);
            snprintf(buffer, sizeof(buffer), "%s %d %s", command.type.c_str(), command.parameter, entireName.c_str());




            // pre-send print
            now = currentTime();
            printf("%.2f: Send  (%s%d)\n", now, command.type.c_str(), command.parameter);

            // write to the server
            // https://stackoverflow.com/questions/22648978/c-how-to-find-the-length-of-an-integer
            n = write(sockfd,buffer,trunc(log10(command.parameter))+2 + entireName.size()+8);
            if (n < 0) {
                error("ERROR writing to socket");
            }
            


            // read from server and print result
            bzero(buffer2,256);
            
            n = read(sockfd,buffer2,255);
            if (n < 0) {
                error("ERROR reading from socket");
            }

            // post-receive print
            now = currentTime();
            printf("%.2f: ", now);

            printf("%s\n",buffer2);
            

            bzero(buffer,256);
            bzero(buffer2,256);
        
        } 
        else if(command.type == "S"){
            
            printf("Sleep %d units\n", command.parameter);
            fflush(stdout);
            Sleep(command.parameter);

        }
        

        command = getCmdType();
    }
    printf("Sent %d transactions\n", transactionNum);
    fflush(stdout);
    
    // terminate
    close(sockfd);
    return 0;
}
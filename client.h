#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/resource.h>
#include <bits/stdc++.h>
#include <map>
#include <pthread.h>
#include <chrono>
#include <fstream>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "tands.h"

using namespace std;

struct Command {
    string type;
    int parameter;
};

void error(const char *msg);

Command getCmdType();

double currentTime();

int main(int argc, char *argv[]);


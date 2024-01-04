#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <sqlite3.h>
#include <time.h>

#define NAME_SIZE 20
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024


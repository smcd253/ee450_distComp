/****** config.h *******/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>

// library for close function
// source: http://stackoverflow.com/questions/42736958/undeclared-identifier-close-macos-gcc
#include <unistd.h> 

// constant definitions
#define SIZE 30
#define MAX_SIZE 1000
#define BUFF_SIZE 4096

// port definitions
#define EDGE_PORT_TCP 23166
#define EDGE_PORT_UDP 24166
#define AND_PORT_UDP 22166
#define OR_PORT_UDP 21166


#endif 

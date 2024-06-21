#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include "udp_tx.hpp" 
#include <getopt.h>
#include <sys/select.h>
#include <cstdlib>

// might need to pass in integer for boardID

using namespace std; 



bool udp_init(int * client_socket, uint8_t boardId){

    int ret;
    char ipAddress[14] = "169.254.10.";

    if (boardId > 15){
        return false; 
    }
    
    // **change sprint statemnt to use %s , no need for conditionals

    
    snprintf(ipAddress + strlen(ipAddress) , sizeof(ipAddress) - strlen(ipAddress), "%d", boardId);


    *client_socket = socket(AF_INET, SOCK_DGRAM, 0);


    if (*client_socket < 0) {
        return false;
    }

    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    inet_pton(AF_INET, ipAddress, &server_address.sin_addr);

    ret = connect(*client_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    if (ret != 0) {
        cout << "Failed to connect to server [" << ipAddress << "]" << endl;
        close(*client_socket);
        return false;
    }

    return true;
}

bool udp_transmit(int client_socket, void * data, int size){
    
    // change UDP_MAX_QUADLET to 
    if (size > UDP_REAL_MTU){
        return false;
    }

    if (send(client_socket, data, size, 0) >= 0){
        return true;
    } else {
        return false;
    } 
}

// might want to do nonblocking to program a stop key 
bool udp_receive(int client_socket, void *data, int len)
{
        uint16_t recieved_bytes = recv(client_socket, data, len, 0);

        if (recieved_bytes > 0){
            return true;
        } else {
            return false;
        }
}

int udp_nonblocking_receive(int client_socket, void *data, int len){

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client_socket, &readfds);

    int ret_code = isDataAvailable(&readfds, client_socket) ;

    if(ret_code == UDP_DATA_IS_AVAILBLE ){
        if (FD_ISSET(client_socket, &readfds)) {
            ret_code = recv(client_socket, data, len, 0);
            if (ret_code == 0){
                return UDP_CONNECTION_CLOSED_ERROR;
            } else if (ret_code < 0){
                return UDP_SOCKET_ERROR;
            } else {
                return UDP_DATA_IS_AVAILBLE;
            }
        }
    }

    return ret_code;
}



// checks if data is available from console in or udp buffer
int isDataAvailable(fd_set *readfds, int client_socket){
    struct timeval timeout;

    // timeout valus
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    int max_fd = client_socket + 1;
    int activity = select(max_fd, readfds, NULL, NULL, &timeout);

    if (activity < 0){
        return UDP_SELECT_ERROR;
    } else if (activity == 0){
        return UDP_DATA_IS_NOT_AVAILABLE_WITHIN_TIMEOUT;
    } else {
        return UDP_DATA_IS_AVAILBLE;
    }
}


bool udp_close(int * client_socket){
    close(*client_socket);
    return true;
}
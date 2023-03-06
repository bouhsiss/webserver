#include "socket.hpp"

int main () {
    struct sockaddr_in serv_addr;
    memset(serv_addr.sin_zero, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    int clientSocket = socket(serv_addr.sin_family, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cout << "cannot create client socket" << std::endl;
        return(EXIT_FAILURE);
    }
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cout << std::endl <<"Invalid address " << std::endl;
        return(EXIT_FAILURE);
    }

    if(connect(clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection Failed" << std::endl;
        return(EXIT_FAILURE);
    }
    
    std::string hello = "Hello from client";
    char buffer[1024] = {0};
    send(clientSocket, hello.c_str(), hello.size(), 0);
    std::cout << "Hello message sent from server" << std::endl;
    int valread = read(clientSocket, buffer, 1024);
    std::cout << buffer << std::endl;
    return(EXIT_SUCCESS);
}
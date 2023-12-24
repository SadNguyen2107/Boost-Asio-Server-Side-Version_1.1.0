#include "../include/Server.h"
#include <iostream>

Server::Server()
{
    std::cout << "Server constructor" << std::endl;
}

Server::~Server()
{
    std::cout << "Server destructor" << std::endl;
}

void Server::Start()
{
    std::cout << "Server started" << std::endl;
}

void Server::Stop()
{
    std::cout << "Server stopped" << std::endl;
}
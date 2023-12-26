#ifndef SERVER_H
#define SERVER_H

#include <utility> // Include this line before Boost.Asio headers
#include <boost/asio.hpp>
#include "./config/export_libs.h"
#include <memory>
#include <string>
#include <stdint.h>
#include <iostream>
#include <csignal>
#include <thread>
#include <atomic>
#include <set>

namespace SN_Server
{
    class Server
    {
    private:
        //* Create io_context for Server
        boost::asio::io_context io_context;

        // Server IP Address Object
        boost::asio::ip::address_v4 server_ipv4_address;

        // Server End Point Object
        boost::asio::ip::tcp::endpoint server_endpoint;

        // To Concurrently shutdown
        std::atomic<bool> is_running;

        // Acceptor Object
        // For Listenning to new Connections
        std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_server;

        // Listenning Thread To Accpet New Client Connections
        std::shared_ptr<std::thread> listening_thread;

        // To Store All The Client Connections
        std::set<std::shared_ptr<boost::asio::ip::tcp::socket>> clientsConnections;

        //! PRIVATE METHODS SECTIONS
        //!========================================================
        //* Methods To Accept new Connection
        void AcceptConnections();

        //* Method To Handle Accept
        void HandleAccept(const boost::system::error_code &error, std::shared_ptr<boost::asio::ip::tcp::socket> client_socket);

        //* Method to Handle User Sending
        void HandleClient(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket);
    public:
        Server(std::string_view server_ipv4_address = "127.0.0.1", std::uint16_t port = 6969);
        ~Server();

        // Simple I/O To Start and Stop
        void Start();
        void Stop();

        bool IsRunning();

        // Simple I/O Send Protocol
        void SendText(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, std::string_view text);
        void SendJSON(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, std::string_view json_file_directory);
        void SendFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, std::string_view file_directory);
        
        // Simple I/O Get Protocol
        void GetText();
        void GetJSON();
        void SendFile();
    };
}

#endif // SERVER_H
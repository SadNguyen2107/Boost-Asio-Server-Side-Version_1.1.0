#ifndef SERVER_H
#define SERVER_H

#include <utility> // Include this line before Boost.Asio headers
#include <boost/asio.hpp>
#include "./config/export_libs.h"
#include <memory>
#include <stdint.h>
#include <thread>
#include <atomic>
#include <set>

namespace SN_Server
{
    enum ClientConnectionStatus
    {
        ConnectionOpen = 0b1,
        ConnectionClose = 0b10
    };

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
        std::set<std::shared_ptr<boost::asio::ip::tcp::socket>> clients_connections;

        // Chunk Size of Data to Send/Get
        std::size_t CHUNK_SIZE = 255;

        //! PRIVATE METHODS SECTIONS
        //!========================================================
        //* Methods To Accept new Connection
        void AcceptConnections();

        //* Method To Handle Accept
        void HandleAccept(const boost::system::error_code &error, std::shared_ptr<boost::asio::ip::tcp::socket> client_socket);

        //* Method to Handle User Sending
        void HandleClient(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket);
    public:
        Server(std::string_view server_ipv4_address = "127.0.0.1", std::uint16_t port = 5000);
        ~Server();

        // Simple I/O To Start and Stop
        void Start();
        void Stop();

        bool IsRunning();

        // Set-Get The Chunk of data
        void SetChunkData(std::size_t new_chunk_size);
        std::size_t GetChunkData() const;

        //========================================================================================================================
        // Simple I/O Send Protocol
        void SendText(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string_view& text);

        // For Sending Text-Based Formats Files
        void SendTextBasedFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string& file_to_send);

        // For Sending Binary Formats Files
        void SendBinaryFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string& file_to_send);
        
        //========================================================================================================================
        // Simple I/O Get Protocol
        ClientConnectionStatus GetText(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, std::string &received_text);

        // For Receiving Text-Based Formats Files
        ClientConnectionStatus GetTextBasedFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string &file_to_store);

        // For Receiving Binary Formats Files
        ClientConnectionStatus GetBinaryFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string &file_to_store);
    };
}

#endif // SERVER_H
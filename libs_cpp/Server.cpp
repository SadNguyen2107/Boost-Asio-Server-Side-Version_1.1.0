#include "../include/Server.h"

namespace SN_Server
{
    /**
     * @brief Construct a new Server:: Server object
     *
     * @param server_ipv4_address_str the IP_V4 Address Of the Default Gateway that the Server use
     * @param port the PORT that the Server to open
     */
    Server::Server(std::string_view server_ipv4_address_str, std::uint16_t port)
    {
        //* Convert the IP Address string to an IP Address Object
        this->server_ipv4_address = boost::asio::ip::address_v4(
            boost::asio::ip::make_address_v4(server_ipv4_address_str));

        //* Make a Server Endpoint
        this->server_endpoint = boost::asio::ip::tcp::endpoint(
            this->server_ipv4_address, port);

        //* Listening for any new incomming connection
        this->acceptor_server = std::make_shared<boost::asio::ip::tcp::acceptor>(
            this->io_context,
            this->server_endpoint);

        // Show a Log of SET UP SERVER
        std::cout << "Sever Configuration..." << std::endl;
        std::cout << "Server Address: " << this->server_endpoint.address() << std::endl;
        std::cout << "Server Port Opening: " << this->server_endpoint.port() << std::endl;
    }

    Server::~Server()
    {
        this->Stop();
    }

    /**
     * @brief Start Accepting New Client-Socket Connection
     *
     */
    void Server::Start()
    {
        // Change the Atomic Variable To True
        this->is_running = true;

        // Make A Listening Thread
        this->listening_thread = std::make_shared<std::thread>([this](){ 
            this->AcceptConnections(); 
        });
    }

    void Server::Stop()
    {
        // Change The Atomic Variable To False
        // Set the flag to stop accepting new connections
        this->is_running = false;

        // Gracefully shutdown all the connections
        for (std::shared_ptr<boost::asio::ip::tcp::socket> client_socket : clientsConnections)
        {
            if (client_socket->is_open())
            {
                std::cout << "Shutting Connection with "
                          << client_socket->remote_endpoint().address() << ":"
                          << client_socket->remote_endpoint().port() << "\n";
                // Send a shutdown message and close the client socket
                // sendData(client_socket, "CLOSE BY SERVER");
                client_socket->close();
            }
        }

        // Clear The Set of Client_socket
        this->clientsConnections.clear();

        // Stop accepting new connections
        std::cout << "Stopped accepting new connections." << std::endl;
        this->acceptor_server->close(); // Close the Acceptor

        // Stop the io_context to exit the run loop
        std::cout << "Server is shutting down. Goodbye!" << std::endl;
        this->io_context.stop(); // Stop the I/O

        // Join the Listenning Threads
        if (this->listening_thread->joinable())
        {
            this->listening_thread->join();
        }

        std::cout << "Stop Running!" << std::endl;
    }

    bool Server::IsRunning()
    {
        return this->is_running;
    }

    //* Send Method Protocol
    void Server::SendText(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, std::string_view text)
    {
        // Synchronous write
        boost::system::error_code error;
        boost::asio::write(*client_socket, boost::asio::buffer(text), error);

        // Check Whether Error Happen
        if (!error)
        {
            std::cout << "Message sent successfully to "
                      << client_socket->remote_endpoint().address() << ":"
                      << client_socket->remote_endpoint().port()
                      << std::endl;
        }
        else
        {
            std::cerr << "Error: " << error.message() << std::endl;
        }
    }

    void Server::SendJSON(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, std::string_view json_file_directory)
    {

    }
    void Server::SendFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, std::string_view file_directory)
    {

    }

    //! PRIVATE METHODS SECTIONS
    //!============================================================================
    void Server::AcceptConnections()
    {
        // Creating a socket to represent a connected client
        // Use Shared_ptr to share the owner ship instead of copying them
        std::shared_ptr<boost::asio::ip::tcp::socket> client_socket = std::make_shared<boost::asio::ip::tcp::socket>(this->io_context);

        // Start an asynchronous accept operation
        this->acceptor_server->async_accept(*client_socket,
        [this, client_socket](const boost::system::error_code &error) {
            this->HandleAccept(error, client_socket);
        });

        // Run the io_context to process asynchronous operations
        this->io_context.run();
    }

    void Server::HandleAccept(const boost::system::error_code &error, std::shared_ptr<boost::asio::ip::tcp::socket> client_socket)
    {
        if (!error)
        {
            // Connection accepted. Handle the connection using client_socket.
            // Get CLIENT'S IP Address and port
            boost::asio::ip::tcp::endpoint client_endpoint = client_socket->remote_endpoint();
            std::cout << "Connected To Client: "
                      << client_endpoint.address()
                      << ":"
                      << client_endpoint.port()
                      << std::endl;

            // Greeting The User
            //? Testing Section
            this->SendText(client_socket,"Hello World\n");

            // Authentication Part

            // Create A thread To Handle The Client
            std::thread client_thread([this](std::shared_ptr<boost::asio::ip::tcp::socket> client_socket) {
                this->HandleClient(client_socket);
            }, client_socket);
            client_thread.detach();

            this->clientsConnections.insert(client_socket);
        }
    }

    void Server::HandleClient(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket)
    {
        std::cout << "Handle Here!" << std::endl;
    }
} // namespace SN_Server

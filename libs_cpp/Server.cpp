#include "../include/Server.h"
#include "../include/encode_decode_base64.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include <iostream>

using namespace JB_Encode_Decode_Base64;
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
            boost::asio::ip::make_address_v4(server_ipv4_address_str)
        );

        //* Make a Server Endpoint
        this->server_endpoint = boost::asio::ip::tcp::endpoint(
            this->server_ipv4_address, port
        );

        //* Listening for any new incomming connection
        this->acceptor_server = std::make_shared<boost::asio::ip::tcp::acceptor>(
            this->io_context,
            this->server_endpoint
        );

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
     * With the default CHUNK_DATA to send and get is 255 characters per send/get
     */
    void Server::Start()
    {
        // Change the Atomic Variable To True
        this->is_running = true;

        // Make A Listening Thread
        this->listening_thread = std::make_shared<std::thread>([this]() { 
            this->AcceptConnections(); 
        });
    }

    /**
     * @brief Stop the Server 
     * Maybe combine with 
     * #include <csignal>
     * 
     * To Make a signal to Stop
     */
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

    /**
     * @brief Set a new CHUNK_SIZE 
     * 
     * @param new_chunk_size new CHUNK_SIZE data. Recommended: 255 as the Default 
     */
    void Server::SetChunkData(std::size_t new_chunk_size)
    {
        if (new_chunk_size > 0)
        {
            this->CHUNK_SIZE = new_chunk_size;
        }
    }

    /**
     * @brief Get the current CHUNK_SIZE data 
     * Default: 255
     * 
     * @return std::size_t the size of CHUNK_SIZE data will be send/get from client
     */
    std::size_t Server::GetChunkData() const
    {
        return this->CHUNK_SIZE;
    }

    //* INFO: For Sending Protocol Method
    /**
     * @brief For Sending Simple Text To a Specify client_socket
     *
     * @param client_socket The Socket Want to Send
     * @param text The Text To Send
     */
    void Server::SendText(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string_view &text)
    {
        // The Variable To check For The Bytes Have Send
        std::size_t total_sent = 0;

        // Send every CHUNK_SIZE character per send
        for (size_t index = 0; index < text.size(); index += this->CHUNK_SIZE)
        {
            std::string_view chunk = text.substr(index, this->CHUNK_SIZE);

            // Synchronous write
            boost::system::error_code error;
            std::size_t bytes_sent = boost::asio::write(*client_socket, boost::asio::buffer(chunk), error);

            // Check Whether Error Happen
            if (!error)
            {
                total_sent += bytes_sent;
                std::cout << "Message sent successfully to "
                          << client_socket->remote_endpoint().address() << ":"
                          << client_socket->remote_endpoint().port()
                          << std::endl;
            }
            else
            {
                std::cerr << "Error: " << error.message() << std::endl;
                break; // Break The Loop On Error
            }
        }

        // Check If All data has been sent
        if (total_sent == text.size())
        {
            std::cout << " All data sent successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Not all data sent. Total sent: " << total_sent << " bytes out of " << text.size() << " bytes." << std::endl;
        }
    }

    /**
     * @brief Call This Function within server object to send a Text-Based Formats \n
     * For Sending Files ends with: \n
     * 1. Plain Text Files (.txt) \n
     * 2. JSON Files (.json) \n
     * 3. XML Files (.xml) \n
     * 4. CSV Files (.csv) \n
     * 5. TCV Files (.tsx) \n
     * And Many More...
     *
     * @param client_socket The client_socket to send the File
     * @param file_to_send The file directory to send
     */
    void Server::SendTextBasedFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string &file_to_send)
    {
        // Open The File JSON and Load every 255 Character To Send
        std::ifstream text_file(file_to_send, std::ios::binary);

        // Check If the File Open Successfully
        if (!text_file.is_open())
        {
            std::cerr << "Error: Unable to open JSON file " << file_to_send << std::endl;
            return;
        }

        // Cut Into Chunk of Data to send
        std::string chunk;

        // The Variable To check For The Bytes Have Send
        std::size_t total_sent = 0;

        while (std::getline(text_file, chunk, static_cast<char>(EOF)) && !chunk.empty())
        {
            // Synchronous write
            boost::system::error_code error;
            std::size_t bytes_sent = boost::asio::write(*client_socket, boost::asio::buffer(chunk), error);

            // Check Whether Error Happen
            if (!error)
            {
                total_sent += bytes_sent;
                std::cout << "Message chunk sent successfully to "
                          << client_socket->remote_endpoint().address() << ":"
                          << client_socket->remote_endpoint().port()
                          << std::endl;
            }
            else
            {
                std::cerr << "Error: " << error.message() << std::endl;
                break; // Break the loop on error
            }
        }

        // Check for end-of-file or error
        if (text_file.eof())
        {
            std::cout << "All data sent successfully!" << std::endl;
        }
        else if (text_file.fail())
        {
            std::cerr << "Error reading binary file: " << file_to_send << std::endl;
        }

        // Check If All data has been sent
        if (total_sent == boost::filesystem::file_size(file_to_send))
        {
            std::cout << " All data sent successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Not all data sent. Total sent: " << total_sent << " bytes out of "
                      << boost::filesystem::file_size(file_to_send) << " bytes." << std::endl;
        }

        // Close the File after Sending Successfully
        text_file.close();
    }

    /**
     * @brief Call This Function within server object to send a Text-Based Formats \n
     * For Sending Files likes: \n
     * 1. Image Files (.jpg, .png, .gif, etc.) \n
     * 2. Binary Documents (.pdf, .docx, .xlsx, etc.) \n
     * 3. Executable Files (.exe) \n
     * 4. Compressed Archives (.zip, .tar, .gz, etc.) \n
     *
     * @param client_socket The client_socket to send the File
     * @param file_to_send The file directory to send
     */
    void Server::SendBinaryFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string &file_to_send)
    {
        // FIXME: Sending Binary Files
        //! Approach 1: Encoding Base 64
        // Create A temp file with subfix .txt
        const std::string temp_file_directory = createTempFile(file_to_send);

        // Encode Binary File into temp file
        encodeFileToFile(file_to_send, temp_file_directory);

        // Then send that temp file
        this->SendTextBasedFile(client_socket, temp_file_directory);

        // Delete that temp file afterward
        int result = std::remove(temp_file_directory.c_str());

        // Check If delete success
        if (result == 0)
        {
            std::cout << "File '" << temp_file_directory << "' deleted successfully." << std::endl;
        }
        else
        {
            std::cerr << "Error deleting file" << std::endl;
        }

        // //! Approach 2: Send Binary
        // // Open The Binary Files
        // std::ifstream binary_file(file_to_send, std::ios::binary);

        // // Check If the file has opened successfully
        // if (!binary_file.is_open())
        // {
        //     std::cerr << "Error: Unable to open binary file " << file_to_send << std::endl;
        //     return;
        // }

        // // The Varible to check for the bytes sent
        // std::size_t total_sent = 0;

        // // Make A Buffer to send
        // char buffer[this->CHUNK_SIZE];
        // while (binary_file.read(buffer, this->CHUNK_SIZE))
        // {
        //     // Synchronous write
        //     boost::system::error_code error;
        //     std::size_t bytes_sent = boost::asio::write(*client_socket, boost::asio::buffer(buffer, binary_file.gcount()), error);

        //     // Check whether an error happened
        //     if (!error)
        //     {
        //         total_sent += bytes_sent;
        //         std::cout << "Bytes chunk sent successfully to "
        //                   << client_socket->remote_endpoint().address() << ":"
        //                   << client_socket->remote_endpoint().port()
        //                   << std::endl;
        //     }
        //     else
        //     {
        //         std::cerr << "Error: " << error.message() << std::endl;
        //         break; // Break the loop on error
        //     }
        // }

        // // Check for end-of-file or error
        // if (binary_file.eof())
        // {
        //     std::cout << "All data sent successfully!" << std::endl;
        // }
        // else if (binary_file.fail())
        // {
        //     std::cerr << "Error reading binary file: " << file_to_send << std::endl;
        // }

        // // Check if all data has been sent
        // if (total_sent == boost::filesystem::file_size(file_to_send))
        // {
        //     std::cout << "All data sent successfully!" << std::endl;
        // }
        // else
        // {
        //     std::cerr << "Not all data sent. Total sent: " << total_sent << " bytes out of "
        //               << boost::filesystem::file_size(file_to_send) << " bytes." << std::endl;
        // }

        // // Close the file after sending successfully
        // binary_file.close();
    }

    //* INFO: For Receiving Protocol Method
    // Simple I/O Get Protocol
    void Server::GetText(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, std::string &received_text)
    {
        // Buffer for Receiving Data
        char buffer[this->CHUNK_SIZE];

        // Variable o calculate bytes received
        std::size_t total_received = 0;

        // Error Code if Thrown
        boost::system::error_code error;

        // Read Until End-Of-Stream
        while (true)
        {
            // Synchronous Read
            std::size_t bytes_received = client_socket->read_some(
                boost::asio::buffer(buffer, this->CHUNK_SIZE),
                error
            );

            if (!error)
            {
                // Append the received data to the text
                total_received += bytes_received;
                received_text.append(buffer, bytes_received);
                std::cout << "Received " << bytes_received << " bytes from "
                          << client_socket->remote_endpoint().address() << ":"
                          << client_socket->remote_endpoint().port()
                          << std::endl;
            }
            else if (error == boost::asio::error::eof)
            {
                // Connection closed by the client
                std::cout << "Connection closed by the client." << std::endl;
                break; // Because The Client Close the Stream
            }
            else
            {
                // An error occurred
                std::cerr << "Error: " << error.message() << std::endl;
                break;
            }
        }

        // Process the received text (you can modify this part based on your needs)
        std::cout << "Received text: " << received_text << std::endl;
        std::cout << "With Total Bytes of: " << total_received << std::endl;
    }

    /**
     * @brief Use this method to get the Text-Based Data File From the given clien_socket and Store in the file_to_store directories
     * 
     * @param client_socket The client_socket sent from
     * @param file_to_store The Place to store the Data
     */
    void Server::GetTextBasedFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string &file_to_store)
    {
        // Open The file to store the received data
        std::ofstream received_file(file_to_store, std::ios::binary | std::ios::app);

        // Check if the file is opened successfully
        if (!received_file.is_open())
        {
            std::cerr << "Error: Unable to open file for receiving data " << file_to_store << std::endl;
            return;
        }

        // Buffer for receiving data
        char buffer[this->CHUNK_SIZE];

        // Amount of bytes received
        std::size_t total_received = 0;

        while (true)
        {
            // Synchronous read
            std::size_t bytes_received = client_socket->read_some(boost::asio::buffer(buffer, this->CHUNK_SIZE));

            // If received bytes
            if (bytes_received > 0)
            {
                // Synchronous write to the received file
                received_file.write(buffer, bytes_received);
                received_file.flush(); // Flush the data to the file

                total_received += bytes_received;
                std::cout << "Received " << bytes_received << " bytes from "
                          << client_socket->remote_endpoint().address() << ":"
                          << client_socket->remote_endpoint().port()
                          << std::endl;
            }
            // The Client CLose the Stream
            else
            {
                // No more data or error on the Client Side
                break;
            }
        }

        // Check If All data has been received
        if (total_received > 0)
        {
            std::cout << "All data received successfully!" << std::endl;
        }
        else
        {
            std::cerr << "No data received or an error occurred." << std::endl;
        }

        // Close the file after receiving data
        received_file.close();
    }

    /**
     * @brief Use this function to get The Binary-Text File from the Client_socket and Store in the file_to_store
     * 
     * @param client_socket The client_socket sent from
     * @param file_to_store The file to place data into
     */
    void Server::GetBinaryFile(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, const std::string &file_to_store)
    {
        //! Decode Base 64
        // Put All the data into a temp file
        std::string temp_file = createTempFile(file_to_store);

        // Open The file to store the received data
        std::ofstream received_file(temp_file, std::ios::binary | std::ios::app);

        // Check if the file is opened successfully
        if (!received_file.is_open())
        {
            std::cerr << "Error: Unable to open file for receiving data " << file_to_store << std::endl;
            return;
        }

        // Buffer for receiving data
        char buffer[this->CHUNK_SIZE];

        // Amount of bytes received
        std::size_t total_received = 0;

        while (true)
        {
            // Synchronous read
            std::size_t bytes_received = client_socket->read_some(boost::asio::buffer(buffer, this->CHUNK_SIZE));

            // If received bytes
            if (bytes_received > 0)
            {
                // Synchronous write to the received file
                received_file.write(buffer, bytes_received);
                received_file.flush(); // Flush the data to the file

                total_received += bytes_received;
                std::cout << "Received " << bytes_received << " bytes from "
                          << client_socket->remote_endpoint().address() << ":"
                          << client_socket->remote_endpoint().port()
                          << std::endl;
            }
            // The Client CLose the Stream
            else
            {
                // No more data or error on the Client Side
                break;
            }
        }

        // Check If All data has been received
        if (total_received > 0)
        {
            std::cout << "All data received successfully!" << std::endl;
        }
        else
        {
            std::cerr << "No data received or an error occurred." << std::endl;
        }

        // Close the Received File
        received_file.close();

        // Open Another File to put the final production
        decodeFileToFile(temp_file, file_to_store);

        // Remove the temporary file
        std::remove(temp_file.c_str());
    }

    //! PRIVATE METHODS SECTIONS
    //!============================================================================
    void Server::AcceptConnections()
    {
        // Creating a socket to represent a connected client
        // Use Shared_ptr to share the owner ship instead of copying them
        std::shared_ptr<boost::asio::ip::tcp::socket> client_socket = std::make_shared<boost::asio::ip::tcp::socket>(this->io_context);

        // Start an asynchronous accept operation
        this->acceptor_server->async_accept(
            *client_socket,
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
            // DEBUG: For Testing Send Data Section
            // this->SendText(client_socket, "Hello World\n");
            // this->SendTextBasedFile(client_socket, "hello.json");
            // this->SendTextBasedFile(client_socket, "Road 4.png");

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

        // DEBUG: For Testing Listenning Method Protocol
        // While the Server is Running
        // Still Listen
        // std::string received_text;
        std::string file_to_store = "decoded.png";
        while (this->is_running)
        {
            // this->GetText(client_socket, received_text);
            // this->GetTextBasedFile(client_socket, file_to_store);
            this->GetBinaryFile(client_socket, file_to_store);
        }
    }
} // namespace SN_Server

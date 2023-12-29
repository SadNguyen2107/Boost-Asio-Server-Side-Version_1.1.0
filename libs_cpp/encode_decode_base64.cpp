#include "../include/encode_decode_base64.h"
#include <fstream>
#include <iostream>

namespace JB_Encode_Decode_Base64
{
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    // this is for base64 encode/decoding.....
    bool is_base64(BYTE c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    const std::string createTempFile(const std::string &input_file_name)
    {
        // Find the position of the dot before the file extension
        std::size_t dot_position = input_file_name.find_last_of('.');

        // Extract The substring before the dot
        std::string prefix = input_file_name.substr(0, dot_position  - 1);

        // Create a new file name by appending '.txt'
        std::string new_file_name = prefix + "_temp.txt";

        return new_file_name; 
    }

    std::string base64_encode(BYTE const *buf, unsigned int bufLen)
    {
        std::string ret;
        int i = 0;
        int j = 0;
        BYTE char_array_3[3];
        BYTE char_array_4[4];

        while (bufLen--)
        {
            char_array_3[i++] = *(buf++);
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i < 4); i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];

            while ((i++ < 3))
                ret += '=';
        }

        return ret;
    }

    std::vector<BYTE> base64_decode(std::string const &encoded_string)
    {
        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        BYTE char_array_4[4], char_array_3[3];
        std::vector<BYTE> ret;

        while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
        {
            char_array_4[i++] = encoded_string[in_];
            in_++;
            if (i == 4)
            {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret.push_back(char_array_3[i]);
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;

            for (j = 0; j < 4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++)
                ret.push_back(char_array_3[j]);
        }

        return ret;
    }

    void encodeFileToFile(const std::string &inputFilename, const std::string &outputFilename)
    {
        std::ifstream inputFile(inputFilename, std::ios::binary);
        std::vector<BYTE> buffer(std::istreambuf_iterator<char>(inputFile), {});

        std::string encoded = base64_encode(buffer.data(), buffer.size());

        std::ofstream outputFile(outputFilename);
        outputFile << encoded;
        outputFile.close();
    }

    bool decodeFileToFile(const std::string &inputFilename, const std::string &outputFilename)
    {
        std::ifstream inputFile(inputFilename);
        if (!inputFile.is_open())
        {
            std::cerr << "Error: Unable to open file for receiving data " << inputFilename << std::endl;
            return false;
        }
        else
        {
            // DEBUG: Fishy
            std::cout << "Something Fishy Here!" << std::endl;
        }
        
        
        std::string encoded((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

        std::vector<BYTE> decoded = base64_decode(encoded);

        std::ofstream outputFile(outputFilename, std::ios::binary);
        if (!outputFile.is_open())
        {
            std::cerr << "Error: Unable to open file for receiving data " << outputFilename << std::endl;
            return false;
        }
        else
        {
            // DEBUG: Fishy
            std::cout << "Something Fishy Here!" << std::endl;
        }
        

        outputFile.write(reinterpret_cast<const char *>(decoded.data()), decoded.size());
        outputFile.flush();     // Flush the data into the file
        outputFile.close();

        return true;
    }
} // namespace JB_Encode_Decode_Base64
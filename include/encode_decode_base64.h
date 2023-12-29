#ifndef ENCODE_DECODE_BASE64
#define ENCODE_DECODE_BASE64

#include <string>
#include <vector>

namespace JB_Encode_Decode_Base64
{
    typedef unsigned char BYTE;

    extern const std::string base64_chars;

    bool is_base64(BYTE c);

    std::string base64_encode(BYTE const *buf, unsigned int bufLen);
    std::vector<BYTE> base64_decode(std::string const &encoded_string);

    const std::string createTempFile(const std::string &input_file_name); 

    void encodeFileToFile(const std::string &input_filename, const std::string &output_filename);
    bool decodeFileToFile(const std::string &input_filename, const std::string &output_filename);
} // namespace JB_Encode_Decode_Base64

#endif // ENCODE_DECODE_BASE64
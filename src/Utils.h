#include <stdint.h>
#include <arpa/inet.h>  // for htonl

int64_t htonll(int64_t value) {
    // Convert the int64_t to uint64_t
    uint64_t uvalue = static_cast<uint64_t>(value);
    
    // Check if we're on a little-endian system
    if (htonl(1) != 1) {
        // If we are, we need to swap the order of the bytes
        uint32_t high_part = htonl(static_cast<uint32_t>(uvalue >> 32));
        uint32_t low_part = htonl(static_cast<uint32_t>(uvalue & 0xFFFFFFFFLL));

        return static_cast<int64_t>((static_cast<uint64_t>(low_part) << 32) | high_part);
    } else {
        // If we're on a big-endian system, we don't need to do anything
        return value;
    }
}

// Function to convert a 64-bit integer from network byte order to host byte order
int64_t ntohll(int64_t value) {
    // Convert the int64_t to uint64_t
    uint64_t uvalue = static_cast<uint64_t>(value);
    
    // Check if we're on a little-endian system
    if (htonl(1) != 1) {
        // If we are, we need to swap the order of the bytes
        uint32_t high_part = ntohl(static_cast<uint32_t>(uvalue >> 32));
        uint32_t low_part = ntohl(static_cast<uint32_t>(uvalue & 0xFFFFFFFFLL));

        return static_cast<int64_t>((static_cast<uint64_t>(low_part) << 32) | high_part);
    } else {
        // If we're on a big-endian system, we don't need to do anything
        return value;
    }
}

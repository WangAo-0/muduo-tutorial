// #include <stdint.h>
#include <arpa/inet.h>  // for htonl

class Constants {
public:
    static const uint8_t SEVR_MSG = 2;
    static const uint8_t CONN_MSG = 1;
    static const uint8_t SEND_MSG = 0;
    static const uint32_t SERVER_ID = 0;
    // 其他常量...
};

// bool isLittleEndian() {
//     static const int num = 1;
//     return *reinterpret_cast<const char*>(&num) == 1;
// }

// int64_t htonll(int64_t value) {
//     if (isLittleEndian()) {
//         const uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
//         const uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));
//         return static_cast<int64_t>((static_cast<uint64_t>(low_part) << 32) | high_part);
//     } else {
//         return value;
//     }
// }

// int64_t ntohll(int64_t value) {
//     if (isLittleEndian()) {
//         const uint32_t high_part = ntohl(static_cast<uint32_t>(value >> 32));
//         const uint32_t low_part = ntohl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));
//         return static_cast<int64_t>((static_cast<uint64_t>(low_part) << 32) | high_part);
//     } else {
//         return value;
//     }
// }

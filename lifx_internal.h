#ifndef LIFX_INTERNAL_H_
#define LIFX_INTERNAL_H_

#include <stdint.h>

#ifdef LIFX_BIG_ENDIAN
#define LE16(i) (((((i) & 0xFF) << 8) | (((i) >> 8) & 0xFF)) & 0xFFFF)
#define LE(i)   (((i & 0xff000000) >> 24) | ((i & 0x00ff0000) >> 8) | ((i & 0x0000ff00) << 8) | (i << 24))
#else
#define LE16(i) (i)
#define LE(i)   (i)
#endif

#define LIFX_MAX_DEVICE_COUNT 16
#define LIFX_BROADCAST_IPV4 0xFFFFFFFF // 255.255.255.255
#define LIFX_BROADCAST_PORT 56700

typedef struct _lifx_version_t
{
    uint64_t build;
    uint16_t major;
    uint16_t minor;
} lifx_version_t;

typedef struct _lifx_section_t
{
    uint8_t uuid[16];
    uint64_t timestamp;
    char label[32];
    char terminator;
} lifx_section_t;

typedef struct _lifx_device_light_t
{
    double hue;
    double saturation;
    double brightness;
    short kelvin;
    uint16_t power;
} lifx_device_light_t;

typedef struct _lifx_device_t
{
    bool in_use;
    // device metadata
    uint8_t mac[6]; // MAC from packet address
    uint32_t ipv4; // IPv4 of device (in host order)
    uint16_t port; // port of service (in host order)
    uint8_t service; // service number (always 1, for UDP)
    uint32_t vendor; // vendor number of device from GetVersion
    uint32_t product; // product number of device from GetVersion
    lifx_version_t version; // firmware version from GetHostFirmware
    lifx_section_t group; // data from GetGroup
    lifx_section_t location; // data from GetLocation
    char label[32]; // device label
    char terminator; // always 0, terminates label
    // update information
    int latency; // milliseconds from discovery to detection
    uint64_t first_update; // unix timestamp, in milliseconds, of the first packet
    uint64_t last_send; // unix timestamp, in milliseconds, of the last sent packet
    uint64_t last_update; // unix timestamp, in milliseconds, of the last recieved packet
    // type-specific information
    bool is_light;
    lifx_device_light_t light;
} lifx_device_t;

#endif // LIFX_INTERNAL_H_

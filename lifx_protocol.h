/*
    liblifx - lifx_protocol.h
    Structure definitions for the LIFX network protocol and its packet types.
*/

#ifndef LIFX_PROTOCOL_H_
#define LIFX_PROTOCOL_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifndef static_assert // hack to silence compiler errors
#define static_assert(...)
#endif

#define PACKED __attribute__((packed))

typedef enum _lifx_packet_type_t
{
    // System packet types
    LIFX_PT_GETSERVICE = 2,
    LIFX_PT_STATESERVICE = 3,
    LIFX_PT_GETHOSTFIRMWARE = 14,
    LIFX_PT_STATEHOSTFIRMWARE = 15,
    LIFX_PT_GETWIFIINFO = 16,
    LIFX_PT_STATEWIFIINFO = 17,
    LIFX_PT_GETWIFIFIRMWARE = 18,
    LIFX_PT_STATEWIFIFIRMWARE = 19,
    LIFX_PT_GETPOWER = 20,
    LIFX_PT_SETPOWER = 21,
    LIFX_PT_STATEPOWER = 22,
    LIFX_PT_GETLABEL = 23,
    LIFX_PT_SETLABEL = 24,
    LIFX_PT_STATELABEL = 25,
    LIFX_PT_GETVERSION = 32,
    LIFX_PT_STATEVERSION = 33,
    LIFX_PT_GETINFO = 34,
    LIFX_PT_STATEINFO = 35,
    LIFX_PT_SETREBOOT = 38,
    LIFX_PT_GETLOCATION = 48,
    LIFX_PT_SETLOCATION = 49,
    LIFX_PT_STATELOCATION = 50,
    LIFX_PT_GETGROUP = 51,
    LIFX_PT_SETGROUP = 52,
    LIFT_PT_STATEGROUP = 53,
    LIFX_PT_ECHOREQUEST = 58,
    LIFX_PT_ECHORESPONSE = 59,
    // Light packet types
    LIFX_PT_GETCOLOR = 101,
    LIFX_PT_SETCOLOR = 102,
    LIFX_PT_SETWAVEFORM = 103,
    LIFX_PT_LIGHTSTATE = 107,
    LIFX_PT_GETLIGHTPOWER = 116,
    LIFX_PT_SETLIGHTPOWER = 117,
    LIFX_PT_STATELIGHTPOWER = 118,
    LIFX_PT_SETWAVEFORMOPTIONAL = 119,
    LIFX_PT_GETINFRARED = 120,
    LIFX_PT_STATEINFRARED = 121,
    LIFX_PT_SETINFRARED = 122,
    LIFX_PT_GETHEVCYCLE = 142,
    LIFX_PT_SETHEVCYCLE = 143,
    LIFX_PT_STATEHEVCYCLE = 144,
    LIFX_PT_SETHEVCYCLECONFIGURATION = 146,
    LIFX_PT_STATEHEVCYCLECONFIGURATION = 147,
    LIFX_PT_GETLASTHEVCYCLERESULT = 148,
    LIFX_PT_STATELASTHEVCYCLERESULT = 149,
} lifx_packet_type_t;

typedef struct _lifx_frame_header_t
{
    uint16_t size;
#ifndef LIFX_BIG_ENDIAN
    uint16_t protocol : 12;
    bool addressable : 1;
    bool tagged : 1;
    uint8_t origin : 2;
#else // big endian platforms are in a different order
    uint8_t origin : 2;
    bool tagged : 1;
    bool addressable : 1;
    uint16_t protocol : 12;
#endif
    uint32_t source;
} PACKED lifx_frame_header_t;
static_assert(sizeof(lifx_frame_header_t) == 8, "frame header size");

typedef struct _lifx_frame_address_t
{
    uint8_t mac[8];
    uint8_t lifxv2[6];
#ifndef LIFX_BIG_ENDIAN
    bool res_required : 1;
    bool ack_required : 1;
    uint8_t reserved : 6;
#else // big endian platforms are in a different order
    uint8_t reserved : 6;
    bool ack_required : 1;
    bool res_required : 1;
#endif
    uint8_t sequence;
} PACKED lifx_frame_address_t;
static_assert(sizeof(lifx_frame_address_t) == 16, "frame address size");

typedef struct _lifx_protocol_header_t
{
    uint8_t reserved_1[8];
    uint16_t type;
    uint8_t reserved_2[2];
} PACKED lifx_protocol_header_t;
static_assert(sizeof(lifx_protocol_header_t) == 12, "protocol header size");

typedef struct _lifx_header_t
{
    lifx_frame_header_t frame;
    lifx_frame_address_t address;
    lifx_protocol_header_t protocol;
} PACKED lifx_header_t;
static_assert(sizeof(lifx_header_t) == 36, "packet header size");

// -- START SYSTEM MESSAGES --

typedef struct _lifx_state_service_t
{
    uint8_t service;
    uint32_t port;
} PACKED lifx_state_service_t;

typedef struct _lifx_state_host_firmware_t
{
    uint64_t timestamp;
    uint8_t reserved[8];
    uint16_t version_minor;
    uint16_t version_major;
} PACKED lifx_state_host_firmware_t;

typedef struct _lifx_state_version_t
{
    uint32_t vendor;
    uint32_t product;
    uint8_t reserved[4];
} PACKED lifx_state_version_t;

typedef struct _lifx_state_label_t
{
    char label[32];
} PACKED lifx_state_label_t;

// -- END SYSTEM MESSAGES --

// -- BEGIN LIGHT-SPECIFIC MESSAGES --

typedef struct _lifx_light_state_t
{
    uint16_t hue;
    uint16_t saturation;
    uint16_t brightness;
    uint16_t kelvin;
    uint8_t reserved_1[2];
    uint16_t power;
    char label[32];
    uint8_t reserved_2[8];
} PACKED lifx_light_state_t;

typedef struct _lifx_state_light_power_t
{
    uint16_t level;
} PACKED lifx_state_light_power_t;

typedef struct _lifx_set_color_t
{
    uint8_t reserved;
    uint16_t hue;
    uint16_t saturation;
    uint16_t brightness;
    uint16_t kelvin;
    uint32_t time_ms;
} PACKED lifx_set_color_t;

typedef struct _lifx_set_light_power_t
{
    uint16_t power;
    uint32_t time_ms;
} PACKED lifx_set_light_power_t;

// -- END LIGHT-SPECIFIC MESSAGES --

#endif // LIFX_PROTOCOL_H_

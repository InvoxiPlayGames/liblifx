/*
    liblifx - lifx.h
    Publicly accessible header for the liblifx library.
*/

#ifndef LIFX_H_
#define LIFX_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef LIFX_INTERNAL_H_
typedef uint8_t lifx_device_t;
#endif

#define LIFX_MAX_PACKET_SIZE 0x80

typedef void (*lifx_send_packet_t)(uint8_t *packet, size_t length, uint32_t ipv4, uint16_t port);
typedef void (*lifx_device_update_t)(lifx_device_t *device, bool new);

// Initialises the library, provided a function to send packets and optionally a function to call when device state is updated.
void lifx_init(lifx_send_packet_t send_packet, lifx_device_update_t device_update);

// Function to be called when a new packet is recieved by the caller.
void lifx_handle_incoming_packet(uint8_t *packet, size_t length, uint32_t ipv4, uint16_t port);

// Gets the number of LIFX devices the library has seen.
int lifx_get_device_count();
// Gets a handle to a LIFX device from an index, starting from 0.
lifx_device_t *lifx_get_device_from_num(int num);
// Gets a handle to a LIFX device from a device's MAC address.
lifx_device_t *lifx_get_device(uint8_t mac[6]);

// Broadcasts a device discovery packet.
void lifx_discover_devices();
// Fires a device discovery packet towards a given IP (in host order).
void lifx_discover_device(uint32_t ipv4);

// Gets the latency from the computer to the device (at the time of discovery.)
int lifx_get_device_latency(lifx_device_t *device);
// Gets the product type of a device.
int lifx_get_device_product(lifx_device_t *device);
// Gets the MAC address of a device.
uint8_t *lifx_get_device_mac(lifx_device_t *device);
// Gets the IPv4 address of a device (in host order).
uint32_t lifx_get_device_ipv4(lifx_device_t *device);
// Gets the current label of a device.
char *lifx_get_device_label(lifx_device_t *device);
// Gets the major firmware version of a device.
int lifx_get_device_firmware_major(lifx_device_t *device);
// Gets the minor firmware revision of a device.
int lifx_get_device_firmware_minor(lifx_device_t *device);

// Gets the current light colour from a light device.
int lifx_get_light_color(lifx_device_t *device, double *hue, double *saturation, double *brightness, short *kelvin);
// Gets whether a light device is powered on or not.
bool lifx_is_light_powered(lifx_device_t *device);
// Sets the colour of a light device, over a period of time ms.
void lifx_set_light_color(lifx_device_t *device, double hue, double saturation, double brightness, short kelvin, uint32_t time);
// Powers a light device on or off, over a period of time ms.
void lifx_set_light_powered(lifx_device_t *device, bool powered, uint32_t time);

// Gets the name of a product type given its ID.
char *lifx_get_product_name(int product_id);
// Gets whether a given product ID is a light.
bool lifx_product_is_light(int product_id);

#endif // LIFX_H_

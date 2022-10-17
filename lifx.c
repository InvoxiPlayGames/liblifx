/*
    liblifx - lifx.c
    The main liblifx library code.
*/

#include <string.h>
#include <sys/time.h>

#include "lifx_products.h"
#include "lifx_protocol.h"
#include "lifx_internal.h"
#include <lifx.h>

static lifx_device_t devices[LIFX_MAX_DEVICE_COUNT];
static int devices_count = 0;
static int source_value = 0;
static uint64_t last_discover_timestamp = 0;

static lifx_send_packet_t lifx_send_outgoing_packet = NULL;
static lifx_device_update_t lifx_device_update = NULL;

// -- START CORE LIBRARY FUNCTIONS --

static uint64_t lifx_get_time_ms()
{
    struct timeval te; 
    gettimeofday(&te, NULL);
    return (te.tv_sec * 1000LL + te.tv_usec / 1000);
}

static lifx_device_t *lifx_get_device_internal(uint8_t mac[6], bool create)
{
    for (int i = 0; i < devices_count; i++) {
        lifx_device_t *device = &devices[i];
        if (device->in_use && memcmp(mac, device->mac, 6) == 0)
            return device;
    }
    if (devices_count < LIFX_MAX_DEVICE_COUNT && create) {
        lifx_device_t *device = &devices[devices_count];
        memset(device, 0, sizeof(lifx_device_t));
        memcpy(device->mac, mac, 6);
        device->in_use = true;
        devices_count++;
        return device;
    }
    return NULL;
}

lifx_device_t *lifx_get_device(uint8_t mac[6])
{
    return lifx_get_device_internal(mac, false);
}

lifx_device_t *lifx_get_device_from_num(int num)
{
    lifx_device_t *device = &devices[num];
    if (num >= LIFX_MAX_DEVICE_COUNT)
        return NULL;
    if (device->in_use == false)
        return NULL;
    return &devices[num];
}

lifx_device_t *lifx_get_devices()
{
    return devices;
}

int lifx_get_device_count()
{
    return devices_count;
}

void lifx_init(lifx_send_packet_t send_packet, lifx_device_update_t device_update)
{
    // set the outgoing packet function
    lifx_send_outgoing_packet = send_packet;
    // clear the devices array
    memset(devices, 0, sizeof(devices));
    devices_count = 0;
    // set the device update function, if it's been set
    if (device_update != NULL)
        lifx_device_update = device_update;
    // set our source value to something random
    srand(time(NULL));
    source_value = rand();
}

void lifx_send_packet(lifx_device_t *target_device, uint16_t packet_type, void *extra_data, size_t extra_size)
{
    uint8_t packet_data[LIFX_MAX_PACKET_SIZE];
    lifx_header_t *lifx_packet = (lifx_header_t *)packet_data;

    memset(lifx_packet, 0, sizeof(lifx_header_t));
    lifx_packet->frame.size = sizeof(lifx_header_t) + extra_size;
    lifx_packet->frame.protocol = 1024;
    lifx_packet->frame.addressable = true;
    lifx_packet->frame.source = source_value;
    lifx_packet->address.res_required = true;
    lifx_packet->protocol.type = packet_type;

    if (extra_data != NULL && extra_size > 0 && extra_size < LIFX_MAX_PACKET_SIZE - sizeof(lifx_header_t)) {
        memcpy(packet_data + sizeof(lifx_header_t), extra_data, extra_size);
    }

    if (target_device != NULL) {
        memcpy(lifx_packet->address.mac, target_device->mac, 6);
        target_device->last_send = lifx_get_time_ms();
        lifx_send_outgoing_packet(packet_data, lifx_packet->frame.size, target_device->ipv4, target_device->port);
    } else {
        lifx_packet->frame.tagged = true;
        lifx_send_outgoing_packet(packet_data, lifx_packet->frame.size, LIFX_BROADCAST_IPV4, LIFX_BROADCAST_PORT);
    }
}

void lifx_discover_devices()
{
    last_discover_timestamp = lifx_get_time_ms();
    lifx_send_packet(NULL, LIFX_PT_GETSERVICE, NULL, 0);
}

void lifx_poll_system(lifx_device_t *device)
{
    lifx_send_packet(device, LIFX_PT_GETVERSION, NULL, 0);
    lifx_send_packet(device, LIFX_PT_GETHOSTFIRMWARE, NULL, 0);
    // uncomment when section code is done
    // lifx_send_packet(device, LIFX_PT_GETLOCATION, NULL, 0);
    // lifx_send_packet(device, LIFX_PT_GETGROUP, NULL, 0);
}

void lifx_poll_light(lifx_device_t *device)
{
    lifx_send_packet(device, LIFX_PT_GETCOLOR, NULL, 0);
}

void lifx_handle_incoming_packet(uint8_t *packet, size_t length, uint32_t ipv4, uint16_t port)
{
    uint64_t time_now = lifx_get_time_ms();
    lifx_header_t *header = (lifx_header_t *)packet;
    // sanity check - the size must match that of the one in the header
    if (length < sizeof(lifx_header_t) || length != header->frame.size)
        return;
    // service definitions should be treated as new devices
    if (header->protocol.type == LIFX_PT_STATESERVICE) {
        // sanity check the packet size
        if ((header->frame.size - sizeof(lifx_header_t)) != sizeof(lifx_state_service_t))
            return;
        lifx_state_service_t *service = (lifx_state_service_t *)(packet + sizeof(lifx_header_t));
        // only accept the UDP service for now
        if (service->service != 1)
            return;
        // create the device object or update if we have one already
        lifx_device_t *device = lifx_get_device_internal(header->address.mac, true);
        if (device == NULL)
            return;
        device->ipv4 = ipv4;
        device->port = service->port;
        device->service = service->service;
        device->first_update = time_now;
        device->last_update = time_now;
        device->latency = time_now - last_discover_timestamp;
        // poll for all the extra info
        lifx_poll_system(device);
        return;
    }
    // check if the source value matches
    if (header->frame.source != source_value)
        return;
    // get the handle to the device that's talking to us
    lifx_device_t *device = lifx_get_device_internal(header->address.mac, false);
    if (device == NULL)
        return;
    // update the last updated packet
    device->last_update = time_now;
    // make sure this information is up to date - it might've changed?
    device->ipv4 = ipv4;
    device->port = port;
    // switch case for packet type
    switch(header->protocol.type) {
        case LIFX_PT_STATEHOSTFIRMWARE:
            // sanity check the packet size
            if ((header->frame.size - sizeof(lifx_header_t)) != sizeof(lifx_state_host_firmware_t))
                return;
            lifx_state_host_firmware_t *fw = (lifx_state_host_firmware_t *)(packet + sizeof(lifx_header_t));
            device->version.build = fw->timestamp;
            device->version.major = fw->version_major;
            device->version.minor = fw->version_minor;
            return;
        case LIFX_PT_STATEVERSION:
            // sanity check the packet size
            if ((header->frame.size - sizeof(lifx_header_t)) != sizeof(lifx_state_version_t))
                return;
            lifx_state_version_t *ver = (lifx_state_version_t *)(packet + sizeof(lifx_header_t));
            device->vendor = ver->vendor;
            device->product = ver->product;
            device->is_light = lifx_product_is_light(device->product);
            if (device->is_light)
                lifx_poll_light(device);
            else // the light state packet includes the label, for non-lights ask politely
                lifx_send_packet(device, LIFX_PT_GETLABEL, NULL, 0);
            return;
        case LIFX_PT_STATELABEL:
            // sanity check the packet size
            if ((header->frame.size - sizeof(lifx_header_t)) != sizeof(lifx_state_label_t))
                return;
            lifx_state_label_t *label = (lifx_state_label_t *)(packet + sizeof(lifx_header_t));
            memcpy(device->label, label->label, 32);
            return;
        case LIFX_PT_LIGHTSTATE:
            // sanity check the packet size
            if ((header->frame.size - sizeof(lifx_header_t)) != sizeof(lifx_light_state_t))
                return;
            lifx_light_state_t *light = (lifx_light_state_t *)(packet + sizeof(lifx_header_t));
            device->light.kelvin = light->kelvin;
            device->light.power = light->power;
            device->light.brightness = (double)light->brightness / 0xFFFF;
            device->light.saturation = (double)light->saturation / 0xFFFF;
            device->light.hue = (((double)light->hue) * 360) / 0x10000;
            memcpy(device->label, light->label, 32);
            return;
        case LIFX_PT_STATELIGHTPOWER:
            // sanity check the packet size
            if ((header->frame.size - sizeof(lifx_header_t)) != sizeof(lifx_state_light_power_t))
                return;
            lifx_state_light_power_t *power = (lifx_state_light_power_t *)(packet + sizeof(lifx_header_t));
            device->light.power = power->level;
            return;
    }
}

// -- END CORE LIBRARY FUNCTIONS --

// -- START GENERIC DEVICE INFO --

int lifx_get_device_latency(lifx_device_t *device)
{
    if (device == NULL || !device->in_use)
        return -1;
    return device->latency;
}

char *lifx_get_device_label(lifx_device_t *device)
{
    if (device == NULL || !device->in_use)
        return NULL;
    return device->label;
}

uint8_t *lifx_get_device_mac(lifx_device_t *device)
{
    if (device == NULL || !device->in_use)
        return NULL;
    return device->mac;
}

uint32_t lifx_get_device_ipv4(lifx_device_t *device)
{
    if (device == NULL || !device->in_use)
        return 0xFFFFFFFF; // not really valid, but no real device would have it...
    return device->ipv4;
}

int lifx_get_device_product(lifx_device_t *device)
{
    if (device == NULL || !device->in_use)
        return -1;
    return device->product;
}

int lifx_get_device_firmware_major(lifx_device_t *device)
{
    if (device == NULL || !device->in_use)
        return -1;
    return device->version.major;
}

int lifx_get_device_firmware_minor(lifx_device_t *device)
{
    if (device == NULL || !device->in_use)
        return -1;
    return device->version.minor;
}

// -- END GENERIC DEVICE INFO --

// -- START LIGHT DEVICE FUNCTIONS --

int lifx_get_light_color(lifx_device_t *device, double *hue, double *saturation, double *brightness, short *kelvin)
{
    if (device == NULL || !device->in_use || !device->is_light)
        return -1;
    if (hue != NULL)
        *hue = device->light.hue;
    if (saturation != NULL)
        *saturation = device->light.saturation;
    if (brightness != NULL)
        *brightness = device->light.brightness;
    if (kelvin != NULL)
        *kelvin = device->light.kelvin;
    return 0;
}

void lifx_set_light_color(lifx_device_t *device, double hue, double saturation, double brightness, short kelvin, uint32_t time)
{
    lifx_set_color_t set_color;
    if (device == NULL || !device->in_use || !device->is_light)
        return;
    set_color.hue = (int)((0x10000 * hue) / 360) % 0x10000;
    set_color.saturation = saturation * 0xFFFF;
    set_color.brightness = brightness * 0xFFFF;
    set_color.kelvin = kelvin;
    set_color.time_ms = time;
    lifx_send_packet(device, LIFX_PT_SETCOLOR, &set_color, sizeof(set_color));
    return;
}

int lifx_get_light_power(lifx_device_t *device, uint16_t *power)
{
    if (device == NULL || !device->in_use || !device->is_light)
        return -1;
    if (power != NULL)
        *power = device->light.power;
    return 0;
}

bool lifx_is_light_powered(lifx_device_t *device)
{
    if (device == NULL || !device->in_use || !device->is_light || device->light.power != 0xFFFF)
        return false;
    return true;
}

void lifx_set_light_powered(lifx_device_t *device, bool powered, uint32_t time)
{
    lifx_set_light_power_t set_power;
    if (device == NULL || !device->in_use || !device->is_light)
        return;
    set_power.power = powered ? 0xFFFF : 0;
    set_power.time_ms = time;
    lifx_send_packet(device, LIFX_PT_SETLIGHTPOWER, &set_power, sizeof(set_power));
    return;
}

// -- END LIGHT DEVICE FUNCTIONS --

// -- START PRODUCT DETAILS --

char *lifx_get_product_name(int product_id)
{
    for (int i = 0; i < lifx_products_count; i++) {
        lifx_product_info_t product = lifx_products[i];
        if (product.id == product_id)
            return product.product_name;
    }
    return "Unknown Product";
}

bool lifx_product_is_light(int product_id)
{
    for (int i = 0; i < lifx_products_count; i++) {
        lifx_product_info_t product = lifx_products[i];
        if (product.id == product_id)
            return !product.relays; // TODO: do we have a better way of knowing this?
    }
    return false;
}

// -- END PRODUCT DETAILS --

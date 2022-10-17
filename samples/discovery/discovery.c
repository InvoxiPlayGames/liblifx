#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <lifx.h>

static int socketfd = 0;

static int create_socket()
{
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    int one = 1;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
    fcntl(s, F_SETFL, O_NONBLOCK);
    return s;
}

static void send_packet(uint8_t *data, size_t size, uint32_t ipv4, uint16_t port)
{
    struct sockaddr_in outgoing;
    outgoing.sin_family = AF_INET;
    outgoing.sin_addr.s_addr = htonl(ipv4);
    outgoing.sin_port = htons(port);
    sendto(socketfd, data, size, 0, (struct sockaddr *)&outgoing, sizeof(outgoing));
}

static void print_device(lifx_device_t *device)
{
    int product_id = lifx_get_device_product(device);
    uint32_t ipn = htonl(lifx_get_device_ipv4(device));
    uint8_t *ip = (uint8_t *)&ipn;
    uint8_t *mac = lifx_get_device_mac(device);
    double hue;
    double saturation;
    double brightness;
    short kelvin;
    printf("%s (%i.%i.%i.%i):\n", lifx_get_product_name(product_id), ip[0], ip[1], ip[2], ip[3]);
    printf("    MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("    Firmware: %i.%i\n", lifx_get_device_firmware_major(device), lifx_get_device_firmware_minor(device));
    printf("    Label: %s\n", lifx_get_device_label(device));
    printf("    Latency: %ims\n", lifx_get_device_latency(device));
    if (lifx_product_is_light(product_id))
    {
        lifx_get_light_color(device, &hue, &saturation, &brightness, &kelvin);
        printf("    HSVK: %.0f° %.0f%% %.0f%% %ik\n", hue, saturation * 100, brightness * 100, kelvin);
        printf("    Power: %s\n", lifx_is_light_powered(device) ? "On" : "Off");
    }
}

int main(int argv, char **argc)
{
    // create our socket
    socketfd = create_socket();

    // initialise the lifx library
    lifx_init(send_packet, NULL);

    // send out a device discovery packet
    lifx_discover_devices();
    
    // recieve incoming packets for 1 second
    int time_passed = 0;
    uint8_t packet_buffer[LIFX_MAX_PACKET_SIZE];
    while (time_passed < 1000)
    {
        struct sockaddr_in device;
        unsigned int devlen = sizeof(device);
        int r = recvfrom(socketfd, packet_buffer, LIFX_MAX_PACKET_SIZE, 0, (struct sockaddr *)&device, &devlen);
        if (r > 0) {
            uint32_t ipv4_host = ntohl(device.sin_addr.s_addr);
            uint16_t port_host = ntohs(device.sin_port);
            // hand off our incoming packet to the lifx library
            lifx_handle_incoming_packet(packet_buffer, r, ipv4_host, port_host);
        }
        usleep(1000);
        time_passed++;
    }

    // print the device information we got back
    int devices_found = lifx_get_device_count();
    printf("Found %i devices!\n", devices_found);
    for (int i = 0; i < devices_found; i++)
    {
        lifx_device_t *device = lifx_get_device_from_num(i);
        printf("\nDevice %i - ", i + 1);
        print_device(device);
    }
    return 0;
}

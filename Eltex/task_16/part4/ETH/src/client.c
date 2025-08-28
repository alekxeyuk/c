#define _GNU_SOURCE

#include "client.h"

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <locale.h>
#include <malloc.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t should_exit = 0;
static int sockfd;

static void signal_handler(int signum) {
  (void)signum;
  should_exit = 1;
  close(sockfd);
  printf("\nStoping...\n");
}

static void print_data(const char *data, size_t size) {
  printf("Data (%zu bytes):\n", size);
  for (size_t i = 0; i < size; i++) {
    if (i != 0 && i % 16 == 0) printf("\n");
    printf("%02X ", (unsigned char)data[i]);
  }
  printf("\nData as string:\n");
  for (size_t i = 0; i < size; i++) {
    if (data[i] >= 32 && data[i] <= 126)
      putchar(data[i]);
    else
      putchar('.');
  }
  printf("\n");
}

static void print_mac(const unsigned char *mac) {
  for (int i = 0; i < 6; i++) {
    if (i != 0) printf(":");
    printf("%02x", mac[i]);
  }
}

static uint16_t ipv4_checksum(const void *buf, size_t len) {
  const uint16_t *data = buf;
  uint32_t sum = 0;

  while (len > 1) {
    sum += *data++;
    len -= 2;
  }
  if (len == 1) {
    sum += *(const uint8_t *)data << 8;
  }
  while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);

  return (uint16_t)~sum;
}

static char *construct_udp_packet(const char *payload, size_t *udp_len, uint16_t src_port, uint16_t dest_port) {
  size_t payload_len = strlen(payload);
  *udp_len = sizeof(struct udphdr) + payload_len;

  char *packet = (char *)malloc(*udp_len);
  if (packet == NULL) error_exit("malloc");
  struct udphdr *udp_header = (struct udphdr *)packet;
  char *data = packet + sizeof(struct udphdr);

  udp_header->source = htons(src_port);
  udp_header->dest = htons(dest_port);
  udp_header->len = htons((uint16_t)*udp_len);
  udp_header->check = 0;

  memcpy(data, payload, payload_len);

  return packet;
}

static char *construct_ip_packet(const char *udp_packet, size_t udp_len, size_t *ip_len, const char *dest_ip,
                                 in_addr_t src_ip) {
  *ip_len = sizeof(struct iphdr) + udp_len;
  char *packet = (char *)malloc(*ip_len);
  if (packet == NULL) error_exit("malloc");
  struct iphdr *ip_header = (struct iphdr *)packet;

  ip_header->ihl = 5;
  ip_header->version = 4;
  ip_header->tos = 0;
  ip_header->tot_len = htons((uint16_t)*ip_len);
  ip_header->id = htons((uint16_t)time(NULL));
  ip_header->frag_off = 0;
  ip_header->ttl = 64;
  ip_header->protocol = IPPROTO_UDP;
  ip_header->check = 0;
  ip_header->saddr = src_ip;
  ip_header->daddr = inet_addr(dest_ip);

  ip_header->check = ipv4_checksum(ip_header, sizeof(struct iphdr));

  memcpy(packet + sizeof(struct iphdr), udp_packet, udp_len);
  return packet;
}

static char *construct_ethernet_frame(const char *ip_packet, size_t ip_len, size_t *frame_len,
                                      const unsigned char *shost, const unsigned char *dhost) {
  *frame_len = sizeof(struct ether_header) + ip_len;
  char *frame = (char *)malloc(*frame_len);
  if (frame == NULL) error_exit("malloc");
  struct ether_header *eth_header = (struct ether_header *)frame;

  memset(eth_header, 0, sizeof(struct ether_header));
  memcpy((unsigned char *)eth_header->ether_shost, shost, 6);
  memcpy((unsigned char *)eth_header->ether_dhost, dhost, 6);
  eth_header->ether_type = htons(ETHERTYPE_IP);

  memcpy(frame + sizeof(struct ether_header), ip_packet, ip_len);
  return frame;
}

static void send_frame(const char *packet, size_t p_len, int fd, source_info *src_info, unsigned char *dhost) {
  struct sockaddr_ll desst_addr;
  memset(&desst_addr, 0, sizeof(desst_addr));
  desst_addr.sll_family = AF_PACKET;
  desst_addr.sll_protocol = htons(ETH_P_IP);
  desst_addr.sll_ifindex = src_info->if_index;
  desst_addr.sll_halen = ETH_ALEN;
  memcpy(desst_addr.sll_addr, dhost, 6);

  if (sendto(fd, packet, p_len, 0, (struct sockaddr *)&desst_addr, sizeof(desst_addr)) < 0) error_exit("sendto");
  printf("Frame sent\n");
}

static void interface_data(long unsigned int cmd, struct ifreq *ifr, int fd, const char *if_name) {
  memset(ifr, 0, sizeof(*ifr));
  snprintf(ifr->ifr_name, sizeof(ifr->ifr_name), "%s", if_name);
  if (ioctl(fd, cmd, ifr) < 0) error_exit("ioctl");
}

static void bind_to_interface(int fd, int if_index) {
  struct sockaddr_ll sll;
  memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_protocol = htons(ETH_P_ALL);
  sll.sll_ifindex = if_index;
  if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) error_exit("bind");
}

static void fill_source_info(const char *if_name, source_info *src_info) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) error_exit("socket");

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));

  interface_data(SIOCGIFADDR, &ifr, sockfd, if_name);
  struct sockaddr_in *ip_addr = (struct sockaddr_in *)&ifr.ifr_addr;
  src_info->ip = ip_addr->sin_addr.s_addr;
  src_info->if_index = ifr.ifr_ifindex;

  interface_data(SIOCGIFHWADDR, &ifr, sockfd, if_name);
  memcpy(src_info->mac, ifr.ifr_hwaddr.sa_data, 6);

  close(fd);
}

static int get_mac_via_ioctl(const char *if_name, const char *ip_addr, unsigned char *mac_addr) {
  struct arpreq arp_req;
  struct sockaddr_in *sin;

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    perror("socket failed");
    return -1;
  }

  memset(&arp_req, 0, sizeof(arp_req));
  sin = (struct sockaddr_in *)&arp_req.arp_pa;
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = inet_addr(ip_addr);
  strncpy(arp_req.arp_dev, if_name, IFNAMSIZ - 1);

  if (ioctl(fd, SIOCGARP, &arp_req) < 0) {
    perror("ARP ioctl failed");
    close(fd);
    return -1;
  }

  if (arp_req.arp_flags & ATF_COM) {
    memcpy(mac_addr, arp_req.arp_ha.sa_data, 6);
    close(fd);
    return 0;
  }

  close(fd);
  return -1;
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "C.utf8");

  char buf[BUFFER_SIZE];
  ssize_t rec_size;
  uint16_t dest_port, from_port;
  char *dest_ip;
  const char *if_name = "eth0";
  source_info src_info;
  unsigned char dest_mac[6] = {0};

  if (argc < 4) {
    fprintf(stderr, "Usage: %s <destination_ip> <destination_port> <from_port> <interface>\n", argv[0]);
    error_exit("Invalid arguments");
  }
  if (argc == 5) if_name = argv[4];
  dest_ip = argv[1];
  dest_port = (uint16_t)atoi(argv[2]);
  from_port = (uint16_t)atoi(argv[3]);
  if (dest_port == 0 || from_port == 0) error_exit("Invalid port number");

  if (get_mac_via_ioctl(if_name, dest_ip, dest_mac) == 0) {
    printf("Destination MAC: [");
    print_mac(dest_mac);
    printf("]\n");
  } else {
    printf("Could not resolve MAC address for IP %s via ARP\n", dest_ip);
    error_exit("MAC resolution failed");
  }

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) error_exit("socket_connect");

  fill_source_info(if_name, &src_info);
  printf("Our IP: [%s]\n", inet_ntoa(*(struct in_addr *)&src_info.ip));
  printf("Our MAC: [");
  print_mac(src_info.mac);
  printf("]\n");

  bind_to_interface(sockfd, src_info.if_index);
  printf("Listening on interface [%s]\n", if_name);

  size_t udp_len;
  char *udp_packet = construct_udp_packet(MESSAGE, &udp_len, from_port, dest_port);
  size_t ip_len;
  char *ip_packet = construct_ip_packet(udp_packet, udp_len, &ip_len, dest_ip, src_info.ip);
  size_t frame_len;
  char *frame = construct_ethernet_frame(ip_packet, ip_len, &frame_len, src_info.mac, dest_mac);

  while (!should_exit) {
    sleep(1);
    send_frame(frame, frame_len, sockfd, &src_info, dest_mac);
    for (;;) {
      rec_size = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
      if (rec_size < 0) {
        if (should_exit) break;
        perror("recv");
        break;
      }

      size_t eth_header_len = sizeof(struct ethhdr);
      char *eth_payload = buf + eth_header_len;
      size_t eth_payload_len = (size_t)rec_size - eth_header_len;

      struct iphdr *ip_header = (struct iphdr *)eth_payload;
      if (ip_header->protocol != IPPROTO_UDP) continue;

      size_t ip_header_len = ip_header->ihl * 4;
      char *transport_payload = eth_payload + ip_header_len;
      size_t transport_len = eth_payload_len - ip_header_len;

      struct udphdr *udp_header = (struct udphdr *)transport_payload;
      if (ntohs(udp_header->dest) != from_port) continue;
      size_t udp_header_len = sizeof(struct udphdr);
      char *udp_data = transport_payload + udp_header_len;
      size_t data_len = transport_len - udp_header_len;

      printf("------------------------------------------------------\n");
      printf("New packet from [%s:%d]\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr), ntohs(udp_header->source));
      print_data(udp_data, data_len);
      printf("------------------------------------------------------\n");
      break;
    }
  }

  close(sockfd);
  free(udp_packet);
  free(ip_packet);
  free(frame);
  exit(EXIT_SUCCESS);
}

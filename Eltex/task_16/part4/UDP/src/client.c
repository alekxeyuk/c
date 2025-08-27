#include "client.h"

#include <arpa/inet.h>
#include <locale.h>
#include <malloc.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static char *constuct_udp_packet(const char *payload, size_t *udp_len, uint16_t src_port, uint16_t dest_port) {
  size_t payload_len = strlen(payload);
  *udp_len = sizeof(struct udphdr) + payload_len;

  char *packet = (char *)malloc(*udp_len);
  struct udphdr *udp_header = (struct udphdr *)packet;
  char *data = packet + sizeof(struct udphdr);

  udp_header->source = htons(src_port);
  udp_header->dest = htons(dest_port);
  udp_header->len = htons((uint16_t)*udp_len);
  udp_header->check = 0;

  memcpy(data, payload, payload_len);

  return packet;
}

static void send_udp_packet(const char *packet, size_t udp_len, int fd, const char *dest_ip, uint16_t dest_port) {
  struct sockaddr_in desst_addr;
  memset(&desst_addr, 0, sizeof(desst_addr));
  desst_addr.sin_family = AF_INET;
  desst_addr.sin_port = htons(dest_port);
  desst_addr.sin_addr.s_addr = inet_addr(dest_ip);

  if (sendto(fd, packet, udp_len, 0, (struct sockaddr *)&desst_addr, sizeof(desst_addr)) < 0) error_exit("sendto");
  printf("Packet sent to [%s:%d]\n", dest_ip, dest_port);
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "C.utf8");

  struct sockaddr_in s_addr;
  int s_addr_len = sizeof(s_addr);
  char buf[BUFFER_SIZE];
  ssize_t rec_size;
  uint16_t dest_port, from_port;
  char *dest_ip;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <destination_ip> <destination_port> <from_port>\n", argv[0]);
    error_exit("Invalid arguments");
  }
  dest_ip = argv[1];
  dest_port = (uint16_t)atoi(argv[2]);
  from_port = (uint16_t)atoi(argv[3]);
  if (dest_port == 0 || from_port == 0) error_exit("Invalid port number");

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) error_exit("socket_connect");

  size_t udp_len;
  char *packet = constuct_udp_packet(MESSAGE, &udp_len, from_port, dest_port);

  while (!should_exit) {
    sleep(1);
    send_udp_packet(packet, udp_len, sockfd, dest_ip, dest_port);
    for (;;) {
      rec_size = recvfrom(sockfd, buf, BUFFER_SIZE - 1, 0, (struct sockaddr *)&s_addr, (socklen_t *)&s_addr_len);
      if (rec_size < 0) {
        if (should_exit) break;
        perror("recvfrom");
        break;
      }

      struct iphdr *ip_header = (struct iphdr *)buf;
      if (ip_header->protocol != IPPROTO_UDP) continue;

      size_t ip_header_len = ip_header->ihl * 4;
      struct udphdr *udp_header = (struct udphdr *)(buf + ip_header_len);
      if (ntohs(udp_header->dest) != from_port) continue;
      size_t udp_header_len = sizeof(struct udphdr);
      size_t data_offset = ip_header_len + udp_header_len;
      size_t data_len = (size_t)rec_size - data_offset;

      printf("------------------------------------------------------\n");
      printf("New packet from [%s:%d]\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr), ntohs(udp_header->source));
      print_data(buf + data_offset, data_len);
      printf("------------------------------------------------------\n");
      break;
    }
  }

  close(sockfd);
  free(packet);
  exit(EXIT_SUCCESS);
}

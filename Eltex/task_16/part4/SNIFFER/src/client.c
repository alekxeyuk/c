#include "client.h"

#include <arpa/inet.h>
#include <locale.h>
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

static void print_ip_header(const struct iphdr *ip_header) {
  struct in_addr src_addr = {ip_header->saddr};
  struct in_addr dst_addr = {ip_header->daddr};

  printf("IP Header:\n");
  printf("\tVersion\t\t: %d\n", ip_header->version);
  printf("\tHeader Length\t: %d DWORDS\n", ip_header->ihl);
  printf("\tType Of Service\t: %d\n", ip_header->tos);
  printf("\tTotal Length\t: %d Bytes\n", ntohs(ip_header->tot_len));
  printf("\tIdentification\t: %d\n", ntohs(ip_header->id));
  printf("\tTTL\t\t: %d\n", ip_header->ttl);
  printf("\tProtocol\t: %d\n", ip_header->protocol);
  printf("\tChecksum\t: %d\n", ntohs(ip_header->check));
  printf("\tSource IP\t: %s\n", inet_ntoa(src_addr));
  printf("\tDestination IP\t: %s\n", inet_ntoa(dst_addr));
}

static void print_udp_header(const struct udphdr *udp_header) {
  printf("UDP Header:\n");
  printf("\tSource Port\t: %d\n", ntohs(udp_header->source));
  printf("\tDestination Port: %d\n", ntohs(udp_header->dest));
  printf("\tLength\t\t: %d\n", ntohs(udp_header->len));
  printf("\tChecksum\t: %d\n", ntohs(udp_header->check));
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

int main(void) {
  setlocale(LC_ALL, "C.utf8");

  struct sockaddr_in s_addr;
  int s_addr_len = sizeof(s_addr);
  char buf[BUFFER_SIZE];
  ssize_t rec_size;

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) error_exit("socket_connect");

  while (!should_exit) {
    rec_size = recvfrom(sockfd, buf, BUFFER_SIZE - 1, 0, (struct sockaddr *)&s_addr, (socklen_t *)&s_addr_len);
    if (rec_size < 0) {
      if (should_exit) break;
      error_exit("recvfrom");
    }

    struct iphdr *ip_header = (struct iphdr *)buf;
    if (ip_header->protocol != IPPROTO_UDP) continue;

    size_t ip_header_len = ip_header->ihl * 4;
    struct udphdr *udp_header = (struct udphdr *)(buf + ip_header_len);
    size_t udp_header_len = sizeof(struct udphdr);
    size_t data_offset = ip_header_len + udp_header_len;
    size_t data_len = (size_t)rec_size - data_offset;

    printf("------------------------------------------------------\n");
    printf("New packet from [%s:%d]\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr), ntohs(udp_header->source));
    print_ip_header(ip_header);
    print_udp_header(udp_header);
    print_data(buf + data_offset, data_len);
    printf("------------------------------------------------------\n");
  }

  close(sockfd);
  exit(EXIT_SUCCESS);
}

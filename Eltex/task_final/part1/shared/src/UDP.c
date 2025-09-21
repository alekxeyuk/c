#define _GNU_SOURCE
#include <stdio.h>

#include "client_lib.h"

static char *constuct_udp_packet(const char *payload, size_t *udp_len, uint16_t src_port, uint16_t dest_port) {
  size_t payload_len = strlen(payload);
  *udp_len = sizeof(struct udphdr) + payload_len;

  char *packet = (char *)malloc(*udp_len);
  if (packet == NULL) return NULL;
  struct udphdr *udp_header = (struct udphdr *)packet;
  char *data = packet + sizeof(struct udphdr);

  udp_header->source = htons(src_port);
  udp_header->dest = htons(dest_port);
  udp_header->len = htons((uint16_t)*udp_len);
  udp_header->check = 0;

  memcpy(data, payload, payload_len);

  return packet;
}

static ClientSession *client_init(uint16_t src_port, const char *if_name) {
  printf("Initializing UDP client on interface %s with source port %u\n", if_name, src_port);
  ClientSession *session = (ClientSession *)malloc(sizeof(ClientSession));
  if (!session) {
    return NULL;
  }
  if ((session->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
    free(session);
    perror("socket");
    return NULL;
  }
  session->is_connected = 0;
  session->src_port = src_port;
  session->dest_port = 0;
  session->if_name = if_name;
  session->correct = true;
  memset(&session->server_addr, 0, sizeof(session->server_addr));
  return session;
}

static int client_connect(ClientSession *session, const char *server_addr, uint16_t port) {
  if (session->is_connected) {
    return CLIENT_ERROR;
  }

  session->server_addr.sin_family = AF_INET;
  session->server_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, server_addr, &session->server_addr.sin_addr) <= 0) {
    return CLIENT_ERROR;
  }
  session->dest_port = port;

  session->is_connected = 1;
  return CLIENT_SUCCESS;
}

static int client_send_message(ClientSession *session, const char *message) {
  if (!session->is_connected) {
    return CLIENT_ERROR;
  }

  size_t udp_len;
  char *udp_packet = constuct_udp_packet(message, &udp_len, session->src_port, session->dest_port);
  if (!udp_packet) {
    return CLIENT_ERROR;
  }

  ssize_t sent_bytes = sendto(session->sockfd, udp_packet, udp_len, 0, (struct sockaddr *)&session->server_addr,
                              sizeof(session->server_addr));
  free(udp_packet);

  if (sent_bytes < 0 || (size_t)sent_bytes != udp_len) {
    return CLIENT_ERROR;
  }

  return CLIENT_SUCCESS;
}

static int client_receive_data(ClientSession *session, char *buf, size_t buf_size, struct sockaddr_in *src_addr,
                               uint16_t *src_port) {
  char recv_buf[BUFFER_SIZE];
  socklen_t addr_len = sizeof(struct sockaddr_in);

  char is_response = ((void *)src_addr == (void *)src_port) && src_addr == NULL;

  ssize_t recv_len = recvfrom(session->sockfd, recv_buf, sizeof(recv_buf), 0, src_addr, &addr_len);
  if (recv_len < 0) {
    return CLIENT_ERROR;
  }

  struct iphdr *ip_header = (struct iphdr *)recv_buf;
  if (ip_header->protocol != IPPROTO_UDP) {
    return CLIENT_ERROR;
  }

  size_t ip_header_len = ip_header->ihl * 4;

  struct udphdr *udp_header = (struct udphdr *)(recv_buf + ip_header_len);

  if (is_response) {
    if (ntohs(udp_header->dest) != session->src_port || ntohs(udp_header->source) != session->dest_port) {
      return CLIENT_NOT_OURS;
    }
  } else {
    if (ntohs(udp_header->dest) != session->src_port) {
      return CLIENT_NOT_OURS;
    }
  }

  size_t udp_header_len = sizeof(struct udphdr);
  size_t data_offset = ip_header_len + udp_header_len;
  size_t data_len = (size_t)recv_len - data_offset;

  if (data_len >= buf_size) {
    return CLIENT_BUFFER_TOO_SMALL;
  }

  memcpy(buf, recv_buf + data_offset, data_len);
  buf[data_len] = '\0';

  if (!is_response) {
    *src_port = ntohs(udp_header->source);
  }

  return CLIENT_SUCCESS;
}

const operations_t export = {
    .init = client_init,
    .connect = client_connect,
    .send_message = client_send_message,
    .receive = client_receive_data,
};
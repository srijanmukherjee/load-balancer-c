#include <arpa/inet.h>
#include <asm-generic/errno.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define do_or_die(x)                                                           \
  if ((x) < 0) {                                                               \
    fprintf(stderr, "(error) " #x " failed: %s\n", strerror(errno));           \
    exit(1);                                                                   \
  }

#define SERVER_COUNT 4
#define BUF_SIZE 4096
#define PROXY_SERVER_NAME "My proxy server"

int server_ports[SERVER_COUNT] = {8000, 8001, 8002, 8003};
const char *server_address = "127.0.0.1";
int current_server = 0;
int sockfd;

int handle_client_sync(int fd) {
  char buf[BUF_SIZE];

  // choose a server to forward the request
  int port = server_ports[current_server];
  printf("(info) forwarding request to port: %d\n", port);

  int remotefd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  struct sockaddr_in remote_addr = {0};
  remote_addr.sin_addr.s_addr = inet_addr(server_address);
  remote_addr.sin_port = htons(port);
  remote_addr.sin_family = AF_INET;

  if (connect(remotefd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) <
      0) {
    return 1;
  }

  int n;
  pid_t pid;

  if ((pid = fork()) == 0) {
    // remote to client
    while (1) {
      n = recv(remotefd, buf, BUF_SIZE, MSG_DONTWAIT);
      if (n == 0)
        break;

      if (n < 0) {
        if (errno == EWOULDBLOCK)
          continue;
        break;
      }

      if (send(fd, buf, n, 0) < 0) {
        break;
      }
    }

    exit(0);
  } else {
    // client to remote
    int stat;
    while (1) {
      // remote connection closed
      if (waitpid(pid, &stat, WNOHANG) > 0) {
        break;
      }

      n = recv(fd, buf, BUF_SIZE, MSG_DONTWAIT);
      if (n == 0)
        break;

      if (n < 0) {
        if (errno == EWOULDBLOCK)
          continue;
        break;
      }
      if (send(remotefd, buf, n, 0) < 0) {
        break;
      }
    }

    kill(pid, SIGKILL);

    wait(NULL);
  }

  close(remotefd);

  return 0;
}

int main() {
  struct sockaddr_in serv_addr = {0};
  struct sockaddr_in client_addr = {0};
  socklen_t addrlen = sizeof(client_addr);

  do_or_die(sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
  do_or_die(
      setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(5000);
  serv_addr.sin_family = AF_INET;

  do_or_die(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)));
  listen(sockfd, SOMAXCONN);

  while (1) {
    int clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
    if (clientfd < 0) {
      fprintf(stderr, "(error) accept() failed: %s\n", strerror(errno));
      exit(1);
    }

    current_server = (current_server + 1) % SERVER_COUNT;

    if (fork() == 0) {
      close(sockfd);
      handle_client_sync(clientfd);
      close(clientfd);
      exit(0);
    } else {
      close(clientfd);
    }
  }
  return 0;
}
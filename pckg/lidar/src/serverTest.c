// Server side implementation of UDP client-server model
#include <arpa/inet.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define MAXLINE 1024

// Driver code
int main() {
  int sockfd;
  char buffer[MAXLINE];
  uint16_t x = 0;
  uint16_t y = 0;
  struct sockaddr_in servaddr, cliaddr;

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  int len, n;

  len = sizeof(cliaddr); // len is value/result
  while (1) {
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0';

    y = (uint16_t)(1000 * fabs(sin(0.3 * x * (M_PI / 180.0))));
    sendto(sockfd, &x, sizeof(x), MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, len);

    x = x == 1080 ? 0 : x + 1;
    sendto(sockfd, &y, sizeof(y), MSG_CONFIRM,
           (const struct sockaddr *)&cliaddr, len);
  }

  return 0;
}
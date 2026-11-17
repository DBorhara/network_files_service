#include <stddef.h>
#define _POSIX_C_SOURCE 200809L

#include "nfsd.h"
#include <errno.h>
#include <netdb.h> //getaddrinfo(), struct addrinfo
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // socket(), bind()
#include <unistd.h>     // close()

// argc - 3
// argv[0] - "./nfsd" , argv[1] - port # "15000" , arg[2] - directory "files",
// argv[3] - NULL
int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Socket requires only 2 args: port and directory");
    exit(EXIT_FAILURE);
  }

  errno = 0;
  // Convert str to long "15000" -> 15000
  char *end; // pointer to where conversion stops from str to long
  long port = strtol(argv[1], &end, 10);
  char *directory = argv[2];

  if (errno == ERANGE || end == argv[1] || *end != '\0' || port < 1 ||
      port > 65535) {
    fprintf(stderr, "Invalid port number\n");
    exit(EXIT_FAILURE);
  }

  // hints tells getaddrinfo what kind of address we want
  struct addrinfo hints = {0};

  hints.ai_flags = AI_PASSIVE;     // 1-Get address for listening server
  hints.ai_family = AF_UNSPEC;     // 0-Accept IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // 1-Use TCP stream socket

  // linked list of hints for getaddrinfo to populate
  struct addrinfo *list;

  // populates local service address(NULL) list with our address requirements
  // from hints
  int result = getaddrinfo(NULL, argv[1], &hints, &list);

  if (result != 0) {
    fprintf(stderr, "getaddrinfo: %s\n",
            gai_strerror(result)); // gai_strerror converts getaddrinfo error
                                   // code to human readable string
    exit(EXIT_FAILURE);
  }

  // Initialize server file descriptor
  int serv_fd = -1;

  // Loop through and create a socket for the first valid socket in the list
  for (struct addrinfo *info = list; info != NULL; info = info->ai_next) {
    serv_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (serv_fd == -1) {
      continue;
    }

    // Successful bind
    if (bind(serv_fd, info->ai_addr, info->ai_addrlen) == 0) {
      break;
    }
    close(serv_fd);
    serv_fd = -1;
  }

  freeaddrinfo(list);

  if (serv_fd == -1) {
    fprintf(stderr, "Unable to bind socket\n");
    exit(EXIT_FAILURE);
  }

  // Listen on port
  if (listen(serv_fd, 5) == -1) {
    perror("listen");
    close(serv_fd);
    exit(EXIT_FAILURE);
  }

  printf("Listening on port: %ld\n", port);
  printf("Directory: %s\n", directory);

  // Wait for client connection
  int client_fd = accept(serv_fd, NULL, NULL);

  if (client_fd == -1) {
    perror("accept");
    close(serv_fd);
    exit(EXIT_FAILURE);
  }

  printf("Client connected.\n");
  // Buffer for read/write
  char buffer[1024];
  // read incoming client bytes to buffer
  ssize_t bytes_received = read(client_fd, buffer, sizeof(buffer));
  if (bytes_received == -1) {
    perror("read");
    close(client_fd);
    close(serv_fd);
    exit(EXIT_FAILURE);
  }
  // If not more incoming bytes from read, close connection
  if (bytes_received == 0) {
    printf("Client disconnected.\n");
    // else list number of incoming bytes from read
  } else {
    printf("Received %zd bytes.\n", bytes_received);

    // Total number of bytes sent via write
    ssize_t total_sent = 0;
    // write bytes back until total sent equals received bytes
    while (total_sent < bytes_received) {
      // write bytes back to client from buffer
      ssize_t sent = write(client_fd, buffer + total_sent,
                           (size_t)(bytes_received - total_sent));
      if (sent == 0) {
        if (sent < 0) {
          perror("write");
        } else {
          // No more to write back to client this
          fprintf(stderr, "write returned zero\n.");
        }
        // cleqn up
        close(client_fd);
        close(serv_fd);
        exit(EXIT_FAILURE);
      }
      total_sent += sent;
    }
  }
  close(client_fd);
  close(serv_fd);

  return EXIT_SUCCESS;
}

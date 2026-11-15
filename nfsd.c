#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Socket requires only 2 args: port and directory");
    exit(EXIT_FAILURE);
  }

  char *port_number = argv[1];
  char *directory_path = argv[2];

  // Created server socket and fd
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  printf("Socket created\n");
  printf("port number: %s\n", port_number);
  printf("Directory path: %s\n", directory_path);

  // Close server fd
  close(server_fd);
  return EXIT_SUCCESS;
}

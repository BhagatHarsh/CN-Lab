#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_PENDING 5
#define MAX_LINE 2048
#define PACKET_SIZE 1024

int main(int argc, char **argv)
{

  struct sockaddr_in sin;
  char buf[MAX_LINE] = "";
  socklen_t len;
  int s, new_s, port = 1234;
  char str[INET_ADDRSTRLEN], GET[] = "GET\n", EXIT[] = "EXIT\n", EMPTY[] = "", c;
  FILE *src;

  while ((c = getopt(argc, argv, "p:")) != -1)
  {
    switch (c)
    {
    case 'p':
      port = atoi(optarg);
      break;
    default:
      printf("Usage: %s -p <port_number> -f <file_name>\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  /* build address data structure */
  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);

  /* setup passive open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("simplex-talk: socket");
    exit(1);
  }

  inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
  printf("Server is using address %s and port %d.\n", str, port);

  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0)
  {
    perror("simplex-talk: bind");
    exit(1);
  }
  else
    printf("Server bind done.\n");

  listen(s, MAX_PENDING);

  /* wait for connection, then receive and print text */
  while (1)
  {
    if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0)
    {
      perror("simplex-talk: accept");
      exit(1);
    }
    printf("Server Listening.\n");
    while (len = recv(new_s, buf, sizeof(buf), 0) > 0)
    {
      fputs(buf, stdout);

      // printf("GET diff: %d\n", strcmp(buf, GET));
      // printf("EXIT diff: %d\n", strcmp(buf, EXIT));

      if (strcmp(buf, GET) == 0)
      {
        printf("GET request received.\n");
        // recieving file name
        strcpy(buf, EMPTY);
        if ((len = recv(new_s, buf, sizeof(buf), 0)) < 0)
        {
          perror("simplex-talk: recv");
          exit(1);
        }

        // Print the file name
        printf("File name: %s\n", buf);

        src = fopen(buf, "r");
        if (src == NULL)
        {
          printf("Source file not found. Exiting.\n");
          char error[] = "Source file not found. Exiting.\n";
          send(new_s, error, strlen(error), 0);
          close(new_s);
          close(s);
          exit(EXIT_FAILURE);
        }

        // Read the file into a buffer
        fseek(src, 0, SEEK_END);
        long file_size = ftell(src);
        rewind(src);
        char *buffer = (char *)malloc(file_size);
        fread(buffer, sizeof(char), file_size, src);
        fclose(src);

        // Send the buffer over the socket in chunks
        for (long i = 0; i < file_size; i += PACKET_SIZE)
        {
          long remaining = file_size - i;
          long chunk_size = remaining < PACKET_SIZE ? remaining : PACKET_SIZE;
          if (send(new_s, buffer + i, chunk_size, 0) < 0)
          {
            printf("Error sending file chunk.\n");
            break;
          }
        }

        // Send the EOT character to signal the end of the file
        if (send(new_s, "\x04", 1, 0) < 0)
        {
          printf("Error sending EOT character.\n");
        }

        free(buffer);
      }
      else if (strcmp(buf, EXIT) == 0)
      {
        printf("Exiting the server.\n");
        close(new_s);
        close(s);
        exit(1);
      }
      else
      {
        printf("Server received %d bytes\n", len);
      }
    }
    close(new_s);
  }

  close(s);
  return 0;
}

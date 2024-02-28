/* CSD 304 Computer Networks, Fall 2016
   Lab 2, server
   Team:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_PORT 5432
#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
  struct sockaddr_in sin;
  struct sockaddr_storage client_addr;
  char clientIP[INET_ADDRSTRLEN]; /* For IPv4 addresses */
  socklen_t client_addr_len;
  char buf[BUF_SIZE];
  int len;
  int s;
  char *host;
  struct hostent *hp;
  char filename[100] = "sample.wav";

  /* Declarations for file(s) to be sent
     ...
  */

  /* For inserting delays, use nanosleep()
     struct timespec ... */

  /* To get filename from commandline */
  /* if (argc==...) {} */
  if (argc == 3)
  {
    strcpy(filename, argv[2]);
  }
  else
  {
    printf("No filename specified. serving default: %s\n", filename);
  }

  /* Create a socket */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("server: socket");
    exit(1);
  }

  /* build address data structure and bind to all local addresses*/
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;

  /* If socket IP address specified, bind to it. */
  if (argc >= 2)
  {
    host = argv[1];
    hp = gethostbyname(host);
    if (!hp)
    {
      fprintf(stderr, "server: unknown host %s\n", host);
      exit(1);
    }
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
  }
  /* Else bind to 0.0.0.0 */
  else
    sin.sin_addr.s_addr = INADDR_ANY;

  sin.sin_port = htons(SERVER_PORT);

  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0)
  {
    perror("server: bind");
    exit(1);
  }
  else
  {
    /* Add code to parse IPv6 addresses */
    inet_ntop(AF_INET, &(sin.sin_addr), clientIP, INET_ADDRSTRLEN);
    printf("Server is listening at address %s:%d\n", clientIP, SERVER_PORT);
  }

  printf("Client needs to send \"GET\" to receive the file %s\n", argv[1]);

  client_addr_len = sizeof(client_addr);

  /* Receive messages from clients*/
  while (len = recvfrom(s, buf, sizeof(buf), 0,
                        (struct sockaddr *)&client_addr, &client_addr_len))
  {

    inet_ntop(client_addr.ss_family,
              &(((struct sockaddr_in *)&client_addr)->sin_addr),
              clientIP, INET_ADDRSTRLEN);

    if (strcmp(buf, "GET") == 0)
    {
      printf("Server received GET from %s\n", clientIP);
      printf("Sending file %s to client\n", filename);
      // break;
      /* Send to client */
      /* Add code to send file if the incoming message is GET */

      FILE *fp = fopen(filename, "r");
      if (fp == NULL)
      {
        fprintf(stderr, "Error opening file\n");
        exit(1);
      }

      while ((len = fread(buf, 1, sizeof(buf), fp)) > 0)
      {
        if (sendto(s, buf, len, 0,
                   (struct sockaddr *)&client_addr,
                   client_addr_len) == -1)
        {
          perror("server: sendto");
          exit(1);
        }
      }
      fclose(fp);
    }

    memset(buf, 0, sizeof(buf));
  }
  /* Send BYE to signal termination */
  // strcpy(buf, "BYE");
  // sendto(s, buf, sizeof(buf), 0,
  //        (struct sockaddr *)&client_addr, client_addr_len);
}

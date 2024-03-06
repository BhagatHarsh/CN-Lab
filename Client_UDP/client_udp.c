#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_PORT 5432
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{

  FILE *fp;
  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[BUF_SIZE];
  int s;
  int len;

  if ((argc == 2) || (argc == 3))
  {
    host = argv[1];
  }
  else
  {
    fprintf(stderr, "usage: client serverIP [download_filename(optional)]\n");
    exit(1);
  }

  if (argc == 3)
  {
    fp = fopen(argv[2], "wb");
    if (fp == NULL)
    {
      fprintf(stderr, "Error opening output file\n");
      exit(1);
    }
  }

  /* translate host name into peer's IP address */
  hp = gethostbyname(host);
  if (!hp)
  {
    fprintf(stderr, "client: unknown host: %s\n", host);
    exit(1);
  }
  else
    printf("Host %s found!\n", argv[1]);

  /* build address data structure */
  memset((char *)&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);

  /* create socket */
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("client: socket");
    exit(1);
  }

  printf("Client will get data from to %s:%d.\n", argv[1], SERVER_PORT);
  printf("To play the music, pipe the downlaod file to a player, e.g., ALSA, SOX, VLC: cat recvd_file.wav | vlc -\n");

  /* send message to server */
  if (sendto(s, "GET", 4, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    perror("Client: sendto()");
    return 0;
  }
  /* get reply, display it or store in a file*/
  // pipe it to vlc directly streaming
  FILE *vlcPipe = popen("vlc -", "w");
  while (1)
  {
    ssize_t bytes_received = recvfrom(s, buf, sizeof(buf), 0, NULL, NULL);
    if (bytes_received < 0)
    {
      perror("recvfrom");
      fclose(fp);
      close(s);
      exit(1);
    }

    // Check if the received data contains "BYE"
    if (strstr(buf, "BYE") != NULL)
    {
      printf("Received BYE message. Closing connection.\n");
      break;
    }
    fwrite(buf, 1, bytes_received, vlcPipe);
    // fwrite(buf, 1, bytes_received, fp);
  }
  pclose(vlcPipe);
  fclose(fp);
  close(s);
  // char *args[] = {"cvlc", argv[2], NULL};
  // execv(args[0], args);
}

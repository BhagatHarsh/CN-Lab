/*
Lab exam CSC 331, odd roll numbers
Nov 16, 2019
*/

/* FILL THIS BEFORE PROCEEDING

Name: Henil Shah
Roll number: AU1940205
Port assigned: 41001
IP address: 10.20.16.74

*/

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

#define MAX_LINE 256

/* complete the main call */
int main(int argc, char *argv[])
{

  struct hostent *hp;
  struct sockaddr_in sin;
  char *host;
  char buf[MAX_LINE];
  unsigned short server_port;

  /* Code to handle command line arguments.
      the first argument must be the remote IP address
      and the second argument must be the remote port number.

      if both the arguments are not present, print an error
      message and exit.
   */

  if (argc < 3)
  {
    perror("Err: usage missing arguments");
    exit(1);
  }
  else
  {
    host = argv[1]; // then gethostbyname
    server_port = atoi(argv[2])
  }

  /* translate host name into peer's IP address */
  hp = gethostbyname(host);
  if (!hp)
  {
    /* print unknown host error message and exit */
    perror("Err: unknown host");
    exit(1);
  }

  /* build address data structure sin*/

  /* zero out the data structure memory */
  bzero((char *)&sin, sizeof(sin));

  /* set address family to AF_INET*/
  sin.sin_family = AF_INET;

  /* set destination IP address */
  bcopy(hp->h_addr, (char *)sin.sin_addr, hp->h_length); // bcopy maj (char *)sin.sin_addr

  /* set destination port */
  sin.sin_port = htons(server_port);

  /* open a TCP socket and assign handle new_s
     check for error; print message and exit if error
     if socket is successfully created, print message
     confirming the same.

     if socket is successfully created, connect to the
     remote host and print message if successful

     If connect fails, close the socket, print error message and exit.

     Function call hints:
     socket(int socket_family, int socket_type, int protocol);

     int connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen)
   */
  int new_s;
  if ((new_s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Err: socket");
    exit(1);
  }
  printf("Socket created successfully\n");
  if (connect(new_s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
  {
    perror("Err: connect");
    close(new_s);
    exit(1);
  }
  printf("Connected Successfully!\n");

  /* protocol and messages
     print all sent and received messages on screen
     you can assume that messages from server are
     no longer than 256 bytes

     Message exchange sequence
     --------------------------

     send "greetings"
     receive message from server
     send your integer roll number (only numbers)
     receive an int (say u)
     send integer 11*u + 23
     close the socket
  */
  char *msg = "greetings";
  int len = strlen(msg) + 1;
  printf("Sending greetings\n");
  send(new_s, msg, len, 0);
  // bzero(buf, sizeof(buf));

  recv(new_s, buf, sizeof(buf), 0);
  printf("Received: %s \n", buf);

  int rollNo = 1940205;
  printf("Sending rollNo: %d\n", rollNo);
  send(new_s, &rollNo, sizeof(int), 0); // or can do sprintf(rollNo, "%s", buf)

  int resp;
  recv(new_s, &resp, sizeof(int), 0); // or recieve string and do atoi
  printf("Received %d from server\n", resp);

  int answer = (11 * resp) + 23;
  printf("Sending the answer %d\n", answer);
  send(new_s, &answer, sizeof(int), 0);

  close(new_s);

  // while(1){} //remove it after completing the assignment
  return 0;
}

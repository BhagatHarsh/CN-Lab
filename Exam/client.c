#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 5432
#define MAX_LINE 256

int main(int argc, char *argv[])
{
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[MAX_LINE], rbuf[MAX_LINE];
    int s;
    int len;
    if (argc == 2)
    {
        host = argv[1];
    }
    else
    {
        fprintf(stderr, "usage: %s host\n", argv[0]);
        exit(1);
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp)
    {
        fprintf(stderr, "%s: unknown host: %s\n", argv[0], host);
        exit(1);
    }
    else
        printf("Client's remote host: %s\n", argv[1]);

    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);

    s = socket(PF_INET, SOCK_STREAM, 0);

    if (s < 0)
    {
        printf("Error connecting the socket\n");
    }

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        printf("Could not connect\n");
    }

    // bzero(buf, sizeof(buf));
    strcpy(buf, "start");
    buf[MAX_LINE - 1] = '\0';
    printf("Send: %s\n", buf);
    len = strlen(buf) + 1;
    send(s, buf, len, 0);

    bzero(rbuf, sizeof(rbuf));
    recv(s, rbuf, sizeof(rbuf), 0);

    printf("Recieved: %s\n", rbuf);

    int number = htonl(2140084), rnumber = 0;
    printf("Send: %d\n", ntohl(number));
    send(s, &number, sizeof(number), 0);

    recv(s, &rnumber, sizeof(rnumber), 0);

    printf("Recieved: %d\n", ntohl(rnumber));

    bzero(buf, sizeof(buf));
    bzero(rbuf, sizeof(rbuf));
    strcpy(buf, "bye");
    buf[MAX_LINE - 1] = '\0';
    printf("Send: %s\n", buf);
    len = strlen(buf) + 1;
    send(s, buf, len, 0);

    recv(s, rbuf, sizeof(rbuf), 0);

    printf("Recieved: %s\n", rbuf);
}
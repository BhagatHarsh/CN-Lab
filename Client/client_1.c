#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_LINE 2048

int main(int argc, char *argv[])
{
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char host[] = "0.0.0.0";
    char buf[MAX_LINE], GET[] = "GET\n", EXIT[] = "EXIT\n", EMPTY[] = "";
    char c;
    int s;
    int len, port = 1234;
    char file_name[100] = "sample.txt";

    while ((c = getopt(argc, argv, "h:p:f:")) != -1)
    {
        switch (c)
        {
        case 'h':
            strcpy(host, optarg);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            strcpy(file_name, optarg);
            break;
        default:
            printf("Usage: %s -h <host_ip>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp)
    {
        fprintf(stderr, "%s: unknown host: %s\n", argv[0], host);
        exit(1);
    }
    else
    {
        printf("Client's remote host: %s and port %d\n", host, port);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(port);

    /* active open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("simplex-talk: socket");
        exit(1);
    }
    else
        printf("Client created socket.\n");

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }
    else
        printf("Client connected.\n");

    /* main loop: get and send lines of text */
    while (fgets(buf, sizeof(buf), stdin))
    {

        buf[MAX_LINE - 1] = '\0';
        len = strlen(buf) + 1;

        // printf("GET diff: %d\n", strcmp(buf, GET));
        // printf("EXIT diff: %d\n", strcmp(buf, EXIT));

        if (send(s, buf, len, 0) > 0)
        {
            printf("Command send: %s\n", buf);
        }
        else
        {
            printf("Sending failed %s\n", buf);
        }

        if (strcmp(buf, GET) == 0)
        {

            // sending filename
            if (send(s, file_name, strlen(file_name), 0) > 0)
            {
                printf("File name send: %s\n", file_name);
            }
            else
            {
                printf("Sending failed %s\n", file_name);
            }

            printf("Recieving File!\n");
            strcpy(buf, EMPTY);
            if (recv(s, buf, sizeof(buf), 0) > 0)
            {
                fputs(buf, stdout);
            }
            printf("\nFile Recieved! %s\n", buf);
            // create the file received and add the contents received
            FILE *fp = fopen(file_name, "w");
            if (fp == NULL)
            {
                printf("File not created\n");
                exit(1);
            }
            fwrite(buf, sizeof(char), strlen(buf), fp);
            fclose(fp);
        }
        else if (strcmp(buf, EXIT) == 0)
        {
            printf("Exiting.\n");
            close(s);
            exit(1);
        }
        strcpy(buf, EMPTY);
    }

    close(s);
    return 0;
}

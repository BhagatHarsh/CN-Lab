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
    char str[INET_ADDRSTRLEN], GET[] = "GET\n", EXIT[] = "EXIT\n", c;
    char file_name[100] = "sample.txt";
    FILE *src;

    while ((c = getopt(argc, argv, "p:f:")) != -1)
    {
        switch (c)
        {
        case 'p':
            port = atoi(optarg);
            break;
        case 'f':
            strcpy(file_name, optarg);
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

            printf("GET diff: %d\n", strcmp(buf, GET));
            printf("EXIT diff: %d\n", strcmp(buf, EXIT));

            if (strcmp(buf, GET) == 0)
            {

                printf("Serving %s to you!\n", file_name);

                src = fopen(file_name, "r");
                if (src == NULL)
                {
                    printf("Source file not found. Exiting.\n");
                    char error[] = "Source file not found. Exiting.\n";
                    send(new_s, error, strlen(error), 0);
                    close(new_s);
                    close(s);
                    exit(EXIT_FAILURE);
                }

                FILE *src = fopen(file_name, "rb");
                if (src == NULL)
                {
                    printf("Source file not found. Exiting.\n");
                }

                // Send the file in packets
                char packet[PACKET_SIZE];
                size_t bytes_read;
                while ((bytes_read = fread(packet, sizeof(char), PACKET_SIZE, src)) > 0)
                {
                    if (send(new_s, packet, bytes_read, 0) < 0)
                    {
                        printf("Error sending file.\n");
                    }
                    else
                    {
                        printf("Sent: %s\n", packet);
                    }
                }

                // Close the file
                fclose(src);
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

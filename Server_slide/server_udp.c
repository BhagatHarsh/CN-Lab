#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define SERVER_PORT 5432
#define BUF_SIZE 1024
#define WINDOW_SIZE 1

typedef struct
{
    uint8_t type;
    uint8_t filename_size;
    char filename[BUF_SIZE];
} FileRequest;

typedef struct
{
    uint8_t type;
    uint16_t sequence_number;
    uint8_t filename_size;
    char filename[BUF_SIZE];
    uint32_t file_size;
    uint16_t block_size;
    char data[BUF_SIZE];
} FileInfoAndData;

typedef struct
{
    uint8_t type;
    uint16_t sequence_number;
    uint16_t block_size;
    char data[BUF_SIZE];
} Data;

typedef struct
{
    uint8_t type;
    uint8_t filename_size;
    char filename[BUF_SIZE];
} FileNotFound;

typedef struct
{
    uint8_t type;
    uint8_t num_sequences;
    uint16_t sequence_no[BUF_SIZE];
} ACK;

int main(int argc, char *argv[])
{
    int s, len;
    struct sockaddr_in sin;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    char buf[BUF_SIZE];
    char file_buf[BUF_SIZE];

    // Socket setup
    s = socket(PF_INET, SOCK_DGRAM, 0);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(SERVER_PORT);
    bind(s, (struct sockaddr *)&sin, sizeof(sin));

    // Variables to keep track of the window
    uint16_t window_start = 1;         // Start of the window
    uint16_t window_end = WINDOW_SIZE; // End of the window
    uint16_t next_sequence_number = 1; // Next sequence number to send

    // Main loop
    while (1)
    {
        client_addr_len = sizeof(client_addr);
        len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (len < 0)
        {
            perror("server: recvfrom");
            exit(1);
        }

        // Determine the type of the received data
        uint8_t type = *(uint8_t *)buf;
        switch (type)
        {
        case 0: // File request
            FileRequest *request = (FileRequest *)buf;
            printf("Received file request\n");
            printf("Filename: %s\n", request->filename);
            // Open file and send initial data
            int fdd = open(request->filename, O_RDONLY);
            if (fdd < 0)
            {
                // File not found
                printf("File not found\n");
                FileNotFound not_found;
                not_found.type = 4;
                not_found.filename_size = strlen(request->filename);
                memcpy(not_found.filename, request->filename, not_found.filename_size);
                sendto(s, &not_found, sizeof(not_found), 0, (struct sockaddr *)&client_addr, client_addr_len);
                break;
            }

            FILE* fd = fopen(request->filename, "rb");
            // File found, send file info and data
            printf("File found\n");
            strcpy(file_buf, request->filename);
            struct stat st;
            fstat(fdd, &st);
            uint32_t file_size = htonl(st.st_size);

            FileInfoAndData info_and_data;
            info_and_data.type = 2;
            info_and_data.sequence_number = htons(1);
            info_and_data.filename_size = request->filename_size;
            memcpy(info_and_data.filename, request->filename, info_and_data.filename_size);
            info_and_data.file_size = htonl(file_size);
            size_t bytes_read = fread(info_and_data.data, 1, BUF_SIZE, fd);
            printf("File size: %u\n", ntohl(info_and_data.file_size));
            printf("File size: %lu\n", st.st_size);
            printf("Block size: %lu\n", bytes_read);
            info_and_data.block_size = htonl(bytes_read);

            sendto(s, &info_and_data, sizeof(info_and_data), 0, (struct sockaddr *)&client_addr, client_addr_len);
            printf("Sent file info and data\n");

            break;
        case 1: // ACK
            ACK *ack = (ACK *)buf;
            printf("Received ACK for sequence number: %u\n", ntohs(ack->sequence_no[0]));
            // Check if the received ACK is for a packet in the window
            if (ntohs(ack->sequence_no[0]) >= window_start && ntohs(ack->sequence_no[0]) < window_end)
            {
                // Acknowledged packet is in the window, slide the window
                window_start = ntohs(ack->sequence_no[0]) + 1;
                window_end = window_start + WINDOW_SIZE;
                printf("Sliding window to %u - %u\n", window_start, window_end);

                Data data_block;
                // Read and send the next packet
                size_t bytes_read = fread(data_block.data, 1, BUF_SIZE, fd);
                printf("Read %lu bytes\n", bytes_read);
                if (bytes_read > 0)
                {
                    // Create and send the Data block
                    data_block.type = 3;
                    data_block.sequence_number = htons(next_sequence_number);
                    data_block.block_size = htonl(bytes_read);
                    sendto(s, &data_block, sizeof(data_block), 0, (struct sockaddr *)&client_addr, client_addr_len);
                    printf("Sent data block %u\n", next_sequence_number);
                    next_sequence_number++;
                }
                else
                {
                    // End of file reached, send BYE message
                    Data bye_message;
                    bye_message.type = 3;
                    bye_message.sequence_number = htons(next_sequence_number);
                    bye_message.block_size = htons(0);
                    sendto(s, &bye_message, sizeof(bye_message), 0, (struct sockaddr *)&client_addr, client_addr_len);
                    printf("End of file reached\n");
                    fclose(fd);
                }
            }
            break;
        default:
            printf("Unknown data type\n");
            break;
        }
    }

    close(s);
    return 0;
}

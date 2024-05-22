#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 5432
#define BUF_SIZE 1024

typedef struct {
    uint8_t type;
    uint8_t filename_size;
    char filename[BUF_SIZE];
} FileRequest;

typedef struct {
    uint8_t type;
    uint16_t sequence_number;
    uint8_t filename_size;
    char filename[BUF_SIZE];
    uint32_t file_size;
    uint16_t block_size;
    char data[BUF_SIZE];
} FileInfoAndData;

typedef struct {
    uint8_t type;
    uint16_t sequence_number;
    uint16_t block_size;
    char data[BUF_SIZE];
} Data;

typedef struct {
    uint8_t type;
    uint8_t filename_size;
    char filename[BUF_SIZE];
} FileNotFound;

typedef struct {
    uint8_t type;
    uint8_t num_sequences;
    uint16_t sequence_no[BUF_SIZE];
} ACK;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <filename>\n", argv[0]);
        return 1;
    }

    int s, len;
    struct sockaddr_in sin;
    struct sockaddr_storage server_addr;
    socklen_t server_addr_len;
    char buf[BUF_SIZE];

    // Socket setup
    s = socket(PF_INET, SOCK_DGRAM, 0);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, argv[1], &sin.sin_addr);

    // Send GET request
    FileRequest request;
    request.type = 0;
    request.filename_size = htons(strlen(argv[2]));
    memcpy(request.filename, argv[2], strlen(argv[2]) + 1);
    sendto(s, &request, sizeof(request), 0, (struct sockaddr *)&sin, sizeof(sin));

    // Main loop
    uint16_t expected_sequence_number = 1;
    FILE *file = NULL;
    while (1) {
        server_addr_len = sizeof(server_addr);
        len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (len < 0) {
            perror("client: recvfrom");
            exit(1);
        }

        // Determine the type of the received data
        uint8_t type = *(uint8_t *)buf;
        printf("Received data type: %d\n", type);
        switch (type) {
            case 2: // File info and data
                FileInfoAndData *info_and_data = (FileInfoAndData *)buf;
                printf("Received file info and data\n");
                if (info_and_data->file_size > 0) {
                    // Process FileInfoAndData
                    printf("File size: %u\n", info_and_data->file_size);
                    printf("Block size: %u\n", info_and_data->block_size);

                    // Open file for writing
                    file = fopen("recieved_file.mp4", "wb");
                    if (file == NULL) {
                        perror("client: fopen");
                        exit(1);
                    }

                    // Write data to file
                    fwrite(info_and_data->data, 1, ntohl(info_and_data->block_size), file);

                    // Send ACK
                    ACK ack;
                    ack.type = 1;
                    ack.num_sequences = 1;
                    ack.sequence_no[0] = info_and_data->sequence_number;
                    sendto(s, &ack, sizeof(ack), 0, (struct sockaddr *)&sin, sizeof(sin));
                    printf("Sent ACK for sequence number: %u\n", ntohs(info_and_data->sequence_number));
                }
                break;
            case 3: // Data chunk
                Data *data = (Data *)buf;
                printf("Received data chunk %d\n", data->sequence_number);
                if (ntohs(data->sequence_number) == expected_sequence_number) {
                    // Write data to file
                    fwrite(data->data, 1, ntohs(data->block_size), file);

                    // Send ACK
                    ACK ack;
                    ack.type = 1;
                    ack.num_sequences = 1;
                    ack.sequence_no[0] = data->sequence_number;
                    sendto(s, &ack, sizeof(ack), 0, (struct sockaddr *)&sin, sizeof(sin));
                    printf("Sent ACK for sequence number: %u\n", data->sequence_number);

                    expected_sequence_number++;
                } else {
                    printf("Received out-of-order data chunk, expected %u but got %u\n", expected_sequence_number, ntohs(data->sequence_number));
                }
                break;
            case 4: // File not found
                FileNotFound *not_found = (FileNotFound *)buf;
                printf("File not found: %s\n", not_found->filename);
                exit(1);
                break;
            default:
                printf("Unknown data type\n");
                break;
        }
    }

    // Close file after receiving all data
    if (file != NULL) {
        fclose(file);
    }

    close(s);
    return 0;
}

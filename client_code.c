#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 1024

void send_get_request(int sock, const char* path, const char* server_ip) {
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    // Format a valid HTTP/1.0 GET request
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.0\r\n"  // <-- Modified to HTTP/1.0
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, server_ip);

    // Send the GET request
    send(sock, request, strlen(request), 0);

    // Receive the server's response
    while (recv(sock, response, BUFFER_SIZE - 1, 0) > 0) {
        printf("%s", response);
        memset(response, 0, BUFFER_SIZE);
    }
}

void send_put_request(int sock, const char* path, const char* server_ip, const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File not found");
        exit(1);
    }

    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char file_buffer[BUFFER_SIZE];
    size_t file_size = 0;
    size_t bytes_read;

    // Find the size of the file
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Format a valid HTTP/1.0 PUT request
    snprintf(request, sizeof(request),
        "PUT %s HTTP/1.0\r\n"  // <-- Modified to HTTP/1.0
        "Host: %s\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, server_ip, file_size);

    // Send the PUT request and file content
    send(sock, request, strlen(request), 0);

    // Send the file in chunks
    while ((bytes_read = fread(file_buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(sock, file_buffer, bytes_read, 0);
    }

    fclose(file);

    // Receive the server's response
    while (recv(sock, response, BUFFER_SIZE - 1, 0) > 0) {
        printf("%s", response);
        memset(response, 0, BUFFER_SIZE);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        printf("Usage: %s <server_ip> <port> <method: GET/PUT> <path> [<filename> for PUT]\n", argv[0]);
        return 1;
    }

    const char* server_ip = argv[1];
    int port = atoi(argv[2]);
    const char* method = argv[3];
    const char* path = argv[4];

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Define server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        return 1;
    }

    // Check the method and handle appropriately
    if (strcmp(method, "GET") == 0) {
        send_get_request(sock, path, server_ip);
    } else if (strcmp(method, "PUT") == 0 && argc == 6) {
        const char* filename = argv[5];
        send_put_request(sock, path, server_ip, filename);
    } else {
        printf("Invalid method or arguments. Usage: %s <server_ip> <port> <method: GET/PUT> <path> [<filename> for PUT]\n", argv[0]);
    }

    close(sock);
    return 0;
}

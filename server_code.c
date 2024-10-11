#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 1024

void handle_get_request(int client_sock, const char* path) {
    char response[BUFFER_SIZE];
    
    // Basic HTML response for GET request
    const char* html_content = "<html><body><h1>Welcome to the HTTP Server!</h1></body></html>";
    
    // Format HTTP/1.0 response
    snprintf(response, sizeof(response),
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n"
        "\r\n%s",
        strlen(html_content), html_content);

    // Send the response to the client
    send(client_sock, response, strlen(response), 0);
}

void handle_put_request(int client_sock, const char* path) {
    FILE *file = fopen("uploaded_file.txt", "wb");
    if (!file) {
        perror("Failed to open file");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Receive file content and write to file
    while ((bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    fclose(file);

    // Send response
    const char* response = "HTTP/1.0 200 OK\r\nConnection: close\r\n\r\nFile uploaded successfully.";
    send(client_sock, response, strlen(response), 0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Bind the socket to the port
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Listening on port 8080
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding failed");
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_sock, 10) == -1) {
        perror("Listening failed");
        return 1;
    }

    printf("Server listening on port 8080...\n");

    while (1) {
        // Accept incoming connection
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock == -1) {
            perror("Connection acceptance failed");
            continue;
        }

        // Receive request from the client
        recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        printf("Request:\n%s\n", buffer);

        // Determine request type (GET or PUT)
        if (strncmp(buffer, "GET", 3) == 0) {
            handle_get_request(client_sock, "/index.html");
        } else if (strncmp(buffer, "PUT", 3) == 0) {
            handle_put_request(client_sock, "/uploaded_file.txt");
        }

        // Close client socket
        close(client_sock);
    }

    // Close server socket
    close(server_sock);
    return 0;
}

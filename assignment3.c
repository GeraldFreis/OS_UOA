#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>  // For close()
#include <arpa/inet.h>  // For inet_addr, sockaddr_in
#include <sys/socket.h>  // For socket functions

// Function to retrieve the -l and -p values
int get_input_values(int argc, char *argv[], int *l_int, char **p_value) {

    char *l_value = NULL;

    // Loop through command-line arguments
    for (int i = 1; i < argc; i++) {
        // Check for the -l flag and get the next argument as the value
        if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            l_value = argv[i + 1];
            i++; // Skip the value part in the next iteration
        }
        // Check for the -p flag and get the next argument as the value
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            *p_value = argv[i + 1];
            i++; // Skip the value part in the next iteration
        }
    }

    // Convert l_value to integer safely
    char *endptr;
    errno = 0;
    *l_int = strtol(l_value, &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Invalid value for -l, must be an integer\n");
        return -1; // Return error
    }

    return 0; 
}

typedef struct Node {
    Node *next = NULL;
    Node *book_next = NULL;
    char data[1024];
    Node *next_frequent_search = NULL;
}

int main(int argc, char *argv[]) {
    int listen_port;
    char *search_pattern = NULL;

    // Get the values for -l and -p
    if (get_input_values(argc, argv, &listen_port, &search_pattern) != 0) {
        return 1; // Error occurred, exit the program
    }

    int server_fd, new_socket;
    struct sockaddr_in address;

    int opt = 1;
    int addrlen = sizeof(address);

    char buffer[1024] = {0};

    // 1. Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Set socket options (optional, for reuse of the address)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Define the address and port for the socket
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address
    address.sin_port = htons(listen_port);  // Host-to-network conversion for port

    // 4. Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 5. Listen for incoming connections (with a backlog of 3)
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    Node *root_node = NULL;
    Node *current_node = root_node;

   while (1) {  // Keep the server running to accept multiple connections
        // Accept a connection from a client
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // writing the output to std::out and creating nodes
        int bytes_read;
        char filename
        while ((bytes_read = read(new_socket, buffer, 1024)) > 0) {
            buffer[bytes_read] = '\0';  
            printf("%s", buffer);  
            // now adding a new node 
        }


        close(new_socket);
    }

    // Close the server socket when done (this will only be reached if you break the loop)
    close(server_fd);

    return 0;
}

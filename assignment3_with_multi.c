#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>  // For close()
#include <arpa/inet.h>  // For inet_addr, sockaddr_in
#include <sys/socket.h>  // For socket functions
#include <pthread.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>

struct Node { // this will store the nodes
    char *data;
    struct Node *next;
    struct Node *book_next;
    struct Node *next_frequent_search;
};

// global variables

struct Node *root = NULL; // root noode set to zero as we can check if it has been defined
pthread_mutex_t node_mutex = PTHREAD_MUTEX_INITIALIZER;
int connection_counter = 0; // this will be used when naming the files, when we have finished reading them in
char **
// Function to get the -l and -p values
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



// functions to create new nodes and add nodes

struct Node *new_node(char *data)
{
    // function to create new nodes (just setting all their ptrs to zero)
    struct Node *new = (struct Node* ) malloc  ( sizeof (struct Node));
    new->data = strdup(data);
    new->book_next, new->next, new->next_frequent_search = NULL;
    return new;
}

void add(struct Node* node_to_add)
{
    // this adds nodes to the linked list
    // but first we have to claim access to the linked list
    pthread_mutex_lock(&node_mutex);

    // checking if the root has been defined, and if so we just set the root as the new node
    if ( root == NULL )
    {
        root = node_to_add;
        pthread_mutex_unlock(&node_mutex);
        return;
    }

    struct Node * temporary = root;
    while (temporary->next != NULL) { temporary = temporary->next;}
    temporary->next = node_to_add;
    pthread_mutex_unlock(&node_mutex);
    return;
}

// okay so now is the fucky part, I want to handle incoming connections in separate thread
// because of this I will have a function that handles each thread and the reading and adding to the node list
int counter = 0;

void* separate_connections(void *socket)
{
    int socket_file_desc = * (int *)socket;
    free(socket);
    int book_id = connection_counter;

    char lines[1024];
    int bytes;

    struct Node *root_of_book = NULL;
    struct Node *final_book_node = NULL;

    while ( (bytes = recv(socket_file_desc, lines, sizeof(lines)-1, 0)) > 0 )
    {
        // while we can read and are not erroring out
        lines[bytes] = '\0'; // because we are good boys and need null to show strend

        char *line = strtok(lines, "\n"); //making linebreaks separate entries in the list
        // fprintf(stderr, "%s\n", line);
        
        struct Node * new = new_node(line);
        add(new); // adding to the global list
        counter++;
        // now writing to the book node list
        if(root_of_book == NULL) { root_of_book = new; }
        // if the final node is not empty we want to over-ride it's next book ptr
        if(final_book_node != NULL) { final_book_node->book_next = new; }
        // now over-riding the last node, as we have set the correct ptr
        final_book_node = new;

        fprintf(stderr, "Book #: %d and Node #: %d \t containing data of: %s\n", book_id, counter, new->data); 

        
    }

    // given we should have correctly read in a book we should now write the 
    // book to a file
    char filename[20];
    snprintf( filename, sizeof(filename), "book_%02d.txt", book_id );
    // the above is because if you have more than 10 connections it may output oddly

    FILE *book = fopen(filename, "w"); // creating and writing to the file
    if(book != NULL)
    {
        struct Node * temporary = root_of_book;
        while( temporary != NULL )
        {
            fprintf(book, "%s\n", temporary->data); // printing the data to the file
            temporary = temporary->book_next;
        }
        fclose(book);

    }

    close(socket_file_desc);
    return NULL;
}

// now doing the analysis threads
pthread_cond_t analysis_cond;     // Condition variable for analysis thread
int analysis_interval = 5;        // Interval in seconds for analysis thread
int analysis_ready = 0;           // Flag to trigger analysis output
char *search_pattern = NULL; // this gets initially set and then used in the analysis


void *analysis(void *argument)
{
    // has to be void ptr because its a thread function
    // meow meow meow meow. I have like 14 days until my first exam and im spending time doing this bullshit as opposed to studying
    while(1)
    {
        // indefinitely we do this analysis
        // first we lock the list because I will be doing little runs
        pthread_mutex_lock(&node_mutex);

        // now we wait for the analysis interval
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 5; 

        pthread_cond_timedwait(&analysis_cond, &node_mutex, &ts);

        if(analysis_ready) // if the analysis bit is set to 0
        {
            analysis_ready = 0; // resetting

            struct Node * temporary = root;
            int matches = 0;

            while( temporary != NULL) // iterating across all nodes until we hit the search string
            {
                if( strstr(temporary->data, search_pattern) != NULL ) // if there is a match
                {
                    matches++;
                }
                temporary = temporary->next;
            }
            printf()

        }
    }
}


// now setting up the server

int main(int argc, char *argv[])
{
    int listen_port;

    // Get the values for -l and -p
    if (get_input_values(argc, argv, &listen_port, &search_pattern) != 0) {
        return 1; // Error occurred, exit the program
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    // creating a socket fd so that we can actually read and write lol

    if( (server_fd  = socket(AF_INET, SOCK_STREAM, 0)) == 0 ){
        perror("Socket no bueno");
        exit(EXIT_FAILURE);
    }

    // setting the socket options, i.e. reusability
    if ( setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("not happy with socket options");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // configuring server addy and binding to the designated port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(listen_port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind :(");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening on the socket
    if (listen(server_fd, 10) < 0) {
        perror("Listen :(");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0) {
        fprintf(stderr, "hello\n");
        connection_counter++;
        pthread_t thread_id;
        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        // Create a new thread to handle the connection
        if (pthread_create(&thread_id, NULL, separate_connections, client_socket) != 0) {
            perror("Failed to create thread");
            close(new_socket);
        } else {
            pthread_detach(thread_id);  // Automatically free thread resources after termination
        }
    }

    if (new_socket < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    close(server_fd);
    return 0;
}
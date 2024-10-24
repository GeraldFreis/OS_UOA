#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

// node structure for linked list
struct Node {
    int book_id; // just for easy matches counting traversal
    char *data;
    struct Node *next;
    struct Node *book_next;
};
// book frequency structure for search pattern analysis
struct BookFrequency {
    int book_id;
    int matches;
};

// shared variables
struct Node *root = NULL;
struct BookFrequency *book_frequencies = NULL;

// input variables
char *search_pattern;
int port_number;
// linked list and mutex's and conditionals
int n_books = 0;

pthread_mutex_t linked_list_mutex;
pthread_cond_t analysis_conditional;
int analysis_flag = 0; // this will be switched on so that analysis can be done

// Function to get the -l and -p values
int get_input_values(int argc, char *argv[], int *l_int, char **p_value)
{

    char *l_value = NULL;

    // Loop through command-line arguments
    for (int i = 1; i < argc; i++)
    {
        // Check for the -l flag and get the next argument as the value
        if (strcmp(argv[i], "-l") == 0 && i + 1 < argc)
        {
            l_value = argv[i + 1];
            i++; // Skip the value part in the next iteration
        }
        // Check for the -p flag and get the next argument as the value
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
        {
            *p_value = argv[i + 1];
            i++; 
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

// function to add new nodes to the linked list
void add_node(const char *data, int book_id, struct Node **previous_node, struct Node **book_root)
{
    pthread_mutex_lock(&linked_list_mutex);

    // creating the new node
    struct Node *new_node = (struct Node*)malloc(sizeof(struct Node));
    if(data == NULL){data = "";}
    new_node->data = strdup(data);
    new_node->next = NULL;
    new_node->book_id = book_id;
    new_node->book_next = NULL;

    // Add the node to the main linked list
    if (root == NULL)
    {
        root = new_node;
    } else {
        struct Node *temporary = root;
        while (temporary->next != NULL) 
        {
            temporary = temporary->next;
        }
        temporary->next = new_node;
    }

    // Add the node to the book-specific linked list
    if (*book_root == NULL) 
    {
        *book_root = new_node;  // Set the root of the book-specific list
    }
    if (*previous_node != NULL) 
    {
        (*previous_node)->book_next = new_node;  // Link the previous book node
    }
    *previous_node = new_node;  // Update the previous node

    fprintf(stderr, "Added node with data: %s to book %d\n", data, book_id);
    
    pthread_mutex_unlock(&linked_list_mutex);
}

int count_occurrences_of_substr(const char *data, const char *s_p)
{
    if(data == NULL || s_p == NULL) return 0;

    int n = strlen(data);
    int m = strlen(s_p);
    int full_count = 0;

    // iterating through and checking if a substring of characters matches the search pattern
    for (int i = 0; i <= n - m; i++) 
    {
        int j = 0;
        // Check if the substring matches starting from position i
        while (j < m && data[i + j] == s_p[j]) 
        {
            j++;
        }
        // If we matched the full substring, increment the match count
        if (j == m) 
        {
            full_count++;
        }
    }
    return full_count;
}

int count_matches_in_book(struct Node *book_head)
{
    // this function iterates over each book and finds the number of matches with the search pattern
    int match_count = 0;
    struct Node *current = book_head;

    while ( current != NULL )
    {
        match_count += count_occurrences_of_substr(current->data, search_pattern);
        current = current->book_next;
    }
    return match_count;
}

// Compare function for sorting books by match frequency
int compare_books(const void *book_1, const void *book_2)
{
    struct BookFrequency *bookA = (struct BookFrequency *)book_1;
    struct BookFrequency *bookB = (struct BookFrequency *)book_2;
    return bookB->matches - bookA->matches;  // Sort in descending order
}

// Function to search for the pattern and count occurrences, then sort and print the results
void *analysis_thread(void *arg) 
{
    while (1) 
    {
        pthread_mutex_lock(&linked_list_mutex);

        // Wait for the analysis interval
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 2;

        pthread_cond_timedwait(&analysis_conditional, &linked_list_mutex, &ts);

        if (analysis_flag)
         {
            // Perform analysis
            analysis_flag = 0;  // Reset the flag once analysis starts, ensuring no other thread can trigger it

            // Initialize frequency tracking
            free(book_frequencies);  // Reset previous array
            book_frequencies = (struct BookFrequency *)malloc(n_books * sizeof(struct BookFrequency));
            // initialising the array
            for (int i = 0; i < n_books; i++) 
            {
                book_frequencies[i].book_id = i;
                book_frequencies[i].matches = 0;
            }

            // Traverse the list to count matches per book
            struct Node *current = root;
            

            for (int i = 0; i < n_books; i++)
            {
                if(current != NULL) 
                {
                    book_frequencies[i].matches = count_matches_in_book(current);

                    // finding next root
                    while(current != NULL && current->book_id != i + 1)
                    { // iterating until we find a node with the next book_id (this has to be the root of that book)
                        current = current->next;
                    }
                    // now current should either be NULL or have the next book id required
                }
            }

            // Sort the books by frequency of pattern matches
            qsort(book_frequencies, n_books, sizeof(struct BookFrequency), compare_books);

            // Output the books sorted by frequency
            printf("Analysis Results: Books sorted by search pattern matches\n");
            for (int i = 0; i < n_books; i++) {
                printf("Book %d: %d matches\n", book_frequencies[i].book_id, book_frequencies[i].matches);
            }
        }

        pthread_mutex_unlock(&linked_list_mutex);
        sleep(2);
    }
    return NULL;
}


void print_book(int book_id)
{
    char filename[20];
    int file_book_id = book_id+1;
    
    snprintf( filename, sizeof(filename), "book_%02d.txt", file_book_id); // book_id starts at 0 so need to increment
    // snprintf( filename, sizeof(filename), "book_01.txt", file_book_id); // book_id starts at 0 so need to increment

    // the above is because if you have more than 10 connections it may output oddly
    struct Node * temp = root;
    while(temp != NULL && temp->book_id != book_id ) // iterating to the first node that is of the same type as we need (this has to be the root)
    {
        temp = temp->next;
    }


    FILE *book = fopen(filename, "w"); // creating and writing to the file
    if(book != NULL)
    {
        struct Node * temporary = temp;
        while( temporary != NULL )
        {
            fprintf(book, "%s\n", temporary->data); // printing the data to the file
            temporary = temporary->book_next;
        }
        fclose(book);
    }

}

// Function for client connection handling (non-blocking reads and storing data)
void *handle_client(void *client_socket)
{
    int sock = *(int*)client_socket;
    char buffer[1024];

    pthread_mutex_lock(&linked_list_mutex);
    int book_id = n_books++;  // Each connection is considered a new book
    pthread_mutex_unlock(&linked_list_mutex);

    struct Node *root_of_book = NULL;
    struct Node *final_book_node = NULL;

    while (1)
    {
        ssize_t bytes_read = recv(sock, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate the received data

            // Tokenize by newline and add each line as a node to the list
            char *line = strtok(buffer, "\n");
            while (line != NULL)
            {
                add_node(line, book_id, &final_book_node, &root_of_book);  // Add line to shared list
                line = strtok(NULL, "\n");
            }

            // Notify analysis threads to start searching
            pthread_mutex_lock(&linked_list_mutex);
            if (!analysis_flag)
            {  // Only trigger analysis if it's not already running
                analysis_flag = 1;  // Set flag to indicate analysis is in progress
                pthread_cond_broadcast(&analysis_conditional);  // Wake up analysis threads
            }
            pthread_mutex_unlock(&linked_list_mutex); // unlocking so other fellers can actually use the linked list
        }

        if (bytes_read == 0) {
            close(sock);
            print_book(book_id);
            break;
        }
    }

    

    return NULL;
}


int main(int argc, char *argv[]) {

    if( get_input_values(argc, argv, &port_number, &search_pattern) < 0){
        perror("Unable to find input values");
        exit(EXIT_FAILURE);
    }

    // Initialize the mutex and condition variable
    pthread_mutex_init(&linked_list_mutex, NULL);
    pthread_cond_init(&analysis_conditional, NULL);

    // Create analysis threads
    pthread_t analysis1, analysis2;
    pthread_create(&analysis1, NULL, analysis_thread, NULL);
    pthread_create(&analysis2, NULL, analysis_thread, NULL);

    // Setup server socket
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Foot not in Sock");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Unhapy socket binding");
        exit(EXIT_FAILURE);
    }

    listen(server_socket, 20); // 20 because you never know how much you will need

    // Accept incoming connections and create new threads for each client
    int counter = 0;
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)))
    {
        char filename[20];
        int file_book_id = counter+1;
    
        snprintf( filename, sizeof(filename), "touch book_%02d.txt", file_book_id); // book_id starts at 0 so need to increment
        int out = system(filename);

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, &client_socket);

        pthread_detach(client_thread); // Detach so we don't need to join
        counter++;
    }

    // Clean up
    close(server_socket);
    pthread_mutex_destroy(&linked_list_mutex);
    pthread_cond_destroy(&analysis_conditional);
    free(book_frequencies);

    return 0;
}
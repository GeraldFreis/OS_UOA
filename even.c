#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


// the function that will be redirected to if sighup
void sighup_handler(int uselessint){
    fprintf(stdout, "Ouch!\n");

}

void sigint_handler(int uselessint){
    fprintf(stdout, "Yeah!\n");
 
}

int main(int argc, char *argv[]){
    // printf("%d", getpid());
    // getting the commandline argument
    long first_n = strtol(argv[1], argv, 0);

    // setting the signal handler to redirect
    struct sigaction sighup_struct, sigint_struct;
    // setting all structure bits to zero
    // bzero(&sighup_struct, sizeof(sighup_struct)); // incase it's garbage that's thrown in
    // bzero(&sigint_struct, sizeof(sigint_struct));

    sighup_struct.sa_handler = &sighup_handler;
    sigint_struct.sa_handler = &sigint_handler;

    sigaction(SIGHUP, &sighup_struct, NULL);
    sigaction(SIGINT, &sigint_struct, NULL);


    // iterating to the first n even numbers
    int counter = 0;
    for(int i = 0; counter < first_n; i++){
        if( i % 2 == 0){
            printf("%d\n", i); // if the number is even (divisible by 2 with no remainder) then we want to print
            sleep(5);
            counter++;
        }
    }

    return 0;
}
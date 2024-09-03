/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File			: minishell.c
   Compiler/System	: gcc/linux

********************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define NV 20			/* max number of command tokens */
#define NL 100			/* input buffer size */
char            line[NL];	/* command input buffer */
char            saved_line[NL];
int amount_of_jobs = 0; // keep track of the amount of jobs

typedef struct background_job {
  char command_name[NL]; // storing the command name - ensuring that its within the command input buffer size
  int job_id; // job id to print
  pid_t process_id; // process id to keep track of the process if its in the background
} background_job_t;

background_job_t background_jobs[30]; // limiting the number of jobs to 30

void handling_background_processes(int signal);

/*
	shell prompt
 */

void prompt(void)
{
  // fprintf(stdout, "\n msh> ");
  fflush(stdout);

}



int main(int argk, char *argv[], char *envp[]){
/* argk - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */


  int             frkRtnVal;	/* value returned by fork sys call */
  int             wait_pid;		/* value returned by wait */
  char           *v[NV];	/* array of pointers to command line tokens */
  char           *sep = " \t\n";/* command line token separators    */
  int             i;		/* parse index */
  int             background_process = 0; // if we have a background process

  /* prompt for and process one command line at a time  */

  while (1) {			/* do Forever */
    background_process = 0; // resetting background process after every loop
    prompt();
    fgets(line, NL, stdin);
    
    strcpy(saved_line, line);
    fflush(stdin);

    if (feof(stdin)) {		/* non-zero on EOF  */

      // fprintf(stderr, "EOF pid %d feof %d ferror %d\n", getpid(),
	    //   feof(stdin), ferror(stdin));
      exit(0);
    }
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000')
      continue;			/* to prompt */

    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++) {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL)
	break;
    }
    // handling signals for the background process - in case we need to terminate one
    struct sigaction background_process_signal_handler;
    background_process_signal_handler.sa_handler = handling_background_processes; // setting handler to our function
    background_process_signal_handler.sa_flags = SA_RESTART | SA_NOCLDSTOP; // restarting signal and not processing SIGCHILD
    sigemptyset(&background_process_signal_handler.sa_mask); // sa_mask can remain empty because we don't want to mask anything
    if(sigaction(SIGCHLD, &background_process_signal_handler, NULL) == -1){ // if handling child signals fails with our structure then we error out
      perror("sigaction");
      exit(EXIT_FAILURE);
    }

    
    // we now  want to check if we have a background process
    // if(v[i-1] != NULL){ // if we have a argument
    //   if(v[0][sizeof(v[0])/4] == '&'){ // if there is a & at the end of the command
    //     v[0][sizeof(v[0])/4] = '\0'; // setting the & to null so that it doesn't exist in the process
    //     background_process = 1;
    //   }
    // }
    // for some reason the above didnt work on gradescope so im gonna try something else
    if(v[i-1] != NULL){ // if there is another argument
      if(strcmp(v[i - 1], "&") == 0) { // if there is an & at the end of the command
        v[i - 1] = NULL;
        background_process = 1;
      }
    }
    

    // checking if we have the cd command
    if (strcmp(v[0], "cd") == 0) {
      if (v[1] !=  NULL) { // if so we want to make sure that the second argument exists
        if( chdir(v[1]) != 0){perror("cd");} // if the cd command fails then we error
      
      } else {
        fprintf(stderr, "cd: missing argument\n"); // if the second argument doesn't exist
      }
      continue;
    }

    /* assert i is number of tokens + 1 */

    /* fork a child process to exec the command in v[0] */
    handling_background_processes(0);
    switch (frkRtnVal = fork()) {
    case -1:			/* fork returns error to parent process */
      {
        // if the fork errored
        perror("fork");
	      break;
      }
    case 0:			/* code executed only by child process */
      {
	    
      if(execvp(v[0], v) == -1) {perror("execvp"); exit(EXIT_FAILURE);} // if the process failed
	
      }
    default:			/* code executed only by parent process */
      {
        if(background_process == 0){
        // if we do not have a background process we want to wait for the process to finish unless there is an error
        // we also want to ignore sigchld because that should only be used for the background process
          sigset_t previous_mask, current_mask;
          sigemptyset(&current_mask);
          sigaddset(&current_mask, SIGCHLD);
          sigprocmask(SIG_BLOCK, &current_mask, &previous_mask); // blocking the child signal

          wait_pid = wait(0);
          if(wait_pid == -1){perror("wait");} // if wait failed
          // amount_of_jobs++;
          sigprocmask(SIG_SETMASK, &previous_mask, NULL);
        }
        else if (background_process == 1){ // if we have a background process
        // we want to add it to the job list

        background_jobs[amount_of_jobs].job_id = amount_of_jobs + 1; // the current job should be added to the set at the end

        background_jobs[amount_of_jobs].process_id = frkRtnVal; // we now copy the process ID and command name to the job so we can keep track of it
        
        
        // making sure we copy the command string accross correctly
        char *cmd_end = strstr(saved_line, " &");
        if(cmd_end != NULL) {*(cmd_end) = '\0';} // removing the "&"
        
        strncpy(background_jobs[amount_of_jobs].command_name, saved_line, NL - 1);
        background_jobs[amount_of_jobs].command_name[NL-1] = '\0'; // null terminating

        fprintf(stdout, "[%d] %d\n", background_jobs[amount_of_jobs].job_id, frkRtnVal);

        amount_of_jobs++; // we increment the amount of jobs
        }
	    
	    break;
      }
    }				/* switch */
  }				/* while */
}				/* main */


void handling_background_processes(int signal){
  // function to handle and kill the background processes as necessary
  int process_status; // this will either be 0 or -1
  pid_t current_process_id; // process ID

  while((current_process_id = waitpid(-1, &process_status, WNOHANG)) > 0) // while we are waiting for the current process to finish - the background process is not hanging
  {
    // prompt();
    for (int j = 0; j < amount_of_jobs; j++) // for each background job
    {
      // if the job we are waiting on is the current process id
      if(current_process_id == background_jobs[j].process_id)
      {
        if(WIFSIGNALED(process_status)) // if the current process has been killed by a signal
        {
          fprintf(stderr, "[%d]  + terminated by signal %d   %s\n", background_jobs[j].job_id, WIFSIGNALED(process_status), background_jobs[j].command_name);
        } else if(WIFEXITED(process_status)) // if the process exited successfully
        {
          // fflush(stderr);
          fprintf(stdout, "[%d]+ Done %s\n", background_jobs[j].job_id, background_jobs[j].command_name);
          // fflush(stdout);
        }

        for (int i = j; i < amount_of_jobs - 1; i++)// removing the job from the list and moving all jobs down one place
        {
          background_jobs[i] = background_jobs[i+1];
        }
        amount_of_jobs--; // decrememtng the jobs count
        break;
      }
    }
    

  }
}

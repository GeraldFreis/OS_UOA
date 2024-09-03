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

// making a job struct to keep track of jobs
typedef struct job {
    pid_t processID;
    int job;
    char *command_name;
} job;

job jobs[NV];
int job_amnt;

/*
	shell prompt
 */

void prompt(void)
{
//   fprintf(stdout, "\n msh> ");
  fflush(stdout);

}

// function to kill / end background processes 
void background_process(int signal);



int main(int argk, char *argv[], char *envp[])
/* argk - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */

{
  int             frkRtnVal;	/* value returned by fork sys call */
  int             wpid;		/* value returned by wait */
  char           *v[NV];	/* array of pointers to command line tokens */
  char           *sep = " \t\n";/* command line token separators    */
  int             i;		/* parse index */


  /* prompt for and process one command line at a time  */

  while (1) {			/* do Forever */
    prompt();
    fgets(line, NL, stdin);
    fflush(stdin);

    if (feof(stdin)) {		/* non-zero on EOF  */

    //   fprintf(stderr, "EOF pid %d feof %d ferror %d\n", getpid(),
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
    /* assert i is number of tokens + 1 */
    if (strcmp(v[0], "cd") == 0) {
      if (v[1] !=  NULL) {
        if( chdir(v[1]) != 0){perror("cd");}
      
      } else {
        fprintf(stderr, "cd: missing argument\n");
      }
      continue;
    }
    struct sigaction sa;
    sa.sa_handler = background_process;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("msh: sigaction failed");
        exit(EXIT_FAILURE);
    }
    int background = 0;
    if (v[0] && (v[0][sizeof(v[0])/4] == '&')) 
    {
        background = 1;
        v[0][sizeof(v[0])/4] = '\0'; // Remove '&' from the command arguments
    }
    printf("%c\n", v[0][sizeof(v[0])/4]);
    /* fork a child process to exec the command in v[0] */
    switch (frkRtnVal = fork()) {
    case -1:            /* fork returns error to parent process */
      perror("msh: fork failed");
      break;

    case 0: {           /* code executed only by child process */
      if (execvp(v[0], v) == -1) {
        perror("msh: execvp failed");
        exit(EXIT_FAILURE); // Terminate the child process
      }
      break;
    }

    default: {          /* code executed only by parent process */
      if (!background) {
        wpid = waitpid(frkRtnVal, NULL, 0); // Wait for the specific child process
        if (wpid == -1) {
          perror("msh: waitpid failed");
        } else {
        //   printf("%s done\n", v[0]);
        continue;
        }
    } else {
                // Store background job details
        jobs[job_amnt].job = job_amnt+1; job_amnt++;
        jobs[job_amnt-1].processID = frkRtnVal;
        strncpy(jobs[job_amnt-1].command_name, v[0], NL);

        printf("[%d] %d\n", jobs[job_amnt-1].job, frkRtnVal);
    }
      break;
    }

    } /* switch */
  } /* while */
}

void background_process(int signal)
{
    pid_t process_ID;
    int process_status;
    while ( (process_ID = waitpid(-1, &process_status, WNOHANG) ) > 0 ) // while the background process is still going
    {
        for (int i = 0; i < job_amnt; i++) {
            if (jobs[i].processID == process_ID) {
                if (WIFEXITED(process_status)) {
                    printf("\n[%d]  + done    %s\n", jobs[i].job, jobs[i].command_name);
                } else if (WIFSIGNALED(process_status)) {
                    printf("\n[%d]  + terminated by signal %d    %s\n", jobs[i].job, WTERMSIG(process_status), jobs[i].command_name);
                }
                prompt(); // Re-print prompt after handling background process completion
                break;
            }
        }
        // reopening prompt
        prompt();
    }
}
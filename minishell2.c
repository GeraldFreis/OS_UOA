#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define NV 20			/* max number of command tokens */
#define NL 100			/* input buffer size */
char line[NL];	/* command input buffer */


/*
	shell prompt
 */

void prompt(void)
{
  fprintf(stdout, "\n msh> ");
  fflush(stdout);
};

int main(int argk, char *argv[], char *envp[])
{
  int frkRtnVal;	/* value returned by fork sys call */
  int wpid;		/* value returned by wait */
  char *v[NV];	/* array of pointers to command line tokens */
  char *sep = " \t\n"; /* command line token separators    */
  int i;		/* parse index */

  /* prompt for and process one command line at a time  */

  while (1) {			/* do Forever */
    prompt();
    fgets(line, NL, stdin);

    if (feof(stdin)) {		/* non-zero on EOF  */
      fprintf(stderr, "EOF pid %d feof %d ferror %d\n", getpid(),
	      feof(stdin), ferror(stdin));
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

    // Handle 'cd' command
    if (strcmp(v[0], "cd") == 0) {
      if (v[1] == NULL || chdir(v[1]) != 0) {
        perror("msh: cd failed");
      }
      continue;
    }

    // Check if the command should run in the background
    // printf("%s\n", v[0]);
    // printf("%c\n", v[0][sizeof(v[0])/4]);
    int background = 0;
    if (&v[0][sizeof(v[0])/4] != NULL && v[0][sizeof(v[0])/4] == '&') {
      background = 1;
      v[0][sizeof(v[0])/4] = '\0'; // Remove '&' from the command arguments
    }
    
    

    /* fork a child process to exec the command in v[0] */
    switch (frkRtnVal = fork()) {
    case -1:			/* fork returns error to parent process */
      perror("msh: fork failed");
      break;

    case 0: {			/* code executed only by child process */
      if (execvp(v[0], v) == -1) {
        perror("msh: execvp failed");
         // Terminate the child process
      }
      break;
    }

    default: {			/* code executed only by parent process */
      if (!background) {
        wpid = wait(0);
        if (wpid == -1) {
          perror("msh: wait failed");
        } else {
          printf("%s done\n", v[0]);
        }
      } else {
        printf("%s started in background with PID: %d\n", v[0], frkRtnVal);
        signal(SIGCHLD, SIG_IGN); // Automatically clean up finished background processes
      }
      break;
    }
    } /* switch */
  } /* while */
  return 0;
} /* main */

/* $begin shellmain */
#include "csapp.h"
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 

int main(int argc, char *argv[]) 
{
    char user_in[MAXLINE];
    int custom = 0;
    char cmdline[MAXLINE]; /* Command line */
    if (argc>1){
    	strcpy(user_in, argv[2]);
	custom=1;
    }

    if (signal(SIGINT, sigint_handler)==SIG_ERR){
    	exit(1);
    }
    while (1) {
	/* Read */
	if(custom==0){
		printf("sh257> ");                   
	}
	else{
		printf("%s>", user_in);
	}
	Fgets(cmdline, MAXLINE, stdin); 
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    } 
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)  
	return;   /* Ignore empty lines */

    if (!builtin_command(argv)) { 
        if ((pid = Fork()) == 0) {   /* Child runs user job */
            if (execvp(argv[0], argv) < 0) {
		printf("Execution failed (in fork)\n");
	        printf("%s: Command not found.\n", argv[0]);
                exit(1);
            }
        }
	
//	else if(pid!=0){
//		printf("Execution failed (in fork)\n");
	//	exit(1);
//	}
	/* Parent waits for foreground job to terminate */
	if (!bg) {
	    int status;
	    if (waitpid(pid, &status, 0) < 0){
		unix_error("waitfg: waitpid error");
	    }
	    if(WIFEXITED(status)){
        printf("Process exited with status code %d \n", WEXITSTATUS(status));
        }
        else{
            printf("Process exited with status code %d\n", WEXITSTATUS(status));
        }
	}
	else
	    printf("%d %s", pid, cmdline);
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "exit")){ /* quit command */
	exit_();
	return 1;
    }
    if (!strcmp(argv[0], "cd")){
    	cd_(argv);
	return 1;
    }
    if(!strcmp(argv[0], "ppid")){
    	ppid_();
	return 1;
    }
    if(!strcmp(argv[0], "pid")){
    	pid_();
	return 1;
    }
    if(!strcmp(argv[0], "quit")){
    	exit(0);
    }	   
    if(!strcmp(argv[0], "help")){
        help_();
	return 1;
    } 
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	return 1;
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    return bg;
}
/* $end parseline */

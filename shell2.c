//add internal commands like cd, help, exit.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FaroghShell:- "

int execute(char* arglist[]);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);
int internal_cmds(char** args);

int main(){
	char *cmdline;
	char** arglist;
	char* prompt = PROMPT; 
	while((cmdline = read_cmd(prompt,stdin)) != NULL){
		if((arglist = tokenize(cmdline)) != NULL){
			if(internal_cmds(arglist) == 0)
				execute(arglist);
			//need to free arglist
         		for(int j=0; j < MAXARGS+1; j++)
	         		free(arglist[j]);
         		free(arglist);
         		free(cmdline);
		}
	}
	printf("\n");
	return 0;
}


int execute(char* arglist[]){
	int status;
	int cpid = fork();
	switch(cpid){
		case -1:
			perror("fork failed");
			exit(1);
		case 0:
			execvp(arglist[0], arglist);
			perror("Command not found...");
			exit(1);
		default:
			waitpid(cpid, &status, 0);
			printf("child exited with status %d \n", status >> 8);
		return 0;
	}
}


char** tokenize(char* cmdline){
	//allocate memory
	char** arglist = (char**)malloc(sizeof(char*)* (MAXARGS+1));
	for(int j=0; j < MAXARGS+1; j++){
		arglist[j] = (char*)malloc(sizeof(char)* ARGLEN);
      		bzero(arglist[j],ARGLEN);
    	}
	if(cmdline[0] == '\0')//if user has entered nothing and pressed enter key
		return NULL;
	int argnum = 0; //slots used
	char*cp = cmdline; // pos in string
	char*start;
	int len;
	while(*cp != '\0'){
		while(*cp == ' ' || *cp == '\t') //skip leading spaces
			cp++;
		start = cp; //start of the word
		len = 1;
		//find the end of the word
		while(*++cp != '\0' && !(*cp ==' ' || *cp == '\t'))
			len++;
		strncpy(arglist[argnum], start, len);
		arglist[argnum][len] = '\0';
		argnum++;
	}
	arglist[argnum] = NULL;
	return arglist;
}      

char* read_cmd(char* prompt, FILE* fp){
	printf("%s", prompt);
	int c; //input character
	int pos = 0; //position of character in cmdline
	char* cmdline = (char*) malloc(sizeof(char)*MAX_LEN);
	while((c = getc(fp)) != EOF){
		if(c == '\n')
			break;
		cmdline[pos++] = c;
	}
	//these two lines are added, in case user press ctrl+d to exit the shell
	if(c == EOF && pos == 0) 
		return NULL;
	cmdline[pos] = '\0';
	return cmdline;
}

int internal_cmds(char** args){
	int NoOfIntCmds = 3, i, switchOwnArg = 0;
    	char* ListOfIntCmds[NoOfIntCmds];
  
    	ListOfIntCmds[0] = "exit";
    	ListOfIntCmds[1] = "cd";
    	ListOfIntCmds[2] = "help";
  
	for (i = 0; i < NoOfIntCmds; i++) {
		if (strcmp(args[0], ListOfIntCmds[i]) == 0) {
			switchOwnArg = i + 1;
			break;
		}
	}
  
	switch (switchOwnArg) {
		case 1:
			//printf("\nGoodbye\n");
			exit(0);
		case 2:
			chdir(args[1]);
			return 1;
		case 3:
			printf("Farogh's Shell\n");
			printf("Type program names and arguments, and hit enter.\n");
			printf("The following are built in:\n");
			printf("cd\thelp\texit\n");
			printf("Use the man command for information on other programs.\n");
			return 1;

		default:
 			break;
	}
	return 0;
}

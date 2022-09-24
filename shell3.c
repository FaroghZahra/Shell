//do input output redirection and functionality of pipe operator

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FaroghShell:- "

int execute(char* arglist[], char* cmdline);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);
int internal_cmds(char** args);
void executePiped(char** buf,int nr);
void executeRedirect(char** buf,int nr,int mode);
void tokenize_buffer(char** param,int *nr,char *buf,const char *c); //to tokenize on specific character
void removeWhiteSpace(char* buf);

int main(){
	
	char *cmdline;
	char** arglist;
	char* prompt = PROMPT; 
	while((cmdline = read_cmd(prompt,stdin)) != NULL){
		if((arglist = tokenize(cmdline)) != NULL){
			if(internal_cmds(arglist) == 0)
				execute(arglist, cmdline);
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


int execute(char* arglist[], char* cmdline){
	int status;
	int cpid = fork();
	int usingpipe = 0;
	int usingredirect = 0;
	int nr = 0;
	char** cmdline2 = arglist;
	switch(cpid){
		case -1:
			perror("fork failed");
			exit(1);
		case 0:
			for(int i=0; arglist[i] != NULL; ++i){
				//pipes
				if (strcmp(arglist[i],"|")==0){
					usingpipe = 1;
					tokenize_buffer(cmdline2,&nr,cmdline,"|");
					executePiped(cmdline2,nr);
					break;
				}
				//redirect input
				else if (strcmp(arglist[i],"<")==0){
					 int file = open(arglist[i + 1], O_RDONLY);
					 if (file == -1 || arglist[i+1]  == NULL) {
						printf("Invalid Command!\n");
						exit(1);	
					}
					dup2(file, 0);
                        		arglist[i] = NULL;
                        		arglist[i + 1] = NULL;
                        		break;
				}
				//output redirect
				else if (strcmp(arglist[i],">")==0){
					int file = open(arglist[i + 1], O_WRONLY | O_CREAT, 0644);
 					if (file == -1 || arglist[i+1]  == NULL) {
 						printf("Invalid Command!\n");
 						exit(1);
					}
					dup2(file, 1);
					arglist[i] = NULL;
 					arglist[i + 1] = NULL;
 					break;
				}
				
			}
			if(usingpipe != 1){
				execvp(arglist[0], arglist);
				perror("Command not found...");
				exit(1);
			}
			
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
			printf("\nGoodbye\n");
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


void executePiped(char** buf,int nr){
	if(nr>10) return;//can support up to 10 piped commands
	
	int fd[nr][2],i,pc;    
	char *argv[100];

	for(i=0;i<nr;i++){
		tokenize_buffer(argv,&pc,buf[i]," ");
		if(i!=nr-1){
			if(pipe(fd[i])<0){
				perror("pipe creating was not successfull\n");
				return;
			}
		}
		if(fork()==0){//child1
			if(i!=nr-1){
				dup2(fd[i][1],1);
				close(fd[i][0]);
				close(fd[i][1]);
			}

			if(i!=0){
				dup2(fd[i-1][0],0);
				close(fd[i-1][1]);
				close(fd[i-1][0]);
			}
			execvp(argv[0],argv);
			perror("invalid input ");
			exit(1);//in case exec is not successfull, exit
		}
		//parent
		if(i!=0){//second process
			close(fd[i-1][0]);
			close(fd[i-1][1]);
		}
		wait(NULL);
	}
	exit(0);
}

void tokenize_buffer(char** param,int *nr,char *buf,const char *c){
	char *token;
	token=strtok(buf,c);             
	int pc=-1;
	while(token){
		param[++pc]=malloc(sizeof(token)+1);
		strcpy(param[pc],token);
		removeWhiteSpace(param[pc]);
		token=strtok(NULL,c);
	}
	param[++pc]=NULL;
	*nr=pc;
}

void removeWhiteSpace(char* buf){
	if(buf[strlen(buf)-1]==' ' || buf[strlen(buf)-1]=='\n')
	buf[strlen(buf)-1]='\0';
	if(buf[0]==' ' || buf[0]=='\n') memmove(buf, buf+1, strlen(buf));
}

//1. Basic shell coded in C.
//2. add internal commands like cd, help, exit.
//3. do input output redirection and functionality of pipe operator(upto 10 pipes)
//4. added functionality to run multiple commands on single line separated by semi colons and able to execute commands in background.
//5. added functionality to maintain command history and accessing them using !n command. added internal(build-in) command of "history"
//6. added functionality of if else block; can use basic operators for comparision. PS:need to provide space on both sides of equation in square bracket, as in shell

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>


#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define MAX_HISTORY 50
#define PROMPT "FaroghShell:- "


char history[MAX_HISTORY][MAX_LEN];
int currenthistory = 0;
int execute(char* arglist[], char* cmdline);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);
int internal_cmds(char** args);
void executePiped(char** buf,int nr);
void morecommands(char** buf,int nr);
void tokenize_buffer(char** param,int *nr,char *buf,const char *c); //to tokenize on specific character
void removeWhiteSpace(char* buf);
void savehistory(char *command);
void displayhistory();
void handleif();
int handledigit(char **condition);
int handlestring(char **condition);

int main(){	
	char *cmdline;
	char** arglist;
	char* prompt = PROMPT; 
	while((cmdline = read_cmd(prompt,stdin)) != NULL){
		if((arglist = tokenize(cmdline)) != NULL){
			savehistory(cmdline);
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
	int more = 0;
	int nr = 0;
	char** cmdline2 = arglist;
	int background = 1;
	int size = 0;
	for(size=0; arglist[size] != NULL; ++size);
	if(strcmp(arglist[size - 1], "&")== 0){   // for run command in background
		background = 0;
		arglist[size - 1] = NULL;
	}
	switch(cpid){
		case -1:
			perror("fork failed");
			exit(1);
		case 0:
			//managing !n to execute nth command
			if(arglist[0][0]=='!' && isdigit(arglist[0][1])){
				char** newarglist;
				int num = arglist[0][1] - '0';
				if(currenthistory != 0 && num < MAX_HISTORY){
					strcpy(cmdline, history[num - 1]);
					newarglist = tokenize(cmdline);
					savehistory(cmdline);
					if(internal_cmds(newarglist) == 0)
						execute(newarglist, cmdline);
					exit(0);
					
				}
				else{
					printf("Wrong Command\n");
					return 0;
				}
				
			}
			if(strcmp(arglist[0],"if")==0){	//to handle if block
				handleif();
				exit(0);
			}
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
				//multiple commands
				if (strcmp(arglist[i],";")==0){
					more = 1;
					tokenize_buffer(cmdline2,&nr,cmdline,";");
					morecommands(cmdline2,nr);
					break;
				}
				
			}
			if(usingpipe != 1 && more != 1){
				execvp(arglist[0], arglist);
				perror("Command not found...");
				exit(1);
			}
			
		default:
			if(background == 1){
				waitpid(cpid, &status, 0);
				printf("child exited with status %d \n", status >> 8);
			}
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
	int NoOfIntCmds = 4, i, switchOwnArg = 0;
    	char* ListOfIntCmds[NoOfIntCmds];
  
    	ListOfIntCmds[0] = "exit";
    	ListOfIntCmds[1] = "cd";
    	ListOfIntCmds[2] = "help";
    	ListOfIntCmds[3] = "history";
  
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
		case 4:
			displayhistory();
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

void morecommands(char** buf,int nr){
	int status;
	int cpid, pc;
	char *argv[100];
	for(int i = 0; i < nr; i++){
		tokenize_buffer(argv,&pc,buf[i]," ");
		cpid = fork();
		switch(cpid){
		case -1:
			perror("fork failed");
			exit(1);
		case 0:
			execvp(argv[0], argv);
			perror("Command not found...");
			exit(1);
		default:
			wait(NULL);
		}
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

void savehistory(char *command){
	if(currenthistory != MAX_HISTORY){    
		strcpy(history[currenthistory++],command);
	}
	else if(currenthistory == MAX_HISTORY){
		for(int x=1;x<MAX_HISTORY;x++)
			strcpy(history[x-1],history[x]);
		strcpy(history[MAX_HISTORY-1],command);
	}
}

void displayhistory(){
	if(currenthistory !=0){
		for(int i=1;i<currenthistory+1;i++)
			printf("%d: %s \n",i,history[i-1]);
	}
	else 
		printf("No Commands in History 0\n");
}

void handleif(){
	char input[MAX_LEN];
	char ifblock[ARGLEN][MAX_LEN];
	int i = 0;
	while(i <= 10){
		printf(">");
		scanf("%[^\n]%*c", input);
		if(strcmp(input,"fi")==0){
			break;
		}
		strcpy(ifblock[i],input);
		i++;
	}
	if(i == 10)
		printf("use fi to end if else block\n");
	if(strcmp(ifblock[1],"then")!=0){
		printf("use if block in following format:\n");
		printf(">if\n>[ expression ]\n>then\n>command\n>else\n>command\n>fi\n");
		return;
	}
	else if(strcmp(ifblock[3],"else")!=0){
		printf("use if block in following format:\n");
		printf(">if\n>[ expression ]\n>then\n>command\n>else\n>command\n>fi\n");
		return;
	}
	char** condition;
	char** command;
	if((condition = tokenize(ifblock[0])) != NULL) { 
		if(isdigit(condition[1][0])){	//if given equation is comparision of digits
			if(handledigit(condition) == 1){
				if((command = tokenize(ifblock[2])) != NULL){
					if(internal_cmds(command) == 0)
						execute(command, ifblock[2]);
				}
			}
			else if(handledigit(condition) == 0){
				if((command = tokenize(ifblock[4])) != NULL){
					if(internal_cmds(command) == 0)
						execute(command, ifblock[4]);
				}
			}
			else
				printf("error\n");
		}
		else{
			if(handlestring(condition) == 1){	//if given equation is comparision of strings
				if((command = tokenize(ifblock[2])) != NULL){
					if(internal_cmds(command) == 0)
						execute(command, ifblock[2]);
				}
			}
			else if(handledigit(condition) == 0){
				if((command = tokenize(ifblock[4])) != NULL){
					if(internal_cmds(command) == 0)
						execute(command, ifblock[4]);
				}
			}
		}
	}
	else
		printf("give an evaluateable equation\n");
	for(int j=0; j < MAXARGS+1; j++)
		free(condition[j]);
	free(condition);
	for(int j=0; j < MAXARGS+1; j++)
		free(command[j]);
	free(command);	
}

int handledigit(char **condition){
	int num1 = atoi(condition[1]);
	int num2 = atoi(condition[3]);
	if((strcmp(condition[2],"-eq") == 0) || (strcmp(condition[2],"==") == 0)){
		if(num1 == num2)
			return 1;
		else
			return 0;
	}
	else if((strcmp(condition[2],"-ne") == 0) || (strcmp(condition[2],"!=") == 0)){
		if(num1 != num2)
			return 1;
		else
			return 0;
	}
	else if((strcmp(condition[2],"-gt") == 0) || (strcmp(condition[2],">") == 0)){
		if(num1 > num2)
			return 1;
		else
			return 0;
	}
	else if((strcmp(condition[2],"-ge") == 0) || (strcmp(condition[2],">=") == 0)){
		if(num1 >= num2)
			return 1;
		else
			return 0;
	}
	else if((strcmp(condition[2],"-lt") == 0) || (strcmp(condition[2],"<") == 0)){
		if(num1 < num2)
			return 1;
		else
			return 0;
	}
	else if((strcmp(condition[2],"-le") == 0) || (strcmp(condition[2],"<=") == 0)){
		if(num1 <= num2)
			return 1;
		else
			return 0;
	}
	else{
		printf("Use one of the following comparision operators: -eq or ==\n-gt or >\n-ge or >=\n-lt or <\n-le or <=\n-ne or !=\n");
		return 2; 
	}
}

int handlestring(char **condition){
	char *str1 = condition[1];
	char *str2 = condition[3];
	if(strcmp(condition[2],"==") == 0){
		if(strcmp(str1,str2)==0)
			return 1;
		else
			return 0;
	}
	else if(strcmp(condition[2],"!=") == 0){
		if(strcmp(str1,str2)!=0)
			return 1;
		else
			return 0;
	}
	else{
		printf("Use one of the following comparision operators for string comparing: == or !=\n");
		return 2; 
	}
}


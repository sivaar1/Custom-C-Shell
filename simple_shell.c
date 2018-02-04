#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>

#include "utils.h"

#define MAXBUFFLEN 100

//Gets the escaped section of a string, using unquoted spaces as the delimiter
char* getEscStr(char* str){
	char *ptr;
	int spaceIndex = first_unquoted_space(str);
	if(spaceIndex == -1)
		spaceIndex = strlen(str);
	char temp[spaceIndex + 1];
	
	int i;	
	for(i = 0; i < spaceIndex; i++)
		temp[i] = str[i];
	temp[spaceIndex] = '\0';
	
	ptr = strdup(temp);
	return ptr;
}

//Duplicates current process then sends duplicate to execute the program given by file path.
void runProcess(char *myArgs[]){
	pid_t pid = fork();
	if(pid == 0){
		//Checks for error
		if(execv(myArgs[0], myArgs) == -1){
			fprintf(stderr, "Error: Invalid Input!\n");
			_Exit(3);		
		}
	}	
	wait(NULL);
}

//Gets the user input from stdin using fgetc.
void getCmd(char* myCmd){
	int tempIndex = 0;
	int myChar = fgetc(stdin);
	while(myChar != '\n'){
		myCmd[tempIndex] = myChar;
		myChar = fgetc(stdin);
		tempIndex++;	
	}
	myCmd[tempIndex] = '\0';		
}


//Counts the number of arguments to initialize the char* array.
int getNumArgs(char* myCmd){
	int count = 0;
	char *tempStr = strdup(myCmd);
	char *tempPtr = strtok(tempStr, " ");
	while(tempPtr != NULL){
		count++;
		tempPtr = strtok(NULL, " ");	
	}
	count++;
	free(tempStr);
	return count;
}

//Splits the user arguments by unquoted spaces and puts them into an array
char* splitArgs(char* myCmd, char *myArgs[]){
	char *tempStr = strdup(myCmd);
	char *ptr = tempStr;
	char *tempEsc;
	int counter = 0;
	int spaceIndex = 0;
	do{
		tempStr = tempStr + spaceIndex;
		//Getting rid of whitespace
		while(tempStr[0] == ' ')
			tempStr++;
		//Reached the end of the user cmd
		if(tempStr[0] == '\0')
			break;
		tempEsc = getEscStr(tempStr);
		spaceIndex = first_unquoted_space(tempStr);
		myArgs[counter] = unescape(tempEsc, stderr);
		counter++;
		free(tempEsc);
	}while(spaceIndex != -1);
	myArgs[counter] = NULL;
	return ptr;
}


//Checks if the first argument the user provided is equal to "exit"
int checkExit(char* str){
	//Checking if no arguments
	char *newStr = str;
	while(newStr[0] == ' ')
		newStr++;
	if(newStr[0] == '\0')
		return 0;

	int isExit = 0;
	char *tempStr = strdup(str);
	char *userStr = strtok(tempStr, " ");
	if(strcmp(userStr, "exit") == 0)
		isExit = 1;
	free(tempStr);
	return isExit;
}

//Runs a certain command based on the first argument (Ex: cd, getenv, setenv, etc.)
//If no recognizeable argument, assumes it is a file path to fork exec
void runCmd(char* str){
	char *newStr = str;
	//Removes whitespace at the beginning
	while(newStr[0] == ' ')
		newStr++;
	
	if(newStr[0] == '\0')
		return;	

	//Gets first argument from user
	char *tempArg = strdup(str);
	char *firstArg = strtok(tempArg, " ");

	//echo
	if(strcmp(firstArg, "echo") == 0){
		//Gets the string after echo
		newStr = newStr + strlen(firstArg);
		while(newStr[0] == ' ')
			newStr++;
		char *unescStr = unescape(newStr, stderr);
		fprintf(stdout, "%s\n", unescStr);
		free(unescStr);	
	}

	//chdir or cd
	else if(strcmp(firstArg, "chdir") == 0 || strcmp(firstArg, "cd") == 0){
		//Checks if argument is provided
		char *tempStr = strtok(NULL, " ");
		if(tempStr != NULL){
			//Gets argument after chdir/cd
			newStr = newStr + strlen(firstArg);
			while(newStr[0] == ' ')
				newStr++;
			char *escStr = getEscStr(newStr);
			
			tempStr = unescape(escStr, stderr);
			if(chdir(tempStr) == -1)
				fprintf(stderr, "Error: Invalid Path!\n");
			else{	
				//Getting current path
				char currentPath[PATH_MAX + 1];
				char *pathPtr;
				pathPtr = getcwd(currentPath, PATH_MAX);
				setenv("PWD", pathPtr, 1);
			}
			free(escStr);
			free(tempStr);
		}
		//Change to HOME directory if nothing given
		else{
			if(chdir(getenv("HOME")) == -1)
				fprintf(stderr, "Error: HOME has not been set or is an invalid path!\n");
			else
				setenv("PWD", getenv("HOME"), 1);
		}
	}

	//getenv
	else if(strcmp(firstArg, "getenv") == 0){
		//Gets the string after getenv
		char *tempVar = strtok(NULL, " ");
		//Makes sure only one argument is given 
		if(strtok(NULL, " ") == NULL){
			if(tempVar != NULL){
				int checkEqual = 0;
				int i;
				//Checks for equal symbol in variable name
				for(i = 0; i < strlen(tempVar); i++){
					if(tempVar[i] == '=')
						checkEqual = 1;
				}

				if(checkEqual)
					fprintf(stderr, "Error: '=' is prohibited in getenv!");
				else{
					char *varVal = getenv(tempVar);
					if(varVal != NULL)
						fprintf(stdout, "%s", varVal);
				}
			}
		}	
		else
			fprintf(stderr, "Error: Too many arguments given!");	
		fprintf(stdout, "\n");
	}
	
	//setenv
	else if(strcmp(firstArg, "setenv") == 0){
		//Gets the string after echo
		newStr = newStr + strlen(firstArg);
		while(newStr[0] == ' ')
			newStr++;
		
		//Checks for '=' in next arg
		char *envArg = strtok(NULL, " ");
		if(envArg != NULL){
			int isEqual = 0;
			int i;
			for(i = 0; i < strlen(envArg); i++){
				if(envArg[i] == '=')
					isEqual = 1;
			}
			if(isEqual){
				while(newStr[0] != '=')
					newStr++;
				newStr++;
				
				//Get the value and unescape it
				char *escStr = getEscStr(newStr);
				char *envVal = unescape(escStr, stderr);
				//Get variable name
				char *envName = strtok(envArg, "=");			

				setenv(envName, envVal, 1);
				
				free(escStr);
				free(envVal);
			}
			else
				fprintf(stderr, "Error: No value given for the environment variable!\n");
		}
		else
			fprintf(stderr, "Error: No argument given to setenv!\n");
	}
	
	else{
		//Initializing char* array for execv
		char *userArgs[getNumArgs(str)];
		char *temp= splitArgs(str, userArgs);
		runProcess(userArgs);
		free(temp);
		int counter = 0;
		while(userArgs[counter] != NULL){
			free(userArgs[counter]);
			counter++;		
		}
	}
	free(tempArg);
}

int main(){
	int checkAgain = 1;
	char *userCmd;

	while(checkAgain){
		userCmd = (char*)malloc(MAXBUFFLEN + PATH_MAX);
		fprintf(stdout, "> ");
		getCmd(userCmd);

		if(checkExit(userCmd))
			checkAgain = 0;
		else
			runCmd(userCmd);	
	
		free(userCmd);		
	}
	
	return 0;
}

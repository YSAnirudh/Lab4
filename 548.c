#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> 
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

char** input(char* line) {
	char** args = (char**)malloc(sizeof(char*) * 64);
	char* seperator = " ";
	char* buffer;
	int i = 0;
	
	buffer = strtok(line, seperator);
	while (buffer != NULL) {
		args[i] = buffer;
		i++;
		
		buffer = strtok(NULL, seperator);
	}
	args[i] = NULL;
	return args;
}

int execute(char** args) {
	if (args == NULL) {
		printf("Invalid Command\n");
		return -1;
	}
	
	pid_t rc = fork();
	
	if (rc < 0) {
		printf("Fork Failed\n");
		exit(1);
	} else if (rc == 0) {
		//Child Process
		if (execvp(*args, args) < 0) {
			printf("Exec Failed\n");
			return -1;
		}
	} else {
		//Parent Process
		wait(NULL);
	}
	return 0;
}

void toLowerCase(char* line) {
	for (int i = 0; i < strlen(line); i++) {
		if (line[i] >= 65 && line[i] <= 90) {
			line[i] += 32;
		}
	}
}

typedef struct Command {
	int index;
	char* shortForm;
	char* fullForm;
	struct Command* next;
} Command;

void historyBrief(Command* commands) {
	if (commands == NULL) return;
	Command* last;
	last = commands;
	
	while (last != NULL) {
		printf("%d) %s\n", last->index, last->shortForm);
		last = last->next;
	}
	return;
}

char* findCommand(Command* commands, char* line) {
	if (commands == NULL) return NULL;
	Command* last;
	last = commands;
	int index = 0;
	while (last != NULL) {
		if (strcmp(line, last->shortForm) == 0 || strcmp(line, last->fullForm) == 0) {
			return line;
		} else {
			index++;
			last = last->next;
		}
	}
	return (last != NULL) ? line : NULL;
}

char* getFirstString(char* line) {
	char* seperator = " ";
	char* buffer;
	char* context;
	int inputLength = strlen(line);
	char *inputCopy = (char*) calloc(inputLength + 1, sizeof(char));
	strncpy(inputCopy, line, inputLength);

	buffer = strtok_r(inputCopy, seperator, &context);

	return buffer;
}

void insertCommand(Command** commands, char* line) {
	if (findCommand(*commands, line) != NULL) {
		printf("Command Already Defined.\n");
		printf("To look at all the commands use 'history brief' or 'history full'.\n");
	} else {
		Command* node = (Command*)malloc(sizeof(Command));
		printf("%s\n", line);
		node->fullForm = line;
		node->shortForm = getFirstString(line);
		node->next = NULL;
		Command* last = *commands;
		if (last == NULL) {
			node->index = 1;
			*commands = node;
			
			printf("Command inserted.\n");
			return;
		}
		int i = 1;
		while(last->next != NULL) {
			last = last->next;
			i++;
		}
		node->index = i;
		last->next = node;
		printf("Command inserted.\n");
		return;
	}
	return;
}

int main() {
	char** args;
	char* line;
	Command* commands =NULL;
	while(1) {
		line = readline("MyShell> ");
		toLowerCase(line);
		if (strcmp(line, "history brief") == 0) {
			if (commands == NULL) {
				printf("No predefined commands found\n");
				printf("To Insert commands use 'insert'.\n");
				continue;
			} else {
				historyBrief(commands);
			}
		}
		else if (strcmp(line, "insert") == 0) {
			char* res;
			res = readline("Enter the command you want to execute and insert: ");
			insertCommand(&commands, res);
		}
		else if (strcmp(line, "stop") == 0) {
			printf("Exiting....\n");
			exit(0);
		}
		else {
			if (findCommand(commands, line) != NULL) {
				args = input(line);
				execute(args);
			} else {
				printf("Command not found\n");
				printf("To Insert commands use 'insert'.\n");
				printf("To look at all the commands use 'history brief' or 'history full'.\n");
			}
			
		}
	}
	free(commands);
	return 0;
}
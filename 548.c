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

const int ARG_SIZE  = 64; //the size of args array for exec
const int ARGS_SIZE = 1024; //the size of each argument

//struct to store all the commands
typedef struct Command {
	int index; //index of the command
	char* shortForm; //for history brief
	char* fullForm; //for history full
	struct Command* next; //next Command
} Command;
char** input(char*);
int execute(char**);
void toLowerCase(char*);
char* substring(char*, int, int);
void insertSubstring(char*, char*, int);
void historyBrief(Command*);
void historyFull(Command*);
char* findCommand(Command*, char*);
char* findCommandByNum(Command*, int);
char* getFirstString(char*);
char* removeFirstWord(char*);
int inputFromCmd(char*);
void execStored(char**);
char** execFile(FILE*);
void insertIntoList(Command**, char*);
void insert(Command**, char**);
void execAllFiles(int, char**, Command**);
void shellInit();
int gameLoop(Command**);

int main(int argc, char* argv[]) {
	if (argc <= 0) {
		printf("please check arguments.\n");
		return 0;
	} else if (argc == 1) {
		printf("Please enter atleast one file.\n");
		return 0;
	}
	Command* commands =NULL;
	char** args;
	char line[ARGS_SIZE];
	execAllFiles(argc, argv, &commands);
	
	printf("\n");
	shellInit();
	printf("\n");
	
	gameLoop(&commands);
	
	free(args);
	free(commands);
	return 0;
}

//main loop to read inputs and execute. The shell loop
int gameLoop(Command** comm) {
	char** args;
	
	Command* commands;
	commands = *comm;
	while(1) {
		args = NULL;
		
		int fd[2];
		if (pipe (fd)) {
			fprintf (stderr, "Pipe failed.\n");
			return EXIT_FAILURE;
		}
		//fork for reading input
		int a = -1;
		int rc = fork();
		if (rc < 0) {
			fprintf (stderr, "Fork failed.\n");
			return EXIT_FAILURE;
		} else if (rc == 0) {
				char line[ARGS_SIZE];
				char* buffer;
			buffer = readline("Enter your command>");
			if (strlen(buffer) != 0) {
				if (buffer[0] == ' ') {
					printf("Don't type spaces before your Command\n");
					continue;
				}
				add_history(buffer);
				strcpy(line, buffer);
				a = 0;
			} else {
				continue;
			}
			toLowerCase(line);
			close(fd[0]); //closing read side
			write(fd[1], line, sizeof(line)); //writing into write side
			exit(0);
		} else {
			// parent process to execute the inputs
			wait(NULL);
			char line[ARGS_SIZE];
			close(fd[1]); //closing write side
			read(fd[0], line, sizeof(line)); //writing into read side
			args = input(line);
			toLowerCase(args[0]);
			
			if (strcmp(line, "history brief") == 0) {
				if (commands == NULL) {
					printf("No predefined commands found\n");
					printf("To Insert commands use 'insert'.\n");
					continue;
				} else {
					historyBrief(commands);
				}
			}
			else if (strcmp(line, "history full") == 0) {
				if (commands == NULL) {
					printf("No predefined commands found\n");
					printf("To Insert commands use 'insert'.\n");
					continue;
				} else {
					historyFull(commands);
				}
			}
			else if (strcmp(args[0], "insert") == 0) {
				
				if (args[1] == NULL) {
					printf("Command Not Found.\n");
					printf("Type Command after 'insert' to insert\n");
					continue;
				}
				printf("Will not check if command can execute.\n");
				printf("Exec might fail for an invalid command.\n");

				char* res;
				res = removeFirstWord(line);
				
				char** temp = input(res);
				
				if (strcmp(temp[0], "ping") == 0) {
					if (temp[1] != NULL) {
						if (strcmp(temp[1], "-c") != 0) {
							insertSubstring(res, "-c 5 ", 6);
						}
					}
				}
				if (findCommand(commands, res) == NULL) { //insert if not found
					printf("Inserting.. %s\n", res);
					insertIntoList(&commands, res);
				} else { //else print command not found
					printf("Command already present\n");
					printf("To Execute commands use 'exec <COMMAND_NAME>' or 'exec <COMMAND_NUM>'.\n");
					printf("To look at all the commands use 'history brief' or 'history full'.\n");
				}
			}
			else if (strcmp(args[0], "exec") == 0){
				char* res;
				res = removeFirstWord(line);
				
				char** temp = input(res);
				
				if (strcmp(temp[0], "ping") == 0) {
					if (temp[1] != NULL) {
						if (strcmp(temp[1], "-c") != 0) {
							insertSubstring(res, "-c 5 ", 6);
						}
					}
				}
				int x = atoi(res);
				if (x != 0) { //to execute if numbers are given
					if (findCommandByNum(commands, x) != NULL) {
						printf("Executing Command %d\n", x);
						res = findCommandByNum(commands, x);
						args = input(res);
						execute(args);
					}
					else {
						printf("Command not found\n");
						printf("To Insert commands use 'insert'.\n");
						printf("To look at all the commands use 'history brief' or 'history full'.\n");
					}
				}
				else if (findCommand(commands, res) != NULL) { //to execute for given commands
					printf("Executing.. %s\n", res);
					res = findCommand(commands, res);
					args = input(res);
					execute(args);
				} else { // if command not found
					printf("Command not found\n");
					printf("To Insert commands use 'insert'.\n");
					printf("To look at all the commands use 'history brief' or 'history full'.\n");
				}
			}
			else if (strcmp(line, "stop") == 0) {
				printf("Exiting....\n");
				return 0;
			}
			else {
				printf("Command not found\n");
				printf("Use 'exec' before commands to execute\n");
				printf("Use 'insert' before commands to insert\n");
			}
		}
	}
	return 0;
}

//function to tokenize the given string
char** input(char* line) {
	char** args = (char**)malloc(sizeof(char*) * ARG_SIZE);
	char* seperator = " ";
	char* buffer;
	int i = 0;
	int length;
	int inputLength = strlen(line);
	char *inputCopy = (char*) calloc(inputLength + 1, sizeof(char));
	strncpy(inputCopy, line, inputLength);
	
	buffer = strtok(inputCopy, seperator);
	while (buffer != NULL) {
		args[i] = buffer;
		buffer = strtok(NULL, " ");
		length = strlen(args[i]);
		if(args[i][length - 1] == '\n' || args[i][length-1] == ' ') {
			args[i][length - 2] = '\0';
		}
		i++;
	}
	args[i] = NULL;
	return args;
}

//function to execute the given tokenized string
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
			printf("Check if it is a proper command\n");
			exit(0);
		}
	} else {
		//Parent Process
		wait(NULL);
		return 0;
	}
	return 0;
}

//function to convert characters into lower case
void toLowerCase(char* line) {
	for (int i = 0; i < strlen(line); i++) {
		if (line[i] >= 65 && line[i] <= 90) {
			line[i] += 32;
		}
	}
}

// start
//functions to add a string in the middle, used for ping command to only execute 5 times
char *substring(char *string, int position, int length) {
   char *pointer;
   int c;
 
   pointer = malloc(length+1);
   
   if( pointer == NULL )
       exit(EXIT_FAILURE);
 
   for( c = 0 ; c < length ; c++ )
      *(pointer+c) = *((string+position-1)+c);      
       
   *(pointer+c) = '\0';
 
   return pointer;
}

void insertSubstring(char *a, char *b, int position) {
   char *f, *e;
   int length;
   
   length = strlen(a);
   
   f = substring(a, 1, position - 1 );      
   e = substring(a, position, length-position+1);

   strcpy(a, "");
   strcat(a, f);
   free(f);
   strcat(a, b);
   strcat(a, e);
   free(e);
}
// end

//to print history brief
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

//to print history full
void historyFull(Command* commands) {
	if (commands == NULL) return;
	Command* last;
	last = commands;
	
	while (last != NULL) {
		printf("%d) %s\n", last->index, last->fullForm);
		last = last->next;
	}
	return;
}

//to find if the given command(String) is present in the linked list command
char* findCommand(Command* commands, char* line) {
	if (commands == NULL) return NULL;
	Command* last;
	last = commands;
	int index = 0;
	while (last != NULL) {
		if (strcmp(line, last->shortForm) == 0 || strcmp(line, last->fullForm) == 0) {
			return last->fullForm;
		} else {
			index++;
			last = last->next;
		}
	}
	return (last != NULL) ? last->fullForm : NULL;
}

//to find if the given command(Int) is present in the linked list command
char* findCommandByNum(Command* commands, int x) {
	if (commands == NULL) return NULL;
	Command* last;
	last = commands;
	int index = 0;
	while (last != NULL) {
		if (last->index == x) {
			return last->fullForm;
		} else {
			index++;
			last = last->next;
		}
	}
	return (last != NULL) ? last->fullForm : NULL;
}

//getting the first word of the string
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

//removing the first word of the given string
char* removeFirstWord(char* line) {
	char* res = (char*)malloc(sizeof(char) * strlen(line));
	int i = 0, index;
	int t = 0;
	while(line[i] != '\0') {
		if (line[i] == ' ' && !t) {
			t = 1;
			index = i + 1;
		}
		i++;
		if (t == 1) {
			res[i - index] = line[i];
		}
		
	}
	res[i] = '\0';
	return res;
}

//function to read from the command line
int inputFromCmd(char* line) {
	char* buffer;
	buffer = readline("Enter your command>");
	if (strlen(buffer) != 0) {
		if (buffer[0] == ' ') {
			printf("Don't type spaces before your Command\n");
			return 1;
		}
		add_history(buffer);
		strcpy(line, buffer);
		return 0;
	} else {
		
		return 1;
	}
}

//start
//functions to execute the commands in a file by first storing the commands in a char* array
void execStored(char** cd) {
	for (int i = 0; i < sizeof(cd); i++) {
		char* args[ARG_SIZE];
		int length;
		int j = 0;
		char* seperator = " ";
		char* buffer = strtok(cd[i], seperator);
		while (buffer != NULL) {
			args[j] = buffer;
			buffer = strtok(NULL, seperator);
			length = strlen(args[j]);
			if(args[j][length - 1] == '\n' || args[j][length-1] == ' ') {
				args[j][length - 2] = '\0';
			}
			j++;
		}
		args[j] = NULL;
		
		execute(args);
		//free(args);
	}
}

char** execFile(FILE* file) {
	if (file == NULL) {
		printf("File is NULL\n");
		exit(1);
	}
	
	char* buff = NULL;
	char** temp = (char**) malloc(sizeof(char*) * ARG_SIZE);
	size_t len = 0;
	ssize_t read;
	int i = 0;
	while ((read = getline(&buff, &len, file)) != -1) {
		temp[i] = (char*)malloc(sizeof(char) * ARGS_SIZE);
		strcpy(temp[i], buff);
		i++;
	}
	char** res = (char**)malloc(sizeof(char*) * sizeof(temp));
	i =0;
	while (temp[i] != NULL) {
		res[i] = (char*) malloc(sizeof(char) * ARGS_SIZE);
		char** residue = input(temp[i]);
			
		if (strcmp(residue[0], "ping") == 0) {
			insertSubstring(temp[i], "-c 5 ", 6);
		}
		printf("%s\n", temp[i]);
		strcpy(res[i], temp[i]);
		int length = strlen(res[i]);
		if(res[i][length - 1] == '\n' || res[i][length-1] == ' ') {
			res[i][length - 2] = '\0';
		}
		//printf("%s\n", res[i]);
		i++;
	}
	i = 0;
	execStored(temp);
	return res;
}
//end

//function to insert the command(String) to the linked list
void insertIntoList(Command** commands, char* line) {
	if (findCommand(*commands, line) != NULL) {
		printf("Command Already Defined.\n");
		printf("To look at all the commands use 'history brief' or 'history full'.\n");
		return;
	}
	Command* node = (Command*)malloc(sizeof(Command));
	//printf("%s\n", line);
	node->fullForm = line;
	node->shortForm = getFirstString(line);
	node->next = NULL;
	Command* last = *commands;
	if (last == NULL) {
		node->index = 1;
		*commands = node;
		
		//printf("Command inserted.\n");
		//free(args);
		return;
	}
	int i = 2;
	while(last->next != NULL) {
		last = last->next;
		i++;
	}
	node->index = i;
	last->next = node;
	//printf("Command inserted.\n");
	//free(args);
	return;
}

//function to insert all command from a given char* array
void insert(Command** commands, char** temp) {
	int i = 0;
	while (temp[i] != NULL) {
		
		insertIntoList(commands, temp[i]);
		i++;
	}
}

//function to call execFile() for all files
void execAllFiles(int argc, char* argv[], Command** commands) {
	char* fileName;
	for (int i = 1; i < argc; i++) {
		fileName = argv[i];
		char* ext = strrchr(argv[i], '.');
		if (!ext) {
			printf("File No. %d Not Valid\nPlease enter a text file.\n", i);
			continue;
		} else {
			if (strcmp(ext, ".txt") != 0) {
				printf("File No. %d Not Valid\nPlease enter a text file.\n", i);
				continue;
			}
		}
		FILE* file;
		file = fopen(fileName, "r");
		
		if (!file) {
			fprintf(stderr, "File not opening.\nPlease check if file is present\n");
			continue;
		}
		printf("Executing Commands in %s: \n", fileName);
		char** temp = execFile(file);
		insert(commands, temp);
		
		printf("\n");
	}
	return;
}

//initialization printf statements for the shell
void shellInit() {
	printf("Files added, if files were not able to open, please check your files and file format.\n");
	printf("Use 'insert' to insert extra commands into history.\n");
	printf("To look at all the commands use 'history brief' or 'history full'.\n");
	printf("Shell Starting Up....\n");
}

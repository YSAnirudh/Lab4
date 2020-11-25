#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
/* Read characters from the pipe and echo them to stdout. */
void read_from_pipe (int file) {
/* the variable file is of integer type and
it points a memory location that can be written 
into*/

	FILE *stream;
	int c;
	stream = fdopen (file, "r");
	while ((c = fgetc (stream)) != EOF)
		putchar (c);
	fclose (stream);
}

/* Write some random text to the pipe. */

void
write_to_pipe (int file) {
/* the variable file is of integer type and
it points a memory location that can be written 
into*/

	FILE *stream;
	char* line;
	stream = fdopen (file, "w");
	fprintf (stdin, "%s", line);
	fprintf (stream, "goodbye, world!\n");
	fclose (stream);
}

int inputFromCmd(char* line) {
	char* buffer;
	buffer = readline("Shell> ");
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

int main () {
	pid_t pid;
	int fd[2];

	if (pipe (fd)) {
		fprintf (stderr, "Pipe failed.\n");
		return EXIT_FAILURE;
	}
	pid = fork ();
	if (pid == (pid_t) 0) {
		char line[1000];
		if (inputFromCmd(line)) {
			exit(0);
		}
		close (fd[0]);
		write(fd[1], line, sizeof(line));
		
		return EXIT_SUCCESS;
	}
	else if (pid < (pid_t) 0) {
		fprintf (stderr, "Fork failed.\n");
		return EXIT_FAILURE;
	}
	else {
		wait(NULL);
		char line[1000];
		close (fd[1]);
		read(fd[0],line, sizeof(line));
		printf("%s\n", line);
		
		return EXIT_SUCCESS;
	}
}
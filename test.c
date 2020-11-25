#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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

int main () {
	pid_t pid;
	int mypipe[2];

	if (pipe (mypipe)) {
		fprintf (stderr, "Pipe failed.\n");
		return EXIT_FAILURE;
	}
	pid = fork ();
	if (pid == (pid_t) 0) {
		char* arr;

		close (mypipe[1]);
		int N = read(0,arr, sizeof(arr)); 
		for (int i = 0; i < N;i++) {
			printf("%s\n", arr);
		}
		return EXIT_SUCCESS;
	}
	else if (pid < (pid_t) 0) {
		fprintf (stderr, "Fork failed.\n");
		return EXIT_FAILURE;
	}
	else {
		int arr[] = {10, 32,12};
		close (mypipe[0]);
		write(1, arr, sizeof(arr));
		
		return EXIT_SUCCESS;
	}
}
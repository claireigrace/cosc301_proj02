#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>


void replace_hash (char arr []) {
	for (int i = 0; i<strlen(arr); i++) {
		if (arr[i] == '#') {
			arr[i] = '\0';
		}
	}
}


bool check_exit (char arr[]) {

	char exit[] = "exit";
	if (strcmp(arr, exit) ==0) {
		return true;
	}
	return false;
}


char** tokenify(char input []) {
	char *ptr_input = input;
	char *pstring = strdup(ptr_input); //pointer to copy of input 
	char *ptoken; 
	ptoken = strtok(pstring, "; \t\n"); // ptoken now points to first token
	char **array = malloc(sizeof(char*)*((strlen(ptr_input)/2)+2));
	
	// count the number of tokens
	int i = 0;
	while (ptoken != NULL) {
		char *ptrtok = strdup(ptoken);
		array[i] = ptrtok;
		ptoken = strtok(NULL, "; \t\n"); // get each next token
		i++;	
	}
	// set last index in array to NULL
	array[i]=NULL;
	// free 
	free(pstring);
	return array;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int main(int argc, char **argv) {

	int mode = 0; // 0 for sequential, 1 for parallel
	bool done = true;


	char input[1024];
	printf("This is the prompt: ");
	fflush(stdout);

	while (fgets(input, 1024, stdin)!= NULL) {
	
		input[strlen(input)-1] = '\0';

		// exit if input only says EOF or exit
		if (check_exit(input)==true) { exit(0);}

		// ignore comments; replace all # with null termination
		replace_hash(input);

		// parse the command and organize the separate commands in memory
		char **ptrs = tokenify(input); // array of char pointers to each different token (commands will always start with /bin)


		// NOW: 
			// check for exit command 
			// handle mode
			// create new process for each new command
	}


	// fgets returned NULL, so check for EOF
	if (feof(stdin)) {
		exit(0); // exit if there's EOF 
	}
	
	free(ptrs);

    return 0;
}


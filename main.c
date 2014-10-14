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


bool check_exit (const char * arr) {
	const char *exit = "exit";
	if (strstr(arr, exit) !=NULL) {
		return true;
	}
	return false;
}


char** tokenify(char input []) {
	char *ptr_input = input;
	char *pstring = strdup(ptr_input); //pointer to copy of input 
	char *ptoken; 
	ptoken = strtok(pstring, ";"); // ptoken now points to first token
	char **array = malloc(sizeof(char*)*((strlen(ptr_input)/2)+2));
	
	// count the number of tokens
	int i = 0;
	while (ptoken != NULL) {
		char *ptrtok = strdup(ptoken);
		array[i] = ptrtok;
		ptoken = strtok(NULL, ";"); // get each next token
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
	int exit = 0; // specifies if we should exit at end of parallel execution 
	int new_mode = 0; // 0 means no change, 1 means to S, 2 means to P
	char ** ptrs;


	char input[1024];
	printf("This is the prompt: ");
	fflush(stdout);

	while (fgets(input, 1024, stdin)!= NULL) {
	
		input[strlen(input)-1] = '\0';

		// exit if input only says EOF or exit
		if (check_exit(input)==true) { exit();} // must pass const char * here

		// ignore comments; replace all # with null termination
		replace_hash(input);

		// parse the command and organize the separate commands in memory
		ptrs = tokenify(input); // array of char pointers to each different token (commands will always start with /bin)



		if (mode ==0) {
			const char * mode_str="mode";
			const char * par="p";
			const char * seq= "s";
			char** temp= ptrs;
			int i =0;
			
			pid_t child_pid;
			while (temp[i]!=NULL) {
				if ( strstr(temp[i], mode_str) !=NULL) {
					if (strstr(temp[i],seq)!=NULL) {new_mode=1;}
					if (strstr(temp[i],par)!=NULL) {new_mode=2;}
					i++;
					continue;
					}
				if ( check_exit(temp[i])==true) {
					exit(0);
					}
				if ((child_pid = fork() ) ==0) { 
					//if in child
					if (execv(temp[i], temp) < 0) {
        						printf("execv failed");
    						}
					}
				else {
					//if parent wait
					wait(); 
					}
				i++; //increment

				}
			
			
			// create new process for each new command
			// change mode 
			if (new_mode==1) {mode=0;}
			if (new_mode==2) {mode=1;}
		}
			
		
		
		// when in parallel mode...
		// check for exit command in parallel mode and mode switch in parallel
		char ** temp = ptrs;
		int i = 0;
		const char * mode_word = "mode";
		const char * exit_word = "exit";
		const char * seq = "s";
		const char * par = "p";
		if (mode ==1) {
			
			// loop through all tokens to find mode or exit changes
			while (temp[i] != NULL) {
		
				if (strstr(temp[i], mode_word) != NULL) {
					// means we need to change mode at the end
					if (strstr(temp[i], seq)) {
						new_mode = 1;
					}
					else if (strstr(temp[i], par)) {
						new_mode = 2;
					}
				}

				if (strstr(temp[i], exit_word)!=NULL) {
					exit = 1;	
				}
				i++;		
			}

			i = 0;

			while (ptrs[i] !=NULL) {
				// if it's NOT mode or exit, then...
				if (strstr(ptrs[i], mode_word)==NULL && strstr(ptrs[i], exit_word)==NULL) {
					pid_t child = fork();

					if (child ==0) {
						execv(ptrs[i]); // HOW do i call execv?
					}
				
					else {
						wait(); 
					}
				}
				i++;
			}

				
			// do mode and exit stuff now

			if (new_mode ==1 ) {
				mode = 0; // update it to sequential
			}
			else if (new_mode ==2) {
				mode = 1; // update it to parallel
			}

			if (exit == 1) {
				exit();
			}
		}
	}





	// fgets returned NULL, so check for EOF
	if (feof(stdin)) {
		exit(0); // exit if there's EOF 
	}
	
	free(ptrs);

    return 0;
}


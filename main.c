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

void removewhitespace(char *s) {

	// move through the c string with reading and writing markers
	int r = 0;
	int w = 0;
	while (r < strlen(s)) {
		if (!isspace(s[r])) {
			s[w] = s[r]; // copy char
			r++; 	
			w++; 	
		}
		else {r++;} // advance reader if space is found	
	}
	s[w] = '\0';
}


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


char** tokenify_semicolon(char input []) {
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

char** tokenify_space(char input []) {
	char *ptr_input = input;
	char *pstring = strdup(ptr_input); //pointer to copy of input 
	char *ptoken; 
	ptoken = strtok(pstring, " \t\n"); // ptoken now points to first token
	char **array = malloc(sizeof(char*)*((strlen(ptr_input)/2)+2));
	
	// count the number of tokens
	int i = 0;
	while (ptoken != NULL) {
		char *ptrtok = strdup(ptoken);
		array[i] = ptrtok;
		ptoken = strtok(NULL, " \t\n"); // get each next token
		i++;	
	}
	// set last index in array to NULL
	array[i]=NULL;
	// free 
	free(pstring);
	return array;
}

bool check_mode(char* input) {
	char * mode = "mode";

	removewhitespace(input);
	if ((strcmp(mode, input))==0) {
		return true;
	}
	return false;

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int main(int argc, char **argv) {

	int mode = 0; // 0 for sequential, 1 for parallel
	int exit_later = 0; // specifies if we should exit at end of parallel execution 
	int new_mode = 0; // 0 means no change, 1 means to S, 2 means to P
	char ** ptrs;
	bool done = false;

	while (!done) {


		char input[1024];
		char * input_dup = input;
		printf("prompt: ");
		fflush(stdout);

		if (fgets(input, 1024, stdin)!= NULL) {
	

		
			input[strlen(input)-1] = '\0';
			const char *inputptr = input;
			// exit if input only says EOF or exit
			if (check_exit(inputptr)==true) { exit(0);} // must pass const char * here

			if (check_mode(input_dup)==true) {

				if (mode ==0) {
					printf("Current mode is sequential\n");
				}

				else if (mode==1) {
					printf("Current mode is parallel\n"); 
				}
			}

			// ignore comments; replace all # with null termination
			replace_hash(input);

			// parse the command and organize the separate commands in memory
			ptrs = tokenify_semicolon(input); // array of char pointers to each different token (commands will always start with /bin)

	////////////////////////////////////////////////////////////////////////
			const char * mode_word = "mode";
			const char * exit_word = "exit";
			const char * seq = "s";
			const char * par = "p";
			char** command;
	//////////////////////////////////////////////
			if (mode ==0) {
				char** temp1= ptrs;
				int i =0;
			
				pid_t child_pid;
				while (temp1[i]!=NULL) {
					if ( strstr(temp1[i], mode_word) !=NULL) {
						if (strstr(temp1[i],seq)!=NULL) {new_mode=1;}
						if (strstr(temp1[i],par)!=NULL) {new_mode=2;}
						i++;
						continue;
						}
					if ( check_exit(temp1[i])==true) {
						exit(0);
						}
					child_pid = fork();
					if (child_pid ==0) { 
						//if in child
						command = tokenify_space(*temp1);
						if (execv(command[0], command) < 0) {
		    						printf("execv failed\n");
						}
						exit(0);
					
						}
					else {
						//if parent wait
						int return_status;
						waitpid(child_pid, &return_status, 0); 
						}
					i++; //increment
					}

				// change mode 
				if (new_mode==1) {mode=0;}
				if (new_mode==2) {mode=1;}
			}
			
	///////////////////////////////////////////////////////////////////////////////////
		
			// when in parallel mode...
			// check for exit command in parallel mode and mode switch in parallel
			char ** temp = ptrs;
			int i = 0;

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
						exit_later = 1;	
					}
					i++;		
				}

				i = 0;

				while (ptrs[i] !=NULL) {
					// if it's NOT mode or exit, then...
					if (strstr(ptrs[i], mode_word)==NULL && strstr(ptrs[i], exit_word)==NULL) {
						pid_t child = fork();

						if (child ==0) {
							execv(ptrs[i], ptrs); // HOW do i call execv?
						}
				
						else {
							waitpid(child, NULL, -1); 
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

				if (exit_later == 1) {
					exit(0);
				}
			}
		}

		// fgets returned NULL, so check for EOF
		if (feof(stdin)) {
			exit(0); // exit if there's EOF 
		}
	
		free(ptrs);

	}
	return 0;
}


//Claire Grace 
//Ethan Jones
// Ethan worked on the sequential code, Claire worked on the initial parallel code, and they worked on everything else together.

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

struct node {
	char * word;
	struct node *next;

};

/* plan for background job capability 
struct node2 {
	pid_t pid;
	char * process;
	char * status;
	struct node2 *next;

};
*/

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


void replace_hash (char *arr) {
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
	free(pstring);
	return array;
}


void free_tokens(char **tokens) {
    int i = 0;
	char ** temp = tokens;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(temp); // then free the array
}

char** tokenify_space(char *input) {
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

void list_append(char* name, struct node **head) { // taken from hw03
	
	struct node *newnode = malloc(sizeof(struct node));
	newnode->word = name;
	newnode->next = NULL;
	if (*head == NULL) { // if list is empty, set it to new node
		*head = newnode; 
		return;
	}
	struct node *temp = *head;
	while (temp->next != NULL) {
		temp = temp->next;
	}
	temp->next = newnode;
	return;
}


void print_mode_status(int mode) {
	if (mode ==0) {
		printf("Current mode is sequential\n");
	}

	else if (mode==1) {
		printf("Current mode is parallel\n"); 
	}	
	return;
}

void update_mode(int new_mode, int * modep) {
		if (new_mode==1) {*modep=0;}
		if (new_mode==2) {*modep=1;}
		return;
}

void free_LL(struct node *head) { 
    while (head != NULL) {
		struct node *temp = head;
		struct node *next = head->next;
        free(temp); // free each node
        head = next;
    }
	free(head);
    return;

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {

	int mode = 0; // 0 for sequential, 1 for parallel
	int * modep = &mode;
	int exit_later = 0; // specifies if we should exit at end of parallel execution 
	int new_mode = 0; // 0 means no change, 1 means to S, 2 means to P
	char ** ptrs;
	bool done = false;
	char *newnewstring = malloc(sizeof(char)*1024);
	struct node *head = malloc(sizeof(struct node));
	const char * mode_word = "mode";
	const char * exit_word = "exit";
	const char * seq = "s";
	const char * par = "p";
	char** command;
	FILE *fp = NULL;
	fp = fopen ("shell-config", "r");
	char line[1024];
	char * linep = line;
	char * newp;
	char newstr[1024];
	char *newstring=newstr;

	while(fgets(line, 128, fp) != NULL)
   {

	 	//strip line
		removewhitespace(linep);
		newp = malloc(sizeof(char)*1024);
		strcpy(newp, linep);
		char slash[] = "/";
		char * slashp = slash; // add a slash to the end of every line in shell-config
		newp=strcat(newp, slashp);

		//then append to the list 
		list_append(newp, &head);
   }
	//free(newp);

	struct node *tempnode = head->next;
	while (!done) {


		char input[1024];
		char * input_dup = input;
		char * mode_check = "mode";
		printf("prompt: ");
		fflush(stdout);

		if (fgets(input, 1024, stdin)!= NULL) {
	
			input[strlen(input)-1] = '\0';
			replace_hash(input); // ignore comments; replace all # with null termination
			ptrs = tokenify_semicolon(input); // parse the command and organize the separate commands in memory
		

			if (strstr(input_dup, mode_check)!=NULL) {
				if (check_mode(input_dup)==true) {
					print_mode_status(mode);
				}
			}

	// if in sequential mode: 
			if (mode ==0) {
				char** temp1= ptrs;
				int i =0;

				pid_t child_pid;
				while (temp1[i]!=NULL) {

					if (strstr(temp1[i], mode_word) !=NULL) {
						if (strstr(temp1[i],seq)!=NULL) {new_mode=1;}
						if (strstr(temp1[i],par)!=NULL) {new_mode=2;}
						i++;
						continue;
						}
					if ( check_exit(temp1[i])==true) {
						exit(0);
					}

					child_pid = fork();
					if (child_pid ==0) {  // if in child
						command = tokenify_space(temp1[i]);
						
						if (command[0] == NULL) {
							i++;
							continue;
						}

						// path variable capability
						struct stat statresult;
						int rv = stat(command[0], &statresult);
						char *cmdcopy=command[0];
						newstr[0] = '\0';	
						
	
						if (rv<0){
							while(tempnode->next!=NULL) {
								newstring=strcat(newstring,tempnode->word);
								newstring=strcat(newstring,cmdcopy);
								rv= stat(newstring,&statresult);
					
								if (rv==0) {
									strcpy(newnewstring, newstring);
									command[0] = newnewstring;
									break;
								}
								tempnode=tempnode->next;
							}
						}

						if (execv(command[0], command) < 0) {
		    						printf("execv failed\n");
						}
						//free_tokens(command);
						exit(0);
					
					}
					
					else {
						//if parent wait
						int return_status;
						waitpid(child_pid, &return_status, 0); 
						i++;	
					}
				}

				// change mode 
				update_mode(new_mode, modep);
			}
			
	///////////////////////////////////////////////////////////////////////////////////
		
			// when in parallel mode...
			// check for exit command in parallel mode and mode switch in parallel

			else if (mode ==1) {
				char ** temp = ptrs;
				int i = 0;
			
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

							pid_t child2 = fork();
							if (child2 == 0) {
								command = tokenify_space(temp[i]);

								if (command[0] == NULL) {
									i++;
									continue;
								}	

								// path variable capability
								struct stat statresult;
								int rv = stat(command[0], &statresult);
								char *cmdcopy=command[0];
								newstr[0] = '\0';
							
								if (rv<0){
					
									while(tempnode->next!=NULL) {
										newstring=strcat(newstring,tempnode->word);
										newstring=strcat(newstring,cmdcopy);
										rv= stat(newstring,&statresult);
										if (rv==0) {
											strcpy(newnewstring, newstring);
											command[0] = newnewstring;
											break;
										}
										tempnode=tempnode->next;
									}
								}

								if (execv(command[0], command) < 0) {
									printf("execv failed\n");
								}
								//free_tokens(command);
								exit(0);
							}
								
							
							else {
								int return_status;
								waitpid(child2, &return_status, 0);
								printf("Process %i has completed", child2);
								exit(0);
							}						
						}
				
						else {
							//int return_status;
							//waitpid(child, &return_status, 0);	
						}
					}
					i++;
				}

				// handle mode and exit now
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
		
	}
	fclose(fp);
	//free_tokens(command);
	free(newnewstring);
	//free_LL(head);
	free(head);		
	return 0;
}


/* Chaitanya Ingle, 2000475661, CS 370 Project 2: Linux Shell */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>

int main() {

// --------------------------------
// Declare Program Variables

  int cursor = 0;
  char next_char;
  int cmd_i;
  char* cwd = (char*) malloc(256);

// --------------------------------
// Create + Initialize Command History array and variables

  char cmd_history[5][256];

  int hist_init = 0;
  while (hist_init < 5) {
	
    int m = 0;
    while (m < 256) {

      cmd_history[hist_init][m] = '\0';
      m++;
    }

    hist_init++;
  }

  int hist_size = 0;
  int hist_i = 0;

// --------------------------------
// Change termios config

  // Get original config
  struct termios origConfig;
  tcgetattr(0, &origConfig); 

  // Create a copy of the original config
  struct termios newConfig = origConfig;

  // Adjust the new config
  newConfig.c_lflag &= ~(ECHO|ICANON);  
  newConfig.c_cc[VMIN] = 10;
  newConfig.c_cc[VTIME] = 2;  
	
  // Set the new config
  tcsetattr(0, TCSANOW, &newConfig);  

// --------------------------------
// Main While loop  
  
  while (true) {

  MainLoopStart:
    
    // Display Prompt ---------------------

    getcwd(cwd, 256);
    printf("%s/shell> ", cwd); 
    cursor = 0;  
    hist_i = hist_size;

    // Get input -------------------------
    
  currentPrompt:

    next_char = getchar();   
      
    char cmd[256];  
    int cmd_i = 0;   

    while (next_char != '\n') { 

      // If backspace or delete keys ------
      if (next_char == 8 || next_char == 127) { 
	    
	if (cursor > 0) {
		  
	  putchar('\b');  
	  putchar(' ');   
	  putchar('\b');  
	  cursor--;
	  cmd_i--;
	  next_char = getchar();
	}
	      			
	else
	  next_char = getchar();   
      }
	  	
      else {

	// If arrow key -------------------
	if (next_char == 27) {        
		
	  char c2, c3;  
	  c2 = getchar();
	  c3 = getchar();

	  // Right arrow
	  if (c3 == 67) {  
	    if (cursor < cmd_i) {
	      putchar(cmd[cursor]);
	      cursor++;
	    }
	  }

	  // Left arrow
	  if (c3 == 68) {
	    if (cursor > 0) {
	      putchar('\b');
	      cursor--;
	    }

	  }

	  // Up arrow
	  if (c3 == 65)  { 

	    if (hist_i-1 >= 0) { 
			
	      hist_i--;
	      cmd_i = 0;   
			  
	      while (cursor > 0) { 
			    
		putchar('\b');
		putchar(' ');
		putchar('\b');
			      
		cursor--;
	      }
			  
	      int j = 0;
	      while (cmd_history[hist_i][j] != '\0') { 
			   
		putchar(cmd_history[hist_i][j]);
		cmd[cmd_i] = cmd_history[hist_i][j];
		j++;
		cmd_i++;
		cursor++;
	      }
	    } 
	  }

	  // Down arrow
	  if (c3 == 66) {   
	    
	    if (hist_i+1 <= hist_size) { 
    
	      hist_i++;
	      cmd_i = 0;
	      while (cursor > 0) {  
	     
		putchar('\b');
		putchar(' ');
		putchar('\b');
			      
		cursor--;
	      }
			  
	      if (hist_i < hist_size) {  
		    
		int j = 0;
		while (cmd_history[hist_i][j] != '\0') {
			
		  putchar(cmd_history[hist_i][j]);
		  cmd[cmd_i] = cmd_history[hist_i][j];
		  j++;
		  cmd_i++;
		  cursor++;
		}
	      }
	    }
	  }
		  
	  next_char = getchar();
	}

	// If not special key, store charater --------------
	else {
       
	    putchar(next_char);    
	    cursor++;
	    cmd[cmd_i] = next_char; 
	    cmd_i++;
	    next_char = getchar();
	  
	}
      }
    }

    cmd[cmd_i] = '\0';

    // Add to history if not '\n' --------------

    // If history is not filled yet -------------
    if (cmd[0] != '\0') {
      
      if (hist_size < 5) { 
	  
	int k = 0;
	while (cmd[k] != '\0') {
	      
	  cmd_history[hist_size][k] = cmd[k];
	  k++;
	}
	cmd_history[hist_size][k] = '\0';
	hist_i = hist_size;
	hist_size++;
      }

      // If history is filled -------------------
      else  { 
	  
	int n = 0;
	while (n < 4) {
	      
	  strcpy(cmd_history[n], cmd_history[n+1]);
	  n++;
	}
	      
	int k = 0;
	while (cmd[k]!='\0') {
	      
	  cmd_history[4][k] = cmd[k];   
	  k++;
	}
	cmd_history[4][k]='\0';
      }
    }
      
   
    int numArgs = 0;
    int k = 0;

// ----------------------------------------
// Tokenize command 

    while (k < sizeof(cmd)) { 
      
      if (cmd[k] == '\0')
	break;
      if (cmd[k]!=' ' && (cmd[k+1]==' ' || cmd[k+1]=='\0'))
	numArgs++;
      k++;
    }
    char* cmdArray[numArgs+1];  
    cmdArray[numArgs] = NULL;
      
    int idx = 0;
    int currentArg = 0;
    char* p;
    p = strtok(cmd, " ");
    while (p != NULL) {  
      
      cmdArray[currentArg] = p;  
      currentArg++;
      p = strtok(NULL, " ");
    }

    int newL_stat = 0;
    if (cmd[0] != '\0') {

      // Check if pause command ------------------
      
      char* pauseString = "pause";
      if (strcmp(cmdArray[0], pauseString) == 0) {
	char lf_char = getchar();

	while (lf_char != '\n')
	  lf_char = getchar();

	printf("\n");
	goto MainLoopStart;
      }

      
      // Check if cd command ----------------
      
      char* cdString = "cd";
      if (strcmp(cmdArray[0], cdString) == 0) { 
	  
	int stat = chdir(cmdArray[1]);  

	if (stat != 0) {
	      
	  perror("\n");
	  newL_stat = 1;
	}
      }
      else {

	// Check if quit command ---------------
	
	char* exitString = "quit";
	if (strcmp(cmdArray[0], exitString) == 0) { 
	      
	  if (cmdArray[1] == '\0') { 
		  
	  badExit:
		
	    printf(" Are you sure you want to quit? (y/n) ");  
	    char exitConfirm;
	    exitConfirm = getchar();
	    if (exitConfirm == 'y') { 
		      
	      printf("%c\n", exitConfirm);
	      break;
	    }
	    
	    else {
		      
	      if (exitConfirm == 'n')  	  
		putchar(exitConfirm);
			  
	      else {
			  
		printf("\nInvalid option  -");  
		goto badExit;
	      }
	    }
	  }
	}
	
// ----------------------------------
// Execute command 
	
	else { 	      

	  int saveIN = dup(0);
	  int saveOUT = dup(1);
	  int findPipe = 0;
	  int pipe_stat;
	  char* pipeString = "|";
	  char* gtString = ">";
	  int findgt = 0;
	  int p;
	  if (numArgs >= 3) {
	    int p = 0;
	    while (p < numArgs+1) {
		      
	      pipe_stat = strcmp(cmdArray[p], pipeString);
	      int gt_stat = strcmp(cmdArray[p], gtString);
	      if (pipe_stat == 0) {
			  
		findPipe = p;
		break;
	      }
	      if (gt_stat == 0) {
			  
		findgt = p;
		break;
	      }
	      p++;
	    }
	  }

// ------------------------------
// If Pipe was found
	  
	  if (findPipe != 0) {
		  
		     
	    char* cmd_1[findPipe+1];
	    char* cmd_2[numArgs - findPipe];
		     
	    int pstat;

	    int k = 0;
	    while (k < findPipe) {
		      
	      cmd_1[k] = cmdArray[k];
	      k++;
	    }
	    cmd_1[k] = '\0';
	       
	    k = 0;
	    int ck = findPipe + 1;
	    while (cmdArray[ck] != '\0') {
		      
	      cmd_2[k] = cmdArray[ck];
	      k++;
	      ck++;
	    }
	    cmd_2[k] = '\0';
		      


	    int fd[2];
	    pipe(fd);
	    int pid_p = fork();
	    if(pid_p == 0) {
		      
			  
	      int pstatC;
	      int pid_pC = fork();
	      if(pid_pC == 0) {
			  

		close(0);
		dup(fd[0]);
		close(fd[1]);
		close(fd[0]);
		execvp(cmd_2[0], cmd_2);

		exit(0);
	      }

	      else {
			  
			      
		printf("\n");

		close(1);
		dup(fd[1]);
		close(fd[0]);
		execvp(cmd_1[0], cmd_1);
			      
	      }
	      exit(0); 
	    }
	    else {
		      
			 
	      waitpid(pid_p, &pstat, WUNTRACED);			  
			  
	      dup2(saveIN, 0);
	      dup2(saveOUT, 0);
			  
	      newL_stat = 1; // Do not print new line at the end
	    }
		      
	  }
	  
// -------------------------------------------
// Merge command
	  
	  else {
		  
		      
	    int pid_stat;
	    int pid = fork();
		      
	    newL_stat = 1; // Do not print new line at the end
		      
	    if (pid == 0) {  //Child process - will execute cmd
		      
	      printf("\n");
	      char* mergeString = "merge";
	      int merge_stat = strcmp(cmdArray[0], mergeString);
	      if (merge_stat == 0) {
			  
		char cat_cmd[256];

		cat_cmd[0]='c';
		cat_cmd[1]='a';
		cat_cmd[2]='t';
		cat_cmd[3]=' ';
			      
		int x = 0;
		while (true) {
			      
		  if (cmd[x]=='e' && cmd[x+1]==' ')
		    break;
		  x++;
		}

		x = x+2;
		int cat_i = 4;
			      
		while (cmd[x] != '\0') {
			      
		  cat_cmd[cat_i] = cmd[x];
		  cat_i++;
		  x++;
		}

		cat_cmd[cat_i] = '\0';
		int sys_stat = system(cat_cmd);
		if (sys_stat == -1)
		  perror(NULL);

		exit(0);
	      }
	      else {
			  
		int execute_stat = execvp(cmdArray[0], cmdArray);
		if (execute_stat == -1)
		  perror(NULL);
		exit(0);
	      }
	    }
		      
	    else //Parent process
	      waitpid(pid, &pid_stat, WUNTRACED);
		      
		      
	  }
	}
      }
      
      if (newL_stat == 0)
	printf("\n");
    }
  }
    

  tcsetattr(0, TCSANOW, &origConfig);  //restore original termios configuration
  
  return 0;
}

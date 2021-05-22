#include <functions.h>


//function to execute mkdir when user enters cd
void call_mkdir(char** array, int count){
   printf("\n");				//print newline for output to look nice
   pid_t spawnPid = -5;
   int childExitStatus = -5;
   spawnPid = fork();
   switch(spawnPid){
      case -1: {perror("Hull Breach.\n"); break;}
      case 0: {
	      sleep(1);
	      execvp(*array, array);				//calls execvp on string array holding mkdir
	      perror("Child: execute failure.\n");
	      break;
	      }
      default: {
	       sleep(2);
	       pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);	//waitfid function to wait for child
	       break;
	       }
   }
}


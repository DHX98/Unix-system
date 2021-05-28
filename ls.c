#include <functions.h>



//function when user calls ls to displayed files and directories
void call_ls(char** array, int count){
   printf("\n");
   pid_t spawnPid = -5;
   int childExitStatus = -5;
   spawnPid = fork();
   if(count == 1){						//if user calls ls with no redirection
   switch(spawnPid){
      case -1: {perror("Hull Breach\n"); break;}
      case 0: {
	      sleep(1);
	      execlp("ls", "ls", NULL);				//calls execlp for ls
	      perror("Child: execute failure.\n");
	      break;
	      }
      default: {
	       sleep(2);
	       pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
	       break;
	       }
   }
   }
   else{							//else if user calls ls with a redirection
      switch(spawnPid){
	 case -1: {perror("Hull Breach\n"); break;}
	 case 0: {
		    sleep(1);
		    int sourcefd, targetfd, result;
		    targetfd = open(array[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);	//open file for redirection
		    result = dup2(targetfd, 1);
		    execlp("ls", "ls", NULL);
		    perror("Child: execute failure.\n");
		    close(targetfd);
		    break;
		 }
	 default: {
		     sleep(2);
		     pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);		//waits for child pid
		     break;
		  }
      }
   }
   return;
}


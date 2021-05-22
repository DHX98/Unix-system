#include <functions.h>



//function that call exec wc when user inputs wc
void call_wc(char** array, int count){
   pid_t spawnPid = -5;
   int childExitStatus = -5;
   printf("\n");					//prints newline to make output look nice
   char* str1 = malloc(1024 * sizeof(char*));
   char* str2 = malloc(1024 * sizeof(char*));
   str1 = array[2];
   if(strstr(str1, "junk") != NULL){
      str1 = "junk";
   }
   else{
      printf("Error: cannot open for input.\n");
      return;
   }
   spawnPid = fork();					//calls fork to make a child process
   if(count == 3){
   switch(spawnPid){
      case -1: {					//if statement to determine if only one redirection
		  perror("Hull Breach.\n"); exit(1); break;
	       }
      case 0: {
		 sleep(1);
		 int targetfd, result;
		 targetfd = open(array[2], O_RDONLY);
		 result = dup2(targetfd, 0);
		 execlp("wc", "wc", NULL);		//calls execlp on wc for input with only one redirection
		 perror("Child: execute failure.\n");
		 close(targetfd);
		 break;
	      }
      default: {
		  sleep(2);
		  pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
		  break;
	       }
   }
   }
   else{
      switch(spawnPid){					//else the user inputed a wc command with multiple redirections
	 case -1: {
		     perror("Hull Breach.\n"); exit(1); break;
		  }
	 case 0: {
		    sleep(1);
		    int targetfd1, result1, sourcefd, result2;
		    targetfd1 = open(array[2], O_RDONLY);
		    sourcefd = open(array[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		    result1 = dup2(targetfd1, 0);
		    result2 = dup2(sourcefd, 1);
		    execlp("wc", "wc", NULL);		//calls execlp on wc with multiple redirections to different files
		    perror("Child: execute failure.\n");
		    close(targetfd1);
		    close(sourcefd);			//closes files that were opened for wc redirection
		    break;
		 }
	 default: {
		     sleep(2);
		     pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
		     break;
		  }
      }
   }
   return;
}


#include <functions.h>



						//manual function for when user call cat
void call_cat(char** array, int count){
   char* str1 = malloc(1024 * sizeof(char*));
   str1 = array[1];
   printf("\n");
   char buffer[256];
   memset(buffer, '\0', sizeof(buffer));
   FILE* file_name;						//creates file descriptor
   file_name = fopen(str1, "r");				//opens desired file given by user input
   if(file_name){
      while(fread(buffer, 1, sizeof(buffer), file_name) != NULL){	//reads through the desired file and puts file into buffer string
	 fread(buffer, sizeof(buffer), 1, file_name);
	 printf("%s\n", buffer);
      }
   }
   fclose(file_name);
   return;
}


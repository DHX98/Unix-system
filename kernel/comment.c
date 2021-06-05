#include <functions.h>



						//manual function for when user wants to enter a comment
void call_comment(char** array, int count){
   int i = 0;
   printf("\n");
   for(i; i < count; i++){
      if(i < (count - 1)){
	 printf("%s ", array[i]);			//prints string and spaces inbetween string
      }
      else{
	 printf("%s", array[i]);			//if array is on the last string, dont print a space at the end
      }
   }
}


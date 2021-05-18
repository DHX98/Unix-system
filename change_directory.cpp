#include <functions.h>




//function that calls cd when user inputs cd
void call_cd(char** array, int count){
    printf("\n");
    if(count > 1){              //if user inputs cd with an argument
        chdir(array[1]);
    }
    else{                   //else the user inputed just cd to get to home directory
        chdir(getenv("HOME"));
    }
}

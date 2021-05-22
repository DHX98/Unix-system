#include <functions.h>




//function to print the exit status
void call_status(char** array, int count){
    pid_t cpid, w;
    int status;
    printf("\n");
    if(count == 1){                   //if statement if stauts is called with &
        printf("Exit status: %d\n", WEXITSTATUS(0));
    }
    else{
        cpid = fork();
        do{                      //else call status by itself
            w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
            if(w == -1){                   //if no child process
                exit(EXIT_FAILURE);
            }
            if(WIFEXITED(status)){             //else return exit status of child
                printf("Exit status: %d\n", WEXITSTATUS(status));
            }
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

// Globals
int pidCount, status;
pid_t runningPidList[256];
bool allowBG = true, allowBGChanged = false;

// Signal handlers
void empty_handler(int sigNo) {
    //write(STDOUT_FILENO,"\n",1);
}

void sigint_handler(int sigNo) {
    //write(STDOUT_FILENO,"t\n",2);
    exit(sigNo);
}

void sigtstp_handler(int sigNo) {
    allowBG = !allowBG;
    allowBGChanged = true;
}

int main(int argc, char **argv) {
    signal(SIGINT, empty_handler);
    signal(SIGTSTP, sigtstp_handler);
    char line[2049] = {0}, **args;
    int argCount;
    printf(": ");

    // Loop until exit
    while (scanf("%[^\n]", line) != EOF) {
        getchar();
        if (allowBGChanged) {
            if (allowBG) printf("Exiting foreground-only mode\n");
            else printf("Entering foreground-only mode (& is now ignored)\n");
            allowBGChanged = false;
        }
        if (strlen(line)) {

            // Parse args
            //args = parse(line, &argCount);

            // Skip comments and empty lines
            if (args[0] == NULL || args[0][0] == '#') {
                printf(": ");
                // Free all data in args
                //clean(args);
                memset(line, 0, sizeof(line));
                continue;
            }

            // Run command from args
            //run_cmd(argCount, args);

            // Check if any background processes finished
            int stat;
            for (int i = 0; i < pidCount; i++) {
                if (waitpid(runningPidList[i], &stat, WNOHANG) == runningPidList[i]) {
                    printf("background pid %d is done: terminated by signal %d\n", runningPidList[i], stat);
                    runningPidList[i] = 0;
                }
            }

            // Update runnning process ID list
            int j = 0;
            for (int i = 0; i < pidCount; i++) { if (runningPidList[i]) { runningPidList[j++] = runningPidList[i]; } }
            pidCount = j;
        }
        fflush(stdout);
        printf(": ");
        memset(line, 0, sizeof(line));
    }
    return 0;
}

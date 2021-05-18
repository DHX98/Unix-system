//main function for the shell

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#define bool int
#define true 1
#define false 0

void addProcess(int pid);

// check command if background.
bool isBackGroundProcess(char **argv);

// implement < and > operator
int redirect(int argc, char **argv, bool isBg);

// back ground mode 
bool isBackGroundProcess(char **argv);

int main(int argc, char **argv) {
    // main process and background process are ignore SIGINT.
    struct sigaction sigint_act = {0}, sigtstp_act = {0};
    sigint_act.sa_handler = SIG_IGN;
    sigfillset(&sigint_act.sa_mask);
    sigint_act.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sigint_act, NULL);
    sigtstp_act.sa_handler = sigtstp_handler;
    sigfillset(&sigtstp_act.sa_mask);
    sigtstp_act.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sigtstp_act, NULL);
    char line[2049] = {0}, **args;
    int argCount;
    printf(": ");
    // get a line
    while (scanf("%[^\n]", line) != EOF) {
        // get nextline character
        getchar();
        if (allowBGChanged) {
            if (allowBG) printf("Exiting foreground-only mode\n");
            else printf("Entering foreground-only mode (& is now ignored)\n");
            allowBGChanged = false;
        }
        if (strlen(line)) {
            args = parse(line, &argCount);
            if (args[0] == NULL || args[0][0] == '#') { // skip comments
                printf(": ");
                clean(args);
                memset(line, 0, sizeof(line));
                continue;
            }
            run_cmd(argCount, args);
        }

        memset(line, 0, sizeof(line));

        int stat;
        int i = 0;
        for (i = 0; i < pidCount; i++) {
            if (waitpid(runningPidList[i], &stat, WNOHANG) == runningPidList[i]) {
                printf("background pid %d is done: terminated by signal %d\n", runningPidList[i], stat);
                // set null
                runningPidList[i] = 0;
            }
        }
        // clean background process
        int j = 0;
        for (i = 0; i < pidCount; i++) {
            if (runningPidList[i])runningPidList[j++] = runningPidList[i];
        }
        pidCount = j;

        printf(": ");
        fflush(stdout);

    }
    return 0;
}
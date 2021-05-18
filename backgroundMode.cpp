//This is the background mode and redirect file

// background process id list
pid_t runningPidList[256];
// number of background processes
int pidCount;
// allow background
bool allowBG = true, allowBGChanged = false;
// exit status of child process
int status;

// check command if background.
bool isBackGroundProcess(char **argv);

// implement < and > operator
int redirect(int argc, char **argv, bool isBg);

// add a new background process
void addProcess(int pid) {
    runningPidList[pidCount++] = pid;
}

// cd build-in
void changeDir(int argc, char **argv) {
    char *dir = NULL;
    if (argc == 1) {
        if (!(dir = getenv("HOME"))) {
            fprintf(stderr,"cd: %s\n", strerror(errno));
        }
    } else if (argc == 2) {
        dir = argv[1];
    } else {
        printf("usage: cd [directory]\n");
    }
    if (chdir(dir) == -1) {
        fprintf(stderr,"cd: %s\n", strerror(errno));
    }
}


bool isBackGroundProcess(char **argv) {
    int i = 0;
    for (i = 0; argv[i]; i++) {
        if (argv[i + 1] == NULL && !strcmp(argv[i], "&")) {
            free(argv[i]);
            argv[i] = NULL;
            return true;
        }
    }
    return false;
}

int redirect(int argc, char **argv, bool isBg) {
    // background process's default io is /dev/null.
    char *input = isBg ? "/dev/null" : NULL;
    char *output = isBg ? "/dev/null" : NULL;
    int newArgc = -1;
    int i = 0;
    for (i = 0; argv[i]; i++) {
        // process <
        if (!strcmp(argv[i], "<")) {
            input = argv[i + 1];
            argv[i] = NULL;
            free(argv[i]);
            newArgc = newArgc == -1 ? i : newArgc;
        } else if (!strcmp(argv[i], ">")) { // process >
            argv[i] = NULL;
            output = argv[i + 1];
            free(argv[i]);
            newArgc = newArgc == -1 ? i : newArgc;
        }
    }
    if (input) {
        // redirect input
        int fd = open(input, O_RDONLY, S_IRUSR | S_IWUSR);
        if (fd <= 0) {
            char buffer[256];
            sprintf(buffer, "cannot open %s for input\n", input);
            write(STDOUT_FILENO, buffer, strlen(buffer));
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        if (strcmp(input, "/dev/null") != 0)free(input);
    }
    if (output) {
        // redirect output
        if (strcmp(output, "/dev/null") != 0) {
            int fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd <= 0) {
                fprintf(stderr, "%s cannot open.\n", output);
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            free(output);
        } else {
            int fd = open(output, O_WRONLY);
            dup2(fd, STDOUT_FILENO);
        }
    }
    return newArgc == -1 ? argc : newArgc;
}
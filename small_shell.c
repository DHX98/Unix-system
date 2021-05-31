#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#define MAX_LEN 2048        //maximum number of user input chars
#define MAX_ARGS 512        //maximum number of args
#define BUF_SIZE 16         //allocate 16 more space for args
#define DELIM " \t\r\n\a"
#define NUM_BUILDINS 6      //number of buildin args
#define MAX_BGS 100         //maximum background process in the same time


void sh_loop();
char *sh_read_line();
char **sh_get_args(char *line);
int sh_execute(char **args);
int sh_lanuch(char **args);
int sh_cd(char **args);
int sh_exit(char **args);
int sh_status(char **args);
int sh_changeCmdName(char **args);
int sh_lock(char **args);
int sh_unlock(char **args);

void sh_add_bgs(long pid);  //add background process pid to array
void sh_kill_zombies();     //check and clear zombies
void catch_TSTP(int signo); //signal handler for SIGTSTP, toggle fg when called

char *buildin_args[] = { "cd", "exit", "status", "changeCmdName", "lock","unlock"};
int (*buildin_func[])(char **) = {&sh_cd, &sh_exit, &sh_status, &sh_changeCmdName, &sh_lock,&sh_unlock};
pid_t sh_pid;               //parent process pid
int num_bgs = 0;            //number of background process
int ter = 0;                //check if process terminated
int fg = 0;                 //foreground mode
long sh_bgs[MAX_BGS];       //store background process pid


int main(int argc, char **argv){
//all process ignored SIGINT, reset signal handler in foreground process
  struct sigaction ignore_action = {0};
  ignore_action.sa_handler = SIG_IGN;
  sigaction(SIGINT, &ignore_action, NULL);
//create new SIGTSTP signal handler
  struct sigaction TSTP_action = {0};

  TSTP_action.sa_handler = catch_TSTP;
  sigfillset(&TSTP_action.sa_mask);
  TSTP_action.sa_flags = SA_RESTART;

  sigaction(SIGTSTP, &TSTP_action, NULL);
//init STATUS and SIG
  setenv("STATUS","0",1);
  setenv("SIG", "0",1);
//get parent process pid
  sh_pid = getpid();
//getline, get args from line, execute
  sh_loop();


  return 0;
}

void sh_loop(){

  char  *line;
  char  **args;
  int   status;
//check if last foreground process get terminated by signal
  do {
    if(ter){     // if is, reset ter, and print terminated message
      sh_status(NULL);
      ter = 0;
    }
  //prompt user for input
    printf(": ");
    fflush(stdout);
  //check and clean zombies
    sh_kill_zombies();
  //read and store userinput
    line = sh_read_line();
  //get args from user input
    args = sh_get_args(line);
  //execute
    status = sh_execute(args);

    free(line);
    free(args);
  } while(status);  //loop while no error in lanuching process

}

char *sh_read_line(){
    char *line = NULL;                  //char pointer for user input
    size_t len = 0;                     //get input from user
    int num_chars = getline(&line, &len, stdin);
    if (num_chars > MAX_LEN) {          //check if input too big
      fprintf(stderr, "ERROR:Too many chars.\n" );
      fflush(stdout);
      exit(1);
    }
    return line;                        //return the line
}

char **sh_get_args(char *line){
  int n = 0;                            //tokens counter
  int bufsize = BUF_SIZE;
  char **args = malloc(bufsize*sizeof(char*));  //alloc space for args

  if(!args){
    fprintf(stderr, "ERROR:Fail to allocate space for args.\n" );
    fflush(stdout);
    exit(1);
  }

  char *token;                           //get args form line
  token = strtok(line, DELIM);
  while (token) {
//  for(n=0; token; n++){
    args[n] = token;
    n++;

    if(n >= bufsize){                     //if number of tokens > BUF_SIZE
      bufsize = bufsize + BUF_SIZE;       //increase bufsize and realloc space
      args = realloc(args, bufsize*sizeof(char*));
      if(!args){
        fprintf(stderr, "ERROR:Fail to allocate space for args.\n" );
        fflush(stdout);
        exit(1);
      }
    }

    if(n > MAX_ARGS){                      //check if too many args
      fprintf(stderr, "ERROR:Too many args.\n" );
      fflush(stdout);
      exit(1);
    }

    token = strtok(NULL, DELIM);
  }

  args[n] = NULL;                           //set end of args
  return args;
}
//execute
int sh_execute(char **args){

  if(args[0] == NULL || args[0][0] == '#'){  //ignored comment and empty line
    return 1;
  }

  int n = 0;
  while(args[n]){
    if(strstr(args[n],"$$")){                //expand $$ to parent pid
      int p;
      for(p = 0; p < strlen(args[n]); p++){  //find where is $$ in arg
        if(args[n][p] == '$' && args[n][p+1] == '$'){
          break;
        }
      }
      char pid_str[20];                       //convert parent pid to string
      sprintf(pid_str,"%ld",(long)(sh_pid));
      int slen = strlen(args[n]) + strlen(pid_str) - 1; //calculate result string length
      char result[slen];
      int t;                                //copy chars before $$
      for(t = 0; t < p ; t++){
        result[t] = args[n][t];
      }
      for(t = 0; t < strlen(pid_str); t++){  //expand $$
        result[p+t] = pid_str[t];
      }
      t += p;
      p += 2;
      for( ;t < slen; t++){                 //copy char after $$
        result[t] = args[n][p];
        p++;
      }
      result[t] = '\0';                     //end of arg
      strcpy(args[n], result);              //replace arg
    }
    n++;
  }

  int i;                                    //if is build in args, call function
  for(i=0; i<NUM_BUILDINS ;i++){
    if(strcmp(args[0], buildin_args[i]) == 0){
      return (*buildin_func[i])(args);
    }
  }

  return sh_lanuch(args);                   //else, lanuch process
}

int sh_lanuch(char **args){
  pid_t pid;
  pid_t wpid;
  int status;
  int bg = 0;
  int i = 0;
  while(args[i]){                           //find last args
    i++;
  }
  if(strcmp(args[i-1],"&") == 0){           //if is &
    args[i-1] = NULL;                       //replace it with NULL
    if(!fg){                                //if is not foreground only mode
      bg = 1;                               //is background process
    }
  }

  pid = fork();
  if (pid == 0) {                           //in child process
    if(!bg){                                //if not background process, reset SIGINT handler
      struct sigaction default_action = {0};
      default_action.sa_handler = SIG_DFL;
      sigaction(SIGINT,&default_action, NULL);
    }

    if(bg && num_bgs >= MAX_BGS){           //if is background, check maximum running bg pros
      fprintf(stderr, "ERROR:Too many background process.\n");
      fflush(stdout);
      exit(1);
    }

    int fd[2];
    if(bg){                                 //if is background process
      fd[0] = open("/dev/null", O_WRONLY);  //redirect stdout and stderror to /dev/null
      dup2(fd[0], STDOUT_FILENO);
      dup2(fd[0], STDERR_FILENO);
      fcntl(fd[0], F_SETFD, FD_CLOEXEC);
      fd[1] = open("/dev/null", O_RDONLY);  //redirect stdin to /dev/null
      dup2(fd[1], STDIN_FILENO);
      fcntl(fd[1], F_SETFD, FD_CLOEXEC);
    }

    int n = 1;
    int fout = 0, fin = 0;
    while(args[n]){                       //check if args contain < or >
      if(strcmp(args[n],">") == 0){       //if true, save index
        fout = n;
      }
      if(strcmp(args[n],"<") == 0){
        fin = n;
      }
      n++;
    }
    if(fout && args[fout+1]){         //if contain > and has next arg
      fd[0] = open(args[fout+1], O_WRONLY | O_CREAT ,0660); //redirect stdout to file
      if(fd[0] == -1){
        fprintf(stderr, "ERROR:File can not be opened.\n" );
        fflush(stdout);
        exit(1);
      }
      dup2(fd[0], STDOUT_FILENO);
      fcntl(fd[0], F_SETFD, FD_CLOEXEC);
      args[fout] = NULL;
      args[fout+1] = NULL;
    }

    if(fin && args[fin+1]){       //if contain < and has next arg
      fd[1] = open(args[fin+1], O_RDONLY); //redirect stdin to file
      if(fd[1] == -1){
        fprintf(stderr, "ERROR:File can not be opened.\n" );
        fflush(stdout);
        exit(1);
      }
      dup2(fd[1], STDIN_FILENO);
      fcntl(fd[1], F_SETFD, FD_CLOEXEC);
      args[fin] = NULL;
      args[fin+1] = NULL;
    }

    if(execvp(args[0],args) == -1){ //execute comman
      perror("smallsh");            //if fail, print error
      fflush(stdout);
    }
    exit(1);
  }
  else if(pid < 0){             //if fork fail
    perror("smallsh");          //print error message
    fflush(stdout);
  }
  else{                         //in parent process
    setenv("STATUS","0",1);     //reset STATUS and SIG
    setenv("SIG","0",1);
    if(bg){                     //if is backgroud process
      printf("background pid is %ld\n",(long)(pid));  //print mesage
      fflush(stdout);
      sh_add_bgs(pid);          //add pid to bg pid array
      return 1;
    }

    do{                         //foreground process
      wpid = waitpid(pid, &status, WUNTRACED);     //wait untill child pocess finished
    }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    if(WIFEXITED(status)){       //if is exited
      char str[12];              //set exit status
      sprintf(str,"%d",WEXITSTATUS(status));
      setenv("STATUS",str,1);
    }
    else{                         //else, if terminated by signal
      ter = 1;                    //set ter to true
      char str[12];               //set SIG
      sprintf(str,"%d",WTERMSIG(status));
      setenv("SIG",str,1);
    }
  }

  return 1;
}
//build in arg cd
int sh_cd(char **args){
    setenv("STATUS","0",1);
    if(args[1] == NULL){          //if no args after cd
      args[1] = getenv("HOME");   //cd to HOME dir
    }

    if (chdir(args[1]) != 0) {
      perror("smallsh");          //send error if chdir fail
      fflush(stdout);
      setenv("STATUS","1",1);     //set status to 1 if fail
    }

    return 1;
}
//build in arg exit
int sh_exit(char **args){
  int n;
  pid_t pid;
  for(n = 0; n<num_bgs; n++){     //kill all zombie process
    pid = sh_bgs[n];
    kill(pid, SIGTERM);
  }

  return 0;                        //exit shell
}
//build in arg status
int sh_status(char **args){
  char *sig_ter = getenv("SIG");    //if SIG is not 0, last process was terminated by signal
  if(strcmp(sig_ter, "0")){
    printf("terminated by signal %s\n",sig_ter);  //print terminated message
    fflush(stdout);
  }
  else{                             //else, print exit status message
    printf("exit value %s\n", getenv("STATUS"));
    fflush(stdout);
  }
  return 1;
}
// change the command base on args[1] and args[2]

int sh_changeCmdName(char **args){
  int i = 0;
  for(i = 0; i < NUM_BUILDINS; i++){
    if (strcmp(args[1], buildin_args[i]) ==0){
      printf("your command: '%s' was successfully changed to '%s'\nyour command '%s' will not work anymore\n",args[1],args[2],args[1]);
      buildin_args[i] = args[2];
      return 1;
    }
  }
  printf("can not find this cmd: '%s'\n", args[1]);
  return 1;
}

int sh_lock(char **args){
  FILE *fp;
  FILE *output;
  
/*  fp = fopen(args[1], "w");
  fprintf(fp, args[2]);
  fclose(fp);*/
  output = fopen("encryptedFile", "w");

  int MAXCHAR = 255; 
  char str[MAXCHAR];
  char str2[255][255]; //source
  int i =0 ; // size of input file
  fp = fopen(args[1], "r+");
  while (fgets(str, MAXCHAR, fp) != NULL){
    strcpy(str2[i], str);
    //strcat(str2[i],"2333");
    printf("%s", str2[i]);
    i++;
  }
  fclose(fp);
  fclose(output);
  printf("your file: %s is locked!\n -use unlock to unlock it\n", args[2]);
  return 1;

}
int sh_unlock(char **args){
  
  char ch[255];
  int MAXCHAR = 255; 
  char password[MAXCHAR];
  char str2[255]; //source
  
  FILE *fp;
  int i =0;
  fp = fopen("password", "r+");
  while(fgets(password, MAXCHAR, fp) != NULL){
    strcpy(str2, password);

    if(strcmp(args[1], str2) == 0){
      printf("password correct.\n");
      printf("Enter your fileName: \n");
      fgets(ch,MAXCHAR,stdin);
      printf("file %s is unlocked\n", ch);
    }
    else{
      printf("password wrong, please try again\n");
      break;
    }

  }

  return 1;

}
//add background pid to array
void sh_add_bgs(long pid){
  sh_bgs[num_bgs] = pid;
  num_bgs++;
}

//check and clean zombies
void sh_kill_zombies(){
  pid_t pid;
  pid_t wpid;
  int status;
  int bg_status = 0;
  int n;
  char *last_ter = getenv("SIG");   //reset last SIG after call status
  for(n=0; n < num_bgs; n++){       //check every pid in background pid array
    pid = sh_bgs[n];
    if(wpid = waitpid(pid, &status, WNOHANG)){    //if found zombie process
      int i = n;                                  //clean it
      while(i < num_bgs-1){                       //remove the pid from array
        sh_bgs[i] = sh_bgs[i+1];
        i++;
      }
      n--;
      num_bgs--;

      if(WIFEXITED(status)){                      //check if zombie exit
        char str[12];                             //if true, set STATUS and call status
        sprintf(str,"%d",WEXITSTATUS(status));
        setenv("STATUS",str,1);
      }
      else{                                       //else, set SIG and call status
        char str[12];
        sprintf(str,"%d",WTERMSIG(status));
        setenv("SIG",str,1);
      }
      printf("background pid %ld is done: ", (long)(pid)); //print zombies pid
      sh_status(NULL);
      setenv("SIG",last_ter,1);                            //set SIG back
      fflush(stdout);
    }
  }
}
//SIGTSTP signal handler
void catch_TSTP(int signo){
  if(fg){                 //toggle fg status and print message
    fg = 0;
    char * message = "Exiting foreground-only mode\n: ";
    write(STDOUT_FILENO, message , 32);
  }
  else{
    fg = 1;
    char * message = "Entering foreground-only mode (& is now ignored)\n: ";
    write(STDOUT_FILENO, message , 52);
  }
}

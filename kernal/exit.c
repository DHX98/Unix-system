#include <functions.h>



//exit function to exit funtion when user enteres "exit"
void call_exit(){
   kill(0, SIGKILL);		//kills all processes
   exit(0);
}

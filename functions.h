#ifndef FUNCTIONS_H
#define FUNCTIONS_H



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


void call_cd(char** array, int count);
void call_status(char** array, int count);
void call_mkdir(char** array, int count);
void call_wc(char** array, int count);
void call_cat(char** array, int count);
void call_comment(char** array, int count);
void call_ls(char** array, int count);
void call_exit();

#endif

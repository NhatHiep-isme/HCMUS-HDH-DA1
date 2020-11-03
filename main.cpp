#include <unistd.h>     // fork()
#include <sys/wait.h>   // waitpid()
#include <stdio.h>      // printf(), fgets()
#include <string.h>     // strtok(), strcmp(), strdup()
#include <stdlib.h>     // free()
#include <fcntl.h>      // open(), creat(), close()

using namespace std;
const unsigned MAX_LINE_LENGTH = 100;
const unsigned BUF_SIZE = 50;
const unsigned REDIR_SIZE = 2;
const unsigned PIPE_SIZE = 3;
const unsigned MAX_HISTORY = 30;
const unsigned MAX_COMMAND_NAME = 30;

void ParseCommand(char cmdString[], char *argv[])
{
  for(int i=0; i< BUF_SIZE; i++)
  {
    argv[i] = NULL;
  }
  
  int index = 0;
  char *tempStr = strtok(cmdstring, " ");
  while(tempStr != NULL)
  {
    argv[index++] = tempStr;
    tempStr = strtok(NULL, " ");
  }
  argv[index] = NULL;
}

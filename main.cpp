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

void ParseCommand(char *cmdString, char *argv[])
{
  int index = 0;
  char delim = " ";
  char *tempstr = strtok(cmdString, delim);
  while(tempStr!= NULL)
  {
    argv[index] = tempStr;
    tempStr = strtok(NULL, delim);
    index++;
  }
}

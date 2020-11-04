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
const unsigned MAX_HISTORY_SIZE = 30;
const unsigned MAX_COMMAND_NAME = 30;

void ParseCommand(char *cmdString, char *argv[], int *waitFlag)
{
  int index = 0;
  char delim = " ";
  char *tempStr = strtok(cmdString, delim);
  while(tempStr!= NULL)
  {
    argv[index] = tempStr;
    tempStr = strtok(NULL, delim);
    index++;
  }
  
  args[index] = NULL;
  if(args[args.end() - 1] != '&')
  { 
    waitFlag = 1;
  }
  else wait = 0;
}

void ChildProcess(char *argv[], char *redirectArgv[])
{
  int fdIn, fdOut;
  if(redirectArgv[0] != NULL)
  {
    //Rediect output
    if(strcmp(redirectArgv[0], ">")==0)
    {
      fdOut = creat(redirectArgv[1], O_WRONLY | O_CREAT | S_IRWXU);
      if(fdOut == -1)
      {
        printf("Can't open file.\n");
      }
      
      dup2(fdOut, STDOUT_FILENO);
      if(dup2(fdOut, STDOUT_FILENO) < 0)
      {
        printf("Failed redirecting output.\n");
        exit(2);
      }
      
      if(close(fdOut) == -1)
      {
        printf("Failed closing output.\n");
      }
    }
    
    //Redirect input
    if(strcmp(redirectArgv[0], "<")==0)
    {
      fdIn = open(redirectArgv[1], O_RDONLY);
      if(fdIn == -1)
      {
        printf("Can't open file.\n");
      }
      
      dup2(fdIn, STDIN_FILENO);
      if(dup2(fdIn, STDIN_FILENO) < 0)
      {
        printf("Failed redirecting input.\n");
        exit(2);
      }
      
      if(close(fdIn) == -1)
      {
        printf("Failed closing input.\n");
      }
    }
  }
  
  if(exexvp(argv[0], argv) == -1)
  {
    printf("Failed executing command!\n");
  }
}
void SaveHistory(char *historyList[], int &historyIndex, char* command) //luu toan bo lich su command
{
  if(historyCount < MAX_HISTORY_SIZE)
  {
    strcpy(historyList[historyIndex++], command);
  }
  else
  {
    free(historyList[0]);
    for(int i=1; i< MAX_HISTORY_SIZE ; i++)
    {
      historyList[i-1] = historyList[i]; 
    }
    strcpy(historyList[MAX_HISTORY_SIZE - 1], command);
 }
}

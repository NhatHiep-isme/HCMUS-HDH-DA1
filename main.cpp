#include"stdafx.h"
#include <unistd.h>     // fork()
#include <sys/wait.h>   // waitpid()
#include <stdio.h>      // printf(), fgets()
#include <string.h>     // strtok(), strcmp(), strdup()
#include <stdlib.h>     // free()
#include <fcntl.h>      // open(), creat(), close() 
#include "string"

#define MAX_LINE 80
#define BUF_SIZE 50
#define MAX_HISTORY 30
void ParseCommand(char *cmdString, char *argv[], int *waitFlag)
{
	if (cmdString[strlen(cmdString) - 1] == '&')
	{
		*waitFlag = 1;
		cmdString[strlen(cmdString) - 1] = '\0';
	}
	else waitFlag = 0;
	int index = 0;
	char *delim = " ";
	char *tempStr = strtok(cmdString, delim);
	while (tempStr != NULL)
	{
		argv[index] = tempStr;
		tempStr = strtok(NULL, delim);
		index++;
	}
	argv[index] = NULL;
}

void History_Save(char cmdString, int line, char *History_List[]) //luu History_List
{
	if (cmdString[0] != '!') //kiem tra co phai cmd !
	{
		if (*line < MAX_HISTORY) //kiem tra da het mang chua
		{
			strcpy(History_List[*line], cmdString);
			*line++;
		}
		else
		{
			int i;
			for (i = 0; i< MAX_HISTORY - 1; i++)
			{
				strcpy(History_List[i], History_List[i + 1]);
			}
			strcpy(History_List[MAX_HISTORY - 1], cmdString);
		}
	}
}

void History_Load(char *History_List[], int line) //in tung dong cua History_List
{
	for (int i = 0; i < line; i++)
	{
		printf("%d:    %s \n", i, History_List[i]);
	}
}

int Parse_Redir_Or_Pipe(char *argv[], char *argv_head[], char *argv_tail[], char *key)
{
	int pos = -1;//vi tri phan tu >,<,>>,<<,|
	for (int i = 0; i< strlen(*argv); i++)
	{
		if (strcmp(argv[i], ">") == 1 || strcmp(argv[i], "<") == 1 || strcmp(argv[i], "|") == 1)
		{
			pos = i;
			strcpy(key, argv[i]);
			break;
		}
	}
	if (pos = -1) return -1;
	for (int i = 0; i< pos; i++)
	{
		argv_head[i] = argv[i];
	}
	argv_head[pos] = NULL;//cuoi mang argv_head
	for (int i = pos + 1; i< strlen(*argv); i++)
	{
		argv_tail[i - pos - 1] = argv[i];
	}
	argv_tail[strlen(*argv) - pos - 1] = NULL;//cuoi mang argv_tail
	return 1;
}

void exec_Command(char argv[], char argv_head[], char argv_tail[], string key)
{
	int fd_in, fd_out;
	if (strcmp(key, ">") == 1)
	{
		fd_out = creat(argv_tail[0], S_IRWXU);
		if (fd_out == -1)
			exit(EXIT_FAILURE);
		dup2(fd_out, stdout_FILENO);
		if (close(fd_out) == -1)
			exit(EXIT_FAILURE);
	}

	if (strcmp(key, "<") == 1)
	{
		fd_in = open(argv_tail[0], O_RDONLY);
		if (fd_in == -1)
			exit(EXIT_FAILURE);
		dup2(fd_in, stdin_FILENO);
		if (close(fd_out) == -1)
			exit(EXIT_FAILURE);
	}

	if (strcmp(key, ">") == 1 || strcmp(key, "<") == 1)
		execvp(argv_head[0], argv_head);
	else
		execvp(argv[0], argv);
}

void pipe_run(char *argv_head[], char *argv_tail[])
{
	int fd[2];
	if (pipe(fd) == -1)
		exit(EXIT_FAILURE);
	int pid1 = fork();
	if (pid1 == 0)
	{
		//dang o process con
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execvp(argv_head[0], argv_head);

	}
	int pid2 = fork();
	if (pid2 == 0)
	{
		//dang o process con
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		close(fd[1]);
		execvp(argv_tail[0], argv_tail);
	}
	close(fd[0]);
	close(fd[1]);
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
}

int main(){
	bool running = 1;
	char *input[MAX_LINE];               //nhap vao
	char *argv[BUF_SIZE];               	//mang da tach
	char *argv_head[BUF_SIZE];         	//tach lan 2 - phan dau
	char *argv_tail[BUF_SIZE];	       	//tach lan 2 - phan sau
	char *History_List[MAX_HISTORY];    	//danh sach lich su
	int History_line = 0;		       	//bien dem dong lich su
	bool save_History = 1;		//bien luu lich su
	int *waitFlag = 0;			//bien cho
	char *key_cmd;			//bien <,>,<<,>>,|

	pid_t pid;

	while (running)
	{
		printf("osh>");
		fflush(stdout);

		while (fgets(*input, MAX_LINE, stdin) == NULL)
		{
			printf("Cannot read input \n");
			fflush(stdout);
		}

		input[strlen(*input) - 1] = '\0';     //thay '\n' bang '\0'

		if (strcmp(*input, "!!") == 1)
		{
			if (History_line == 0)
				printf("Khong co lich su!! \n");
			else
			{
				strcpy(*input, History_List[History_line]);
			}
			save_History = 0;//khong luu lich su
		}
		else { save_History = 1; }//luu lich su

		if (save_History == 1)
			History_Save(*input, History_line, History_List);

		ParseCommand(*input, argv, waitFlag);
		Parse_Redir_Or_Pipe(argv, argv_head, argv_tail, key_cmd);

		if (strcmp(key_cmd, "|") == 1) //chay pipe
			pipe_run(argv_head, argv_tail);

		if (strcmp(argv[0], "history") == 1 || strcmp(argv[0], "History") == 1)
			History_Load(History_List, History_line);

		pid_t pid = fork();

		switch (pid) {
		case -1:
			exit(EXIT_FAILURE);
			break;
		case 0:
			exec_Command(*argv, *argv_head, *argv_tail, key_cmd);
			break;
		default:
			if (*waitFlag == 1)
				waitpid(pid, NULL, 0);
			break;

		}
		return 0;
	}

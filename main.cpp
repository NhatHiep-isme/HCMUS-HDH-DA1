// SimpleShell123.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <unistd.h>     // thu vien ham fork()
#include <sys/wait.h>   // thu vien ham waitpid()
#include <stdio.h>      // thu vien ham printf(), fgets()
#include <string.h>     // thu vien ham strtok(), strcmp(), strdup()
#include <stdlib.h>     // thu vien ham free()
#include <fcntl.h>      // thu vien ham open(), creat(), close()

using namespace std;
const unsigned MAX_LINE_LENGTH = 100;
const unsigned BUF_SIZE = 50;
const unsigned REDIR_SIZE = 2;
const unsigned PIPE_SIZE = 3;
const unsigned MAX_HISTORY_SIZE = 30;
const unsigned MAX_COMMAND_NAME = 30;

void ParseCmd(char cmdString[], char *argv[], int *waitFlag) {
	for (unsigned idx = 0; idx < BUF_SIZE; idx++) {
		argv[idx] = NULL;
	}

	// kiem tra ki tu ket thuc cua chuoi command 
	if (cmdString[strlen(cmdString) - 1] == '&') {
		*waitFlag = 1;
		cmdString[strlen(cmdString) - 1] = '\0';
	}
	else {
		*waitFlag = 0;
	}

	// Tach chuoi cmd nhap vao thanh cac token
	const char *delim = " ";
	int index = 0;

	char *tempTokenStr = strtok(cmdString, delim);
	while (tempTokenStr != NULL) {
		argv[index++] = tempTokenStr;
		tempTokenStr = strtok(NULL, delim);
	}

	argv[index] = NULL;
}

void SaveHistoryList(char *historyList[], int &historyIndex, char* cmdString) {
	// Neu history_count vuot qua MAX_HISTORY_SIZE, hay ghi de lenh cuoi cung

	if (historyIndex < MAX_HISTORY_SIZE) {
		strcpy(historyList[historyIndex++], cmdString);
	}
	else {
		free(historyList[0]);//Giai phong command dau tien

		for (int i = 1; i < MAX_HISTORY_SIZE; i++) {
			strcpy(historyList[i - 1], historyList[i]);
		}

		strcpy(historyList[MAX_HISTORY_SIZE - 1], cmdString);
	}
}

void ParseRedirect(char *argv[], char *redirArgv[]) {
	int index = 0;
	redirArgv[0] = NULL;
	redirArgv[1] = NULL;

	while (argv[index] != NULL) {

		// kiem tra xem command co chua ki tu > hay <
		if (strcmp(argv[index], "<") == 0 || strcmp(argv[index], ">") == 0) {

			// kiem tra ten file hop le
			if (argv[index + 1] != NULL) {

				redirArgv[0] = strdup(argv[index]);
				redirArgv[1] = strdup(argv[index + 1]);
				argv[index] = NULL;
				argv[index + 1] = NULL;
			}
		}

		index++;
	}
}

void ChildProcess(char* argv[], char* redirArgv[]) {
	int fdOut, fdIn;
	if (redirArgv[0] != NULL) {

		// Redirect output
		if (strcmp(redirArgv[0], ">") == 0) {

			// lay file description
			fdOut = creat(redirArgv[1], S_IRWXU);
			if (fdOut == -1) {
				printf("Redirect output failed");
				exit(EXIT_FAILURE);
			}

			// thay the stdout -> output
			dup2(fdOut, STDOUT_FILENO);

			// kiem tra loi(khi dong chuong trỉnh)
			if (close(fdOut) == -1) {
				printf("Closing output failed");
				exit(EXIT_FAILURE);
			}
		}

		// Redirect input
		else if (strcmp(redirArgv[0], "<") == 0) {
			fdIn = open(redirArgv[1], O_RDONLY);
			if (fdIn == -1) {
				printf("Redirect input failed");
				exit(EXIT_FAILURE);
			}

			dup2(fdIn, STDIN_FILENO);

			if (close(fdIn) == -1) {
				printf("Closing input failed");
				exit(EXIT_FAILURE);
			}
		}
	}

	//Thuc thi lenh cua user nhap vao trong tien trinh con
	if (execvp(argv[0], argv) == -1) {
		printf("Fail to execute command");
		exit(EXIT_FAILURE);
	}
}


void ParentProcess(pid_t childPid, int waitFlag) {
	int status;
	printf("Parent <%d> spawned a child <%d>.\n", getpid(), childPid);
	switch (waitFlag) {

		// Hai tien trinh cha va con cung luc chay
	case 0: {
		waitpid(childPid, &status, 0);
		break;
	}

		// Tien trinh cha doi tien trinh con ( with PID) cham dut
	default: {
		waitpid(childPid, &status, WUNTRACED);

		if (WIFEXITED(status)) {
			printf("Child <%d> exited with status = %d.\n", childPid, status);
		}
		break;
	}
	}
}

//ham xu li pipe
bool ParsePipe(char* argv[], char *childArgv1[], char *childArgv2[]) {
	int index = 0, splitIndex = 0;
	bool containsPipe = false;
	int count = 0;

	while (argv[index] != NULL) {

		// Kiem tra xem user command co bao gom ki tu pipe "|"
		if (strcmp(argv[index], "|") == 0) {
			splitIndex = index;
			containsPipe = true;
		}
		index++;
	}

	if (!containsPipe) {
		return false;
	}

	//Copy cac tham so truoc khi tach pipe vao bien childArgv1[]
	for (index = 0; index < splitIndex; index++) {
		childArgv1[index] = strdup(argv[index]);
	}
	childArgv1[index++] = NULL;

	// Copy cac tham so sau khi tach pipe vao bien childArgv2[]
	while (argv[index] != NULL) {
		childArgv2[index - splitIndex - 1] = strdup(argv[index]);
		index++;
	}
	childArgv2[index - splitIndex - 1] = NULL;

	return true;
}



void ExecWithPipe(char* childArgv1[], char* childArgv2[]) {
	int pipefd[2];

	if (pipe(pipefd) == -1) {
		//Tao 1 pipe voi 1 input va 1 output (file descriptor)
                //Index = 0 -> read pipe, Index = 1 -> write pipe
		printf("pipe() failed");
		exit(EXIT_FAILURE);
	}

	// Tao tien trinh con thu nhat (child 1)  
	if (fork() == 0) {

		// Redirect STDOUT to output part of pipe 
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);

		execvp(childArgv1[0], childArgv1);
		printf("Fail to execute first command");
		exit(EXIT_FAILURE);
	}

	// Tao tien trinh con thu hai(child 2)
	if (fork() == 0) {

		// Redirect STDIN to input part of pipe
		dup2(pipefd[0], STDIN_FILENO);
		close(pipefd[1]);
		close(pipefd[0]);

		execvp(childArgv2[0], childArgv2);
		printf("Fail to execute second command");
		exit(EXIT_FAILURE);
	}

	close(pipefd[0]);
	close(pipefd[1]);
	// Wait for child 1
	wait(0);
	// Wait for child 2
	wait(0);
}

int main() {
	bool running = true;
	pid_t pid;
	int status = 0, historyIndex = 0, waitFlag;
	char cmdString[MAX_LINE_LENGTH];
	char *argv[BUF_SIZE], *redirArgv[REDIR_SIZE];
	char *childArgv1[PIPE_SIZE], *childArgv2[PIPE_SIZE];
	char *historyList[MAX_HISTORY_SIZE];

	for (int i = 0; i < MAX_HISTORY_SIZE; i++) {
		historyList[i] = (char*)malloc(MAX_COMMAND_NAME);
	}

	while (running) {
		printf("osh>");
		fflush(stdout);

		// Doc va tach, chuyen doi user command
		while (fgets(cmdString, MAX_LINE_LENGTH, stdin) == NULL) {
			printf("Cannot read user input!");
			fflush(stdin);
		}

		// Bo di ki tu ket thuc chuoi
		cmdString[strcspn(cmdString, "\n")] = '\0';

		// Kiem tra xem user co nham command: "exit" khong?
		if (strcmp(cmdString, "exit") == 0) {
			running = false;
			continue;
		}

		// Kiem tra xem user co nham command: "!!" ?
		if (strcmp(cmdString, "!!") == 0) {
			if (historyIndex == 0) {
				fprintf(stderr, "No commands in history before\n");
				continue;
			}
			strcpy(cmdString, historyList[historyIndex - 1]);
			printf("HiepNe>%s\n", cmdString);
		}

		SaveHistoryList(historyList, historyIndex, cmdString);
		ParseCmd(cmdString, argv, &waitFlag);
		ParseRedirect(argv, redirArgv);

		if (ParsePipe(argv, childArgv1, childArgv2)) {
			ExecWithPipe(childArgv1, childArgv2);
			continue;
		}

		// Fork child process
		pid_t pid = fork();

		// Fork tra ve 2 lan neu thanh cong: 0 - child process, > 0 - parent process
		switch (pid) {
		case -1:
			printf("fork() failed!");
			exit(EXIT_FAILURE);

		case 0:     // trong tien trinh con
			ChildProcess(argv, redirArgv);
			exit(EXIT_SUCCESS);

		default:    // Trong tien trinh cha
			ParentProcess(pid, waitFlag);
		}

	}
	return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

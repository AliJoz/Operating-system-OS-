#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
using namespace std;
bool Stop_Token = false;

#define RULE_FILE "input.txt"
#define MAX_TASKS 1000
#define	MAX_WORKERS 100

HANDLE R = CreateSemaphore(NULL, 1, 1, NULL);
HANDLE RW = CreateSemaphore(NULL, 1, 1, NULL);
HANDLE Q = CreateSemaphore(NULL, 1, 1, NULL);
HANDLE TryWrite = CreateSemaphore(NULL, 1, 1, NULL);
HANDLE TryRead = CreateSemaphore(NULL, 0, 1, NULL);

int readCount = 0;
int bestResult = MAXINT32;
int bestThreadId = MAXINT32;
int bestAssign[MAX_TASKS];
int numTasks;
int numWorkers;
int Tasks[MAX_WORKERS][MAX_TASKS];
int TasksTimes[MAX_TASKS];

int runtime = 5; //second
DWORD Start;

int main();
DWORD WINAPI Operator(LPVOID Param);
void ReadRulesFromFile();
int StartProcess(int assignedTasks[]);



int main()
{
	ReadRulesFromFile();

	DWORD numprocess = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);

	Start = GetTickCount64();

	HANDLE* handles = new HANDLE[numprocess];
	for (int i = 0; i < numprocess; i++)
	{
		handles[i] = CreateThread(NULL, 0, Operator, NULL, 0, NULL);
	}

	while (!Stop_Token)
	{
		WaitForSingleObject(TryRead, INFINITE);
		if (Stop_Token)
			break;

		cout << "\n******************************\n"
			<< "Thread ID: " << bestThreadId <<endl
			<< "Best Result: " << bestResult <<endl;

		cout << "\ntime: " << (GetTickCount64() - Start) / 1000;

		ReleaseSemaphore(TryWrite, 1, NULL);
	}

	//WaitForMultipleObjects(numprocess, handles, true, INFINITE);

	DWORD end = GetTickCount64();

	for (int i = 0; i < numprocess; i++)
	{
		CloseHandle(handles[i]);
	}

	//Display Results..
	cout << "\n****** RESULTS ******\n";
	cout << "Process Time: " << (end - Start) / 1000 << " Seconds \n\n";

	cout << "\n\nResult of ThreadID: " << bestThreadId << " \nresult: " << bestResult;
	cout << "\n   Best Result: ";
	for (int j = 0; j < numTasks; j++)
	{
		cout << j << " -> " << bestAssign[j] <<endl;
	}

	return 0;
}

DWORD WINAPI Operator(LPVOID Param)
{
	int ThreadID = GetCurrentThreadId();
	srand(time(NULL) + ThreadID);
	int cachedGlobalBestResult = bestResult;

	while (!Stop_Token)
	{
		int assign[MAX_TASKS];
		int result = StartProcess(assign);

		if (result < cachedGlobalBestResult)
		{
			WaitForSingleObject(Q, INFINITE);
			WaitForSingleObject(R, INFINITE);
			if (readCount == 0)
				WaitForSingleObject(RW, INFINITE);
			readCount++;
			ReleaseSemaphore(Q, 1, NULL);
			ReleaseSemaphore(R, 1, NULL);
			//Read Data
			cachedGlobalBestResult = bestResult;

			WaitForSingleObject(R, INFINITE);
			readCount--;
			if (!readCount)
				ReleaseSemaphore(RW, 1, NULL);
			ReleaseSemaphore(R, 1, NULL);

			if (result < cachedGlobalBestResult) {
				WaitForSingleObject(Q, INFINITE);
				WaitForSingleObject(RW, INFINITE);
				ReleaseSemaphore(Q, 1, NULL);
				if (result < bestResult && !Stop_Token)
				{
					WaitForSingleObject(TryWrite, INFINITE);
					bestResult = result;
					bestThreadId = ThreadID;
					for (int i = 0; i < numTasks; i++)
					{
						bestAssign[i] = assign[i];
					}
				ReleaseSemaphore(TryRead, 1, NULL);
				}
				ReleaseSemaphore(RW, 1, NULL);
			}
		}
		if ((GetTickCount64() - Start) / 1000 > runtime)
		{
			Stop_Token = true;
			ReleaseSemaphore(TryRead, 1, NULL);
		}
	}

	//Display End Message
	string start_Message = "\nThread Ended- ";
	start_Message.append("ThreadID: ");
	start_Message.append(to_string(ThreadID));
	start_Message.append("\n");
	cout << start_Message;
	return 0;
}

void ReadRulesFromFile()
{
	fstream file("input.txt");
	if (!file) {
		cerr << "Failed to open input file!" <<endl;
		exit(-1);
	}

	file >> numTasks >> numWorkers;

	for (int i = 0; i < numTasks; i++) {
		file >> TasksTimes[i];
	}

	for (int i = 0; i < numTasks; i++) {
		for (int j = 0; j < numWorkers; j++) {
			file >> Tasks[i][j];
		}
	}
	file.close();
}

int StartProcess(int assignedTasks[])
{
	int assignedWorkers[MAX_WORKERS] = { 0 };

	int max = INT_MIN, min = INT_MAX;

	// Random allocation
	for (int j = 0; j < numTasks; ) {
		int randWorker = rand() % numWorkers;
		if (Tasks[j][randWorker] == 1) {
			assignedTasks[j] = randWorker;
			assignedWorkers[randWorker] += TasksTimes[j];
			j++;
		}
	}

	// Find max and min workloads
	for (int i = 0; i < numWorkers; i++) {
		if (assignedWorkers[i] > max) max = assignedWorkers[i];
		if (assignedWorkers[i] < min) min = assignedWorkers[i];
	}
	return max - min;
}

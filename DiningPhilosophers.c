#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> //for I/O
#include <stdlib.h> //for rand/strand
#include <windows.h> //for Win32 API
#include <time.h>    //for using the time as the seed to strand!

//5 Dining Philosophers
#define N 5

//Each Philosopher is Philosophising 5 rounds
#define MAX_PHILOSOPHY_ROUNDS 5

#define RIGHT (philID+1)%N
#define LEFT (philID+N-1)%N

//Every Philosopher waits random time between 1 and 500 msec
//before it takes forks and before it puts forks
#define MAX_SLEEP_TIME 500 //Miliseconds (1/2 second)

#define True 1
#define False 0

//The possible states of a Philosopher (enumeration)
enum PHIL_STATES { THINKING, HUNGRY, EATING };

///Global State Data:

//state array, holds current state of each Philosopher in the System
int state[N];

//single mutex to protect access to the global state data. Recall, like in 
//lecture notes, global state data must be accessed exclusively by each 
//Philosopher!
HANDLE mutex;

//Array of Scheduling Constraints Semaphores - one for Each Philosopher
HANDLE sem[N];

//Function to Initialise and clean global data (mainly for creating the 
//Semaphore Haldles before all Threads start and closing them properly after all Threads finish!). 
int initGlobalData();
void cleanupGlobalData();

//Thread function for each Philosopher
DWORD WINAPI Philosopher(PVOID);

//better to write a generic function to randomise a Sleep time between 1 and 
//MAX_SLEEP_TIME msec
int calcSleepTime();

//Functions to manipulate Philosopher State - as defined in lecture notes
void take_forks(int);
void put_forks(int);
void test(int);



int main(int argc, char *argv[])
{
	DWORD temp;
	HANDLE phils[N];
	int philID[N];

	if (!initGlobalData())
	{
		printf("big balagan. bye bye");
		exit(0);
	}


	
	for (int i = 0; i < N; i++)
	{
		philID[i] = i;
	}
	for (int i = 0; i < N; i++)
	{
	
		phils[i] = CreateThread(NULL, 0, Philosopher, &philID[i], NULL, &temp);
		if (phils[i] == NULL)
		{
			printf("balagan");
		}
	}

	WaitForMultipleObjects(N, phils, TRUE, INFINITE);

	printf("all phils finish\n");
	for (int i = 0; i < N; i++)
	{
		CloseHandle(phils[i]);
	}
	cleanupGlobalData();

	system("pause");
}

void take_forks(int philID)
{

	WaitForSingleObject(mutex,INFINITE);
	state[philID] = HUNGRY;
	printf("philID %d: HUNGRY\n", philID);
	test(philID);
	if (!ReleaseMutex(mutex))
	{
		printf("take_forks::Unexpected error mutex.\n");
	}
	WaitForSingleObject(sem[philID], INFINITE);

}

void put_forks(int philID)
{

	WaitForSingleObject(mutex, INFINITE);
	state[philID] = THINKING;
	printf("philID %d: THINKING\n", philID);
	test(LEFT);
	test(RIGHT);
	if (!ReleaseMutex(mutex))
	{
		printf("put_forks::Unexpected error mutex.\n");
	}

}

void test(int philID)
{

	if (state[philID] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
	{
		state[philID] = EATING;
		printf("philID %d: EATING\n", philID);
		if (!ReleaseSemaphore(sem[philID], 1, NULL))
			printf("test::Unexpected error sem[philID].V()\n");
	}
}

int initGlobalData()
{
	mutex = CreateMutex(NULL, FALSE, NULL);
	for (int i = 0; i < N; i++)
	{
		state[i] = THINKING;

	}
	for (int i = 0; i < N; i++)
	{

		sem[i] = CreateSemaphore(NULL, 0, 1, NULL);
	}

	return TRUE;
}

void cleanupGlobalData()
{
	CloseHandle(mutex);

	for (int i = 0; i < N; i++)
	{
		CloseHandle(sem[i]);
	}
}

int calcSleepTime()
{
	int sleep = rand() % MAX_SLEEP_TIME + 1;
	return sleep;
}

DWORD WINAPI Philosopher(PVOID Param)
{


	int philID = *(int*)Param;

	for (int i = 0; i < MAX_PHILOSOPHY_ROUNDS; i++)
	{

		Sleep(calcSleepTime());
		take_forks(philID);
		Sleep(calcSleepTime());
		put_forks(philID);

	}

	return 0;
}
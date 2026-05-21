#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "../engine/db.h"

#define KSIZE (16)
#define VSIZE (1000)

#define NUM_THREADS (8) // NOTE: setting number of threads

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

typedef struct args{ // NOTE: struct for passing arguments to wrapper functions of kiwi.c
	char *k;
	char *v;
	long int c;
	int ran;
	int tid;
	int nthreads;
	int f;
	int wr;
	int res;
	int per_read;
	int comprand;
	int searches;
	DB *base;
}Arguments;

double get_ustime_sec(void);
void _random_key(char *key,int length);

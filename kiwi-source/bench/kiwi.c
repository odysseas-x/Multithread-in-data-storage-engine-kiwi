#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb")

void* threads_adding(void* arg) // NOTE: wrapper function for pthread_create calling the function db_add with arguments passing as a struct
{
	int j;
	Variant sk, sv;
	Arguments *args = (Arguments *)arg;
	for(j=args->tid; j<args->c; j+=args->nthreads)
	{
		if(args->ran)
			_random_key(args->k, KSIZE);
		else
			snprintf(args->k, KSIZE, "key-%d", j);
		fprintf(stderr, "%d adding %s\n", j, args->k);
		snprintf(args->v, VSIZE, "val-%d", j);

		sk.length = KSIZE;
		sk.mem = args->k;
		sv.length = VSIZE;
		sv.mem = args->v;
		db_add(args->base, &sk, &sv);
		if((j % 10000) == 0){
			fprintf(stderr,"random write finished %d ops%30s\r", j, "");
			fflush(stderr);
		}
	}
	return NULL;
}

void _write_test(long int count, int r)
{
	int i;
	double cost;
	//Variant sk, sv;
	double start,end;
	DB* db;

	pthread_t tid[NUM_THREADS]; // NOTE: declaration of NUM_THREADS threads
	Arguments argument[NUM_THREADS]; // NOTE: declaration of our Arguments struct for passing arguments of each thread to wrapper function threads_adding
	
	//char key[KSIZE + 1];
	//char val[VSIZE + 1];
	char sbuf[1024];

	//memset(key, 0, KSIZE + 1);
	//memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	db = db_open(DATAS);

	start = get_ustime_sec();


	for(i=0; i<NUM_THREADS; i++) // NOTE: passing arguments for each thread in struct
	{
		argument[i].k=malloc(KSIZE+1);
		memset(argument[i].k, 0, KSIZE+1);
		argument[i].v=malloc(VSIZE+1);
		memset(argument[i].v, 0, VSIZE+1);
		argument[i].c=count;
		argument[i].ran=r;
		argument[i].base=db;
		argument[i].tid=i;
		argument[i].nthreads=NUM_THREADS;
		pthread_create(&tid[i], NULL, threads_adding, &argument[i]); // NOTE: creating NUM_THREADS threads and calling threads_adding with arguments
	}


	for(i=0; i<NUM_THREADS; i++) // NOTE: main thread waiting all threads in join to finish
	{
		pthread_join(tid[i], NULL);
		free(argument[i].k);
		free(argument[i].v);
	}



	/*for (i = 0; i < count; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;					// source code 
		sv.length = VSIZE;
		sv.mem = val;
		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}*/

	db_close(db);

	end = get_ustime_sec();
	cost = end -start;

	printf(LINE);
	printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
		,count, (double)(cost / count)
		,(double)(count / cost)
		,cost);	
}


void* threads_getting(void* arg) // NOTE: wrapper function for pthread_create calling the db_get function with arguments passing as a struct
{
	int j;
	Variant sk,sv;
	Arguments *args = (Arguments *)arg;
	for(j=args->tid; j<args->c; j+=args->nthreads)
	{
		memset(args->k, 0, KSIZE+1);
		if(args->ran)
			_random_key(args->k, KSIZE);
		else
			snprintf(args->k, KSIZE, "key-%d", j);
		fprintf(stderr, "%d searching %s\n", j, args->k);
		sk.length = KSIZE;
		sk.mem = args->k;
		args->res = db_get(args->base, &sk, &sv);
		if(args->res){
			(args->f)++;
		} else {
			INFO("not found key#%s", sk.mem);
		}
		if((j % 10000) == 0){
			fprintf(stderr,"random read finished %d ops%30s\r", j, "");
			fflush(stderr);
		}

	}
	return NULL;
}


void _read_test(long int count, int r)
{
	int i;
	//int ret;
	int found = 0;
	double cost;
	double start,end;
	//Variant sk;
	//Variant sv;
	DB* db;
	//char key[KSIZE + 1];

	pthread_t tid[NUM_THREADS]; // NOTE: declaration of NUM_THREADS threads
	Arguments argument[NUM_THREADS]; // NOTE: declaration of our Arguments struct for passing arguments of each thread to wrapper function threads_getting

	db = db_open(DATAS);
	start = get_ustime_sec();


	for(i=0; i<NUM_THREADS; i++) // NOTE: passing arguments for each thread in struct
	{
		argument[i].k=malloc(KSIZE+1);
		argument[i].c=count;
		argument[i].ran=r;
		argument[i].base=db;
		argument[i].tid=i;
		argument[i].f=found;
		argument[i].nthreads=NUM_THREADS;
		pthread_create(&tid[i], NULL, threads_getting, &argument[i]); // NOTE: creating NUM_THREADS threads and calling threads_getting with arguments
	}

	for(i=0; i<NUM_THREADS; i++) // NOTE: main thread waiting for all threads to finish in join
	{
		pthread_join(tid[i], NULL);
		found+=argument[i].f; // NOTE: computing the total amount of keys found 
		free(argument[i].k);
	}

	/*for (i = 0; i < count; i++) {
		memset(key, 0, KSIZE + 1);

		// if you want to test random write, use the following 
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(db, &sk, &sv);			// source code 
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}*/

	db_close(db);

	end = get_ustime_sec();
	cost = end - start;
	printf(LINE);
	printf("|Random-Read	(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
		count, found,
		(double)(cost / count),
		(double)(count / cost),
		cost);
}

void* threads_readwrite(void* arg) // NOTE: wrapper function for pthread_create calling either db_add or db_get depending
				   // on a possibility combined with user's % of reads request, all arguments passed as a struct  
{
	int j;
	Variant sk,sv;
	Arguments *args = (Arguments *)arg;
	for(j=args->tid; j<args->c; j+=args->nthreads)
	{
		args->comprand = rand() % 100; // NOTE: getting a number between 0-99 as a possibility
		if(args->per_read > args->comprand) // NOTE: if %reads > possibility, we have a reader
		{
			// READ
			memset(args->k, 0, KSIZE+1);
			if(args->ran)
				_random_key(args->k, KSIZE);
			else
				snprintf(args->k, KSIZE, "key-%d", j);
			fprintf(stderr, "%d searching %s\n", j, args->k);
			sk.length = KSIZE;
			sk.mem = args->k;
			args->res = db_get(args->base, &sk, &sv);
			args->searches++; // NOTE: counting each thread's searches even if key not found
			if(args->res)
				args->f++;
			else
				INFO("not found key#%s", sk.mem);
			if((j % 10000) == 0){
				fprintf(stderr,"random read finished %d ops%30s\r", j, "");
				fflush(stderr);
			}
		}
		else
		// WRITE
		{
			if(args->ran)
				_random_key(args->k, KSIZE);
			else
				snprintf(args->k, KSIZE, "key-%d", j);
			fprintf(stderr, "%d searching %s\n", j, args->k);
			sk.length = KSIZE;
			sk.mem = args->k;
			sv.length = VSIZE;
			sv.mem = args->v;
			db_add(args->base, &sk, &sv);
			args->wr++;
			if((j % 10000) == 0){
				fprintf(stderr,"random read finished %d ops%30s\r", j, "");
				fflush(stderr);
			}
		}

	}
	return NULL;
}

void _readwrite_test(long int count, int reads, int r) // NOTE: function used for readwrite request
						       // passing total keys, %reads and random keys request
	// NOTE: logic same as in _read_test, _write_test
{
	int i, found=0, written=0, searches=0; // NOTE: counting total keys written, searched and found
	double cost;
	double start, end;
	DB* db;

	pthread_t tid[NUM_THREADS];
	Arguments argument[NUM_THREADS];
	
	char sbuf[1024];
	memset(sbuf, 0, 1024);

	db = db_open(DATAS);

	start = get_ustime_sec();

	for(i=0; i<NUM_THREADS; i++) // NOTE: passing arguments for each thread in struct
	{
		argument[i].k=malloc(KSIZE+1);
		memset(argument[i].k, 0, KSIZE+1);
		argument[i].v=malloc(VSIZE+1);
		memset(argument[i].v, 0, VSIZE+1);
		argument[i].c=count;
		argument[i].ran=r;
		argument[i].base=db;
		argument[i].tid=i;
		argument[i].nthreads=NUM_THREADS;
		argument[i].f=found;
		argument[i].wr=written;
		argument[i].per_read=reads;
		argument[i].searches=searches;
		pthread_create(&tid[i], NULL, threads_readwrite, &argument[i]);
	}

	for(i=0; i<NUM_THREADS; i++) // NOTE: main thread waiting for each thread to finish in join
	{
		pthread_join(tid[i], NULL);
		found+=argument[i].f; // NOTE: computing total amount of keys found
		written+=argument[i].wr; // total amount of keys written
		searches+=argument[i].searches; // total amount of keys searched 
		free(argument[i].k);
		free(argument[i].v);
	}

	db_close(db);

	end = get_ustime_sec();
	cost = end - start;
	printf(LINE);
	printf("Readwrite    (written:%d, read:%d, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n", written, searches, found, (double)(cost / count), (double)(count / cost), cost);
}

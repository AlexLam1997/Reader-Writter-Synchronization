#include <stdio.h>
#include <semaphore.h>
#include <pthread.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define w_count 10
#define r_count 500

int print_read = 0;

sem_t rw_mutex;
sem_t mutex;
int current_readers = 0;
int shared_int = 0;
float writer_max_wait;
float reader_max_wait;
float writer_min_wait = 70000;
float reader_min_wait = 70000;
float writer_total_wait;
float reader_total_wait;

void nighttime(int wr_switch){
    int r = rand() % 100;
    if(wr_switch == 1 && print_read != 0){
        printf("Reader sleeping for %d milliseconds \n", r);
    }else if (print_read !=0){
        printf("Writer sleeping for %d milliseconds \n", r);
    }
    usleep(1000 * r);
}

void* reader(void * arg){
    int reader_retries = *((int*) arg);
    clock_t start_t, total_t;
    int retry_count = 0;
    do {
        nighttime(1);
        start_t = clock();
        retry_count++;
        sem_wait(&mutex);
        current_readers++;
        if (current_readers == 1)
            sem_wait(&rw_mutex);
        sem_post(&mutex);
        
        total_t = clock() - start_t;
        if(total_t < reader_min_wait){
            reader_min_wait = total_t;
        }else if(total_t > reader_max_wait){
            reader_max_wait = total_t;
        }
        reader_total_wait += total_t;
        // read is performed
        printf("Read: %d \n", shared_int);

        sem_wait(&mutex);
        current_readers--;
        if (current_readers == 0)
        sem_post(&rw_mutex);
        sem_post(&mutex);
    } while (retry_count <= reader_retries-1);
}

void* writer(void *arg){
    int writer_retries = *((int*) arg);
    clock_t start_t, total_t;
    int retry_count = 0;
    do{
        nighttime(2);
        retry_count++;
        start_t = clock();
        sem_wait(&rw_mutex);
        shared_int += 10;
        printf("Write: %d \n", shared_int);
        sem_post(&rw_mutex);

        total_t = clock() - start_t;
        if(total_t < writer_min_wait){
            writer_min_wait = total_t;
        }else if(total_t > writer_max_wait){
            writer_max_wait = total_t;
        }
        writer_total_wait += total_t;
    } while(retry_count <= writer_retries-1);
}

int main(int argc, char *argv[])
{
    int writter_count = w_count; 
    int reader_count = r_count; 
    int writter_retries = atoi(argv[1]); 
    int reader_retries = atoi(argv[2]);
    
    // char line[25];
    // printf("Writter retry count: ");
    // fgets (line, sizeof(line), stdin);
    // sscanf (line, "%d", &writter_retries);

    // printf("Reader retry count: ");
    // fgets (line, sizeof(line), stdin);
    // sscanf (line, "%d", &reader_retries);

    // printf("Writter retry count: %d \n", writter_retries);
    // printf("Reader retry count: %d \n", reader_retries);

    // 0 value for pshared means shared by threads 
    sem_init(&rw_mutex, 0, 1); 
    sem_init(&mutex, 0, 1);    

    pthread_t writers[writter_count]; 
    pthread_t readers[reader_count]; 

    for (int count = 0; count < writter_count; ++count){
        if (pthread_create(&writers[count], NULL, writer, &writter_retries) != 0)
        {
          fprintf(stderr, "error: Cannot create thread # %d\n", count);
          break;
        }
    }

    for (int count = 0; count < reader_count; ++count){
        if (pthread_create(&readers[count], NULL, reader, &reader_retries) != 0)
        {
          fprintf(stderr, "error: Cannot create thread # %d\n", count);
          break;
        }
    }

    for (int i = 0; i < writter_count; ++i)
    {
      if (pthread_join(writers[i], NULL) != 0)
        {
          fprintf(stderr, "error: Cannot join writer thread # %d\n", i);
        }
    }

    for (int i = 0; i < reader_count; ++i)
    {
      if (pthread_join(readers[i], NULL) != 0)
        {
          fprintf(stderr, "error: Cannot join reader thread # %d\n", i);
        }
    }
    printf("----------------------------------- \n");
    printf("Number of writters: %d \n", w_count);
    printf("Number of readers: %d \n", r_count);
    printf("Writter retry count: %d \n", writter_retries);
    printf("Reader retry count: %d \n", reader_retries);
    printf("Min writer wait time: %f milliseconds\n", writer_min_wait);
    printf("Min reader wait time: %f milliseconds\n", reader_min_wait);
    printf("Max writer wait time: %f milliseconds\n", writer_max_wait);
    printf("Max reader wait time: %f milliseconds\n", reader_max_wait);
    printf("Average writer wait time: %f milliseconds\n", writer_total_wait/(w_count*writter_retries));
    printf("Average reader wait time: %f milliseconds\n", reader_total_wait/(r_count*reader_retries));
    printf("Total writer wait time: %f milliseconds\n", writer_total_wait);
    printf("Total reader wait time: %f milliseconds\n", reader_total_wait);
}
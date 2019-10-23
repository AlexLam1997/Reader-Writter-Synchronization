#include <stdio.h>
#include <semaphore.h>
#include <pthread.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define w_count 5
#define r_count 900

int print_read = 0;

sem_t rw_mutex;
sem_t mutex;

int current_readers = 0, shared_int = 0;
float writer_max_wait, reader_max_wait, writer_total_wait, reader_total_wait;
float writer_min_wait = 10000;
float reader_min_wait = 10000;

float rmin_avg, wmin_avg, rmax_avg, wmax_avg; 

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
        retry_count++;
        start_t = clock();
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

        // Read
        if (print_read == 1) printf("Read: %d \n", shared_int);

        sem_wait(&mutex);
        current_readers--;
        if (current_readers == 0)
            sem_post(&rw_mutex);
        sem_post(&mutex);

        // Sleep
        nighttime(1);
    } while (retry_count <= reader_retries-1);
}

void* writer(void *arg){
    int writer_retries = *((int*) arg);
    clock_t start_t, total_t;
    int retry_count = 0;
    do{
        retry_count++;
        start_t = clock();
        sem_wait(&rw_mutex);
        // Measure waiting time
        total_t = clock() - start_t;
        if(total_t < writer_min_wait){
            writer_min_wait = total_t;
        }else if(total_t > writer_max_wait){
            writer_max_wait = total_t;
        }
        writer_total_wait += total_t;

        // Write
        shared_int += 10;

        if (print_read==1) printf("Write: %d \n", shared_int);

        // Release
        sem_post(&rw_mutex);

        nighttime(2);
    } while(retry_count <= writer_retries-1);
}

int main(int argc, char *argv[])
{
    int writter_count = w_count; 
    int reader_count = r_count; 
    int writter_retries = atoi(argv[1]); 
    int reader_retries = atoi(argv[2]);
    
    // run it multiple times to get averages wait time values
    int average_count = 10;
    for(int i = 0; i < average_count; i++ ){
        if (i == 1) print_read = 0;
        printf("%d \n", i);
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

        for (int i = 0; i < writter_count; ++i){
        if (pthread_join(writers[i], NULL) != 0)
            {
            fprintf(stderr, "error: Cannot join writer thread # %d\n", i);
            }
        }

        for (int i = 0; i < reader_count; ++i){
        if (pthread_join(readers[i], NULL) != 0)
            {
            fprintf(stderr, "error: Cannot join reader thread # %d\n", i);
            }
        }
        sem_destroy(&rw_mutex);
        sem_destroy(&mutex);

        rmin_avg+= reader_min_wait;
        wmin_avg+= writer_min_wait;
        rmax_avg+= reader_max_wait;
        wmax_avg+= writer_max_wait;

        // reset 
        reader_min_wait=10000;
        writer_min_wait=10000;
        reader_max_wait=0;
        writer_max_wait=0;
    }

    printf("----------------------------------- \n");
    printf("Averaged results over %d runs: \n", average_count);
    printf("Number of writters: %d \n", w_count);
    printf("Number of readers: %d \n", r_count);
    printf("Writter retry count: %d \n", writter_retries);
    printf("Reader retry count: %d \n \n", reader_retries);

    printf("Min reader wait time: %f milliseconds\n", rmin_avg/average_count);
    printf("Max reader wait time: %f milliseconds\n", rmax_avg/average_count);
    printf("Average reader wait time: %f milliseconds\n", reader_total_wait/(r_count*reader_retries *average_count));
    printf("Total reader wait time: %f milliseconds\n \n", reader_total_wait/average_count);

    printf("Min writer wait time: %f milliseconds\n", wmin_avg/average_count);
    printf("Max writer wait time: %f milliseconds\n", wmax_avg/average_count);
    printf("Average writer wait time: %f milliseconds\n", writer_total_wait/(w_count*writter_retries*average_count));
    printf("Total writer wait time: %f milliseconds\n", writer_total_wait/average_count);
}
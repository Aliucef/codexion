// #include <stdio.h>
// #include <pthread.h>
// #include <unistd.h>

// // Shared resources
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
// int data_ready = 0;

// void* producer(void* arg) { // arg is used here because the pthread_create third argument requires a pointer to a function with this specific type matching
//     sleep(2); // Simulate work

//     pthread_mutex_lock(&mutex);
//     data_ready = 1;
//     printf("Producer: Data is ready. Signaling...\n");

//     // Wake up the waiting thread
//     pthread_cond_signal(&cond);
//     pthread_mutex_unlock(&mutex);

//     return NULL;
// }

// void* consumer(void* arg) {
//     pthread_mutex_lock(&mutex);

//     // Always use a 'while' loop to check the condition to handle "spurious wakeups"
//     while (data_ready == 0) {
//         printf("Consumer: Waiting for data...\n");
//         // Atomically releases mutex and sleeps until signaled
//         pthread_cond_wait(&cond, &mutex);
//     }

//     printf("Consumer: Data received! Processing...\n");
//     pthread_mutex_unlock(&mutex);

//     return NULL;
// }

// int main() {
//     pthread_t t1, t2; // creating 2 threads this basically stores the thread id only

//     pthread_create(&t1, NULL, consumer, NULL);
//     pthread_create(&t2, NULL, producer, NULL);

//     pthread_join(t1, NULL);
//     pthread_join(t2, NULL);

//     // Clean up
//     pthread_mutex_destroy(&mutex);
//     pthread_cond_destroy(&cond);

//     return 0;
// }

// // pthread_create(thread of type pthread_t *, attr almost always set to NULL , start_routine(function pointer) aka the task to do , arg (the data) type void* the input u wanna pass into the thread function or NULL)

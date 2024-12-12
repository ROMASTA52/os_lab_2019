#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Два мьютекса для демонстрации deadlock
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void *thread1_function(void *arg) {
    printf("Thread 1: locking mutex1...\n");
    pthread_mutex_lock(&mutex1);
    sleep(1); // Имитируем выполнение задачи
    printf("Thread 1: trying to lock mutex2...\n");
    pthread_mutex_lock(&mutex2);

    printf("Thread 1: acquired both mutexes!\n");

    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

void *thread2_function(void *arg) {
    printf("Thread 2: locking mutex2...\n");
    pthread_mutex_lock(&mutex2);
    sleep(1); // Имитируем выполнение задачи
    printf("Thread 2: trying to lock mutex1...\n");
    pthread_mutex_lock(&mutex1);

    printf("Thread 2: acquired both mutexes!\n");

    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);

    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Создаем два потока
    pthread_create(&thread1, NULL, thread1_function, NULL);
    pthread_create(&thread2, NULL, thread2_function, NULL);

    // Ожидаем завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Program finished successfully.\n");

    return 0;
}

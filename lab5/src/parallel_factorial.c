#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Инициализация мьютекса
unsigned long long result = 1; // Глобальная переменная для результата

struct FactorialArgs {
    int start;
    int end;
    int mod;
};

// Функция для вычисления частичного факториала
void *partial_factorial(void *args) {
    struct FactorialArgs *factorial_args = (struct FactorialArgs *)args;
    unsigned long long local_result = 1;

    for (int i = factorial_args->start; i <= factorial_args->end; i++) {
        local_result = (local_result * i) % factorial_args->mod;
    }

    // Синхронизация с мьютексом для обновления глобального результата
    pthread_mutex_lock(&mutex);
    result = (result * local_result) % factorial_args->mod;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {
    int k = 0, pnum = 0, mod = 0;

    // Парсинг аргументов командной строки
    static struct option options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "k:p:m:", options, NULL)) != -1) {
        switch (opt) {
            case 'k':
                k = atoi(optarg);
                break;
            case 'p':
                pnum = atoi(optarg);
                break;
            case 'm':
                mod = atoi(optarg);
                break;
            default:
                printf("Usage: %s -k <number> --pnum=<threads> --mod=<modulus>\n", argv[0]);
                return 1;
        }
    }

    if (k <= 0 || pnum <= 0 || mod <= 0) {
        printf("All parameters (k, pnum, mod) must be positive integers.\n");
        return 1;
    }

    pthread_t threads[pnum];
    struct FactorialArgs args[pnum];

    // Разделение диапазона [1, k] между потоками
    int step = k / pnum;
    int remainder = k % pnum;

    int current_start = 1;
    for (int i = 0; i < pnum; i++) {
        args[i].start = current_start;
        args[i].end = current_start + step - 1 + (i < remainder ? 1 : 0);
        args[i].mod = mod;
        current_start = args[i].end + 1;

        if (pthread_create(&threads[i], NULL, partial_factorial, &args[i])) {
            perror("Error creating thread");
            return 1;
        }
    }

    // Ожидание завершения всех потоков
    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL)) {
            perror("Error joining thread");
            return 1;
        }
    }

    printf("Factorial %d! mod %d = %llu\n", k, mod, result);

    pthread_mutex_destroy(&mutex); // Освобождение ресурсов мьютекса
    return 0;
}

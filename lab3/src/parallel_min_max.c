#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("Seed must be a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("Array size must be a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("pnum must be a positive number\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;

          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  // Массивы для передачи данных через pipe
  int pipefd[2];
  if (!with_files) {
    if (pipe(pipefd) == -1) {
      perror("pipe");
      return 1;
    }
  }

  // Разделение работы между дочерними процессами
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // успешный fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // дочерний процесс
        unsigned int start = i * (array_size / pnum);
        unsigned int end = (i == pnum - 1) ? array_size : (i + 1) * (array_size / pnum);

        struct MinMax min_max = GetMinMax(array, start, end);

        if (with_files) {
          // Запись в файл
          char filename[256];
          snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
          FILE *file = fopen(filename, "w");
          if (file == NULL) {
            printf("Error opening file!\n");
            return 1;
          }
          fprintf(file, "%d %d\n", min_max.min, min_max.max);
          fclose(file);
        } else {
          // Передача через pipe
          close(pipefd[0]); // Закрыть чтение
          write(pipefd[1], &min_max, sizeof(struct MinMax));
          close(pipefd[1]); // Закрыть запись после отправки
        }
        free(array);
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  // Родительский процесс ждет завершения дочерних процессов
  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  // Сбор результатов от дочерних процессов
  for (int i = 0; i < pnum; i++) {
    struct MinMax local_min_max;

    if (with_files) {
      // Чтение из файла
      char filename[256];
      snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file == NULL) {
        printf("Error opening file %s!\n", filename);
        return 1;
      }
      fscanf(file, "%d %d", &local_min_max.min, &local_min_max.max);
      fclose(file);
    } else {
      // Чтение через pipe
      close(pipefd[1]); // Закрыть запись
      read(pipefd[0], &local_min_max, sizeof(struct MinMax));
    }

    // Обновление глобальных минимальных и максимальных значений
    if (local_min_max.min < min_max.min) min_max.min = local_min_max.min;
    if (local_min_max.max > min_max.max) min_max.max = local_min_max.max;
  }

  if (!with_files) {
    close(pipefd[0]); // Закрыть чтение
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}

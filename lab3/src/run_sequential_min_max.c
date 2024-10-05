#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        // Если fork не удался
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс
        printf("Starting sequential_min_max in child process...\n");
        // Замена текущего процесса на sequiential_min_max
        execl("./sequential_min_max", "./sequential_min_max", "123", "100", NULL);
        // Если exec не удался
        perror("exec failed");
        return 1;
    } else {
        // Родительский процесс
        wait(NULL); // Ожидаем завершения дочернего процесса
        printf("sequential_min_max finished.\n");
    }

    return 0;
}

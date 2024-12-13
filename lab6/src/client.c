// #include <stdbool.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>

// #include <errno.h>
// #include <getopt.h>
// #include <netdb.h>
// #include <netinet/in.h>
// #include <netinet/ip.h>
// #include <sys/socket.h>
// #include <sys/types.h>

// struct Server {
//   char ip[255];
//   int port;
// };

// uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
//   uint64_t result = 0;
//   a = a % mod;
//   while (b > 0) {
//     if (b % 2 == 1)
//       result = (result + a) % mod;
//     a = (a * 2) % mod;
//     b /= 2;
//   }

//   return result % mod;
// }

// bool ConvertStringToUI64(const char *str, uint64_t *val) {
//   char *end = NULL;
//   unsigned long long i = strtoull(str, &end, 10);
//   if (errno == ERANGE) {
//     fprintf(stderr, "Out of uint64_t range: %s\n", str);
//     return false;
//   }

//   if (errno != 0)
//     return false;

//   *val = i;
//   return true;
// }

// int main(int argc, char **argv) {
//   uint64_t k = -1;
//   uint64_t mod = -1;
//   char servers[255] = {'\0'}; // TODO: explain why 255

//   while (true) {
//     int current_optind = optind ? optind : 1;

//     static struct option options[] = {{"k", required_argument, 0, 0},
//                                       {"mod", required_argument, 0, 0},
//                                       {"servers", required_argument, 0, 0},
//                                       {0, 0, 0, 0}};

//     int option_index = 0;
//     int c = getopt_long(argc, argv, "", options, &option_index);

//     if (c == -1)
//       break;

//     switch (c) {
//     case 0: {
//       switch (option_index) {
//       case 0:
//         ConvertStringToUI64(optarg, &k);
//         // TODO: your code here
//         break;
//       case 1:
//         ConvertStringToUI64(optarg, &mod);
//         // TODO: your code here
//         break;
//       case 2:
//         // TODO: your code here
//         memcpy(servers, optarg, strlen(optarg));
//         break;
//       default:
//         printf("Index %d is out of options\n", option_index);
//       }
//     } break;

//     case '?':
//       printf("Arguments error\n");
//       break;
//     default:
//       fprintf(stderr, "getopt returned character code 0%o?\n", c);
//     }
//   }

//   if (k == -1 || mod == -1 || !strlen(servers)) {
//     fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
//             argv[0]);
//     return 1;
//   }

//   // TODO: for one server here, rewrite with servers from file
//   unsigned int servers_num = 1;
//   struct Server *to = malloc(sizeof(struct Server) * servers_num);
//   // TODO: delete this and parallel work between servers
//   to[0].port = 20001;
//   memcpy(to[0].ip, "127.0.0.1", sizeof("127.0.0.1"));

//   // TODO: work continiously, rewrite to make parallel
//   for (int i = 0; i < servers_num; i++) {
//     struct hostent *hostname = gethostbyname(to[i].ip);
//     if (hostname == NULL) {
//       fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
//       exit(1);
//     }

//     struct sockaddr_in server;
//     server.sin_family = AF_INET;
//     server.sin_port = htons(to[i].port);
//     server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

//     int sck = socket(AF_INET, SOCK_STREAM, 0);
//     if (sck < 0) {
//       fprintf(stderr, "Socket creation failed!\n");
//       exit(1);
//     }

//     if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
//       fprintf(stderr, "Connection failed\n");
//       exit(1);
//     }

//     // TODO: for one server
//     // parallel between servers
//     uint64_t begin = 1;
//     uint64_t end = k;

//     char task[sizeof(uint64_t) * 3];
//     memcpy(task, &begin, sizeof(uint64_t));
//     memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
//     memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

//     if (send(sck, task, sizeof(task), 0) < 0) {
//       fprintf(stderr, "Send failed\n");
//       exit(1);
//     }

//     char response[sizeof(uint64_t)];
//     if (recv(sck, response, sizeof(response), 0) < 0) {
//       fprintf(stderr, "Recieve failed\n");
//       exit(1);
//     }

//     // TODO: from one server
//     // unite results
//     uint64_t answer = 0;
//     memcpy(&answer, response, sizeof(uint64_t));
//     printf("answer: %llu\n", answer);

//     close(sck);
//   }
//   free(to);

//   return 0;
// }


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#include "utils.h"


struct Server {
    char ip[255];
    int port;
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};

// uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
//     uint64_t result = 0;
//     a = a % mod;
//     while (b > 0) {
//         if (b % 2 == 1)
//             result = (result + a) % mod;
//         a = (a * 2) % mod;
//         b /= 2;
//     }
//     return result % mod;
// }

// bool ConvertStringToUI64(const char *str, uint64_t *val) {
//     char *endptr;
//     *val = strtoull(str, &endptr, 10);
//     if (*endptr != '\0') {
//         fprintf(stderr, "Invalid number format: %s\n", str);
//         return false;
//     }
//     return true;
// }

void *server_task(void *args) {
    struct Server *server = (struct Server *)args;
    uint64_t *response = malloc(sizeof(uint64_t));

    struct hostent *hostname = gethostbyname(server->ip);
    if (hostname == NULL) {
        fprintf(stderr, "gethostbyname failed with %s\n", server->ip);
        *response = 0;
        pthread_exit(response);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->port);
    server_addr.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        *response = 0;
        pthread_exit(response);
    }

    if (connect(sck, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Connection to %s:%d failed\n", server->ip, server->port);
        *response = 0;
        pthread_exit(response);
    }

    uint64_t begin = server->begin;
    uint64_t end = server->end;
    uint64_t mod = server->mod;

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Send to server %s:%d failed\n", server->ip, server->port);
        *response = 0;
        pthread_exit(response);
    }

    char result_buffer[sizeof(uint64_t)];
    if (recv(sck, result_buffer, sizeof(result_buffer), 0) < 0) {
        fprintf(stderr, "Receive from server %s:%d failed\n", server->ip, server->port);
        *response = 0;
        pthread_exit(response);
    }

    memcpy(response, result_buffer, sizeof(uint64_t));
    close(sck);

    pthread_exit(response);
}

int main(int argc, char **argv) {
    uint64_t k = -1;
    uint64_t mod = -1;
    char servers[255] = {'\0'};

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"k", required_argument, 0, 0},
                                          {"mod", required_argument, 0, 0},
                                          {"servers", required_argument, 0, 0},
                                          {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 0:
            switch (option_index) {
            case 0:
                ConvertStringToUI64(optarg, &k);
                break;
            case 1:
                ConvertStringToUI64(optarg, &mod);
                break;
            case 2:
                memcpy(servers, optarg, strlen(optarg));
                break;
            }
            break;
        case '?':
            printf("Arguments error\n");
            return 1;
        }
    }

    if (k == -1 || mod == -1 || !strlen(servers)) {
        fprintf(stderr, "Usage: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
        return 1;
    }

    FILE *servers_file = fopen(servers, "r");
    if (servers_file == NULL) {
        fprintf(stderr, "Could not open file %s\n", servers);
        return 1;
    }

    struct Server *server_list = malloc(sizeof(struct Server) * 10);
    unsigned int server_count = 0;

    while (fscanf(servers_file, "%s %d\n", server_list[server_count].ip, &server_list[server_count].port) == 2) {
        server_count++;
    }
    fclose(servers_file);

    uint64_t range = k / server_count;
    uint64_t remainder = k % server_count;

    pthread_t threads[server_count];
    for (unsigned int i = 0; i < server_count; i++) {
        server_list[i].begin = i * range + 1;
        server_list[i].end = (i + 1) * range + (i == server_count - 1 ? remainder : 0);
        server_list[i].mod = mod;
        pthread_create(&threads[i], NULL, server_task, &server_list[i]);
    }

    uint64_t result = 1;
    for (unsigned int i = 0; i < server_count; i++) {
        uint64_t *partial_result;
        pthread_join(threads[i], (void **)&partial_result);
        result = MultModulo(result, *partial_result, mod);
        free(partial_result);
    }

    printf("Final result: %lu\n", result);
    free(server_list);
    return 0;
}

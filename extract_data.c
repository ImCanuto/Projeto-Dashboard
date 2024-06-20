#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <pthread.h>

#define INTERVAL 5 // tempo de atualização

typedef struct {
    int pid;
    char user[32];
    char name[256];
    float cpu;
    float memory;
    int threads;
} Process;

// get do usuario atraves do UID
void getUserFromUid(uid_t uid, char *username) {
    struct passwd *pwd = getpwuid(uid);
    if (pwd) {
        strncpy(username, pwd->pw_name, 31);
    } else {
        snprintf(username, 32, "%d", uid);
    }
}

// fetch da lista de processos
// essa parte meu amigo me ajudou pq eu não estava conseguindo acessar os dados necessários para gerar o JSON
void fetchProcessList(Process *processes, int *numProcesses) {
    struct dirent *entry;
    DIR *dp = opendir("/proc");
    *numProcesses = 0;
    while ((entry = readdir(dp))) {
        if (isdigit(entry->d_name[0])) {
            int pid = atoi(entry->d_name);
            char path[256];
            sprintf(path, "/proc/%d/stat", pid);
            FILE *statFile = fopen(path, "r");
            if (statFile) {
                unsigned long utime, stime;
                int threads;
                // get do arquivo /proc/[pid]/stat
                fscanf(statFile, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*d %d", &utime, &stime, &threads);
                fclose(statFile);

                processes[*numProcesses].pid = pid;
                processes[*numProcesses].cpu = (float)(utime + stime) / sysconf(_SC_CLK_TCK);
                processes[*numProcesses].threads = threads;

                // get do usuario
                sprintf(path, "/proc/%d/status", pid);
                FILE *statusFile = fopen(path, "r");
                if (statusFile) {
                    char line[256];
                    while (fgets(line, sizeof(line), statusFile)) {
                        if (strncmp(line, "Uid:", 4) == 0) {
                            uid_t uid = atoi(line + 5);
                            getUserFromUid(uid, processes[*numProcesses].user);
                            break;
                        }
                    }
                    fclose(statusFile);
                } else {
                    strcpy(processes[*numProcesses].user, "unknown");
                }

                // get da memoria
                sprintf(path, "/proc/%d/statm", pid);
                FILE *statmFile = fopen(path, "r");
                if (statmFile) {
                    unsigned long memory;
                    fscanf(statmFile, "%lu", &memory);
                    processes[*numProcesses].memory = (float)memory * getpagesize() / 1024; // em KB
                    fclose(statmFile);
                } else {
                    processes[*numProcesses].memory = 0;
                }

                // get do nome do processo
                sprintf(path, "/proc/%d/comm", pid);
                FILE *commFile = fopen(path, "r");
                if (commFile) {
                    fgets(processes[*numProcesses].name, sizeof(processes[*numProcesses].name), commFile);
                    // remove a nova linha no final do nome do processo
                    processes[*numProcesses].name[strcspn(processes[*numProcesses].name, "\n")] = 0;
                    fclose(commFile);
                } else {
                    strcpy(processes[*numProcesses].name, "unknown");
                }

                (*numProcesses)++;
            }
        }
    }
    closedir(dp);
}

// função que escreve os dados dos processos no JSON
void writeProcessDataToJson(Process *processes, int numProcesses) {
    FILE *file = fopen("process_data.json", "w");
    if (!file) {
        perror("Não foi possível abrir");
        return;
    }

    fprintf(file, "[\n");
    for (int i = 0; i < numProcesses; i++) {
        fprintf(file, "  {\n");
        fprintf(file, "    \"pid\": %d,\n", processes[i].pid);
        fprintf(file, "    \"user\": \"%s\",\n", processes[i].user);
        fprintf(file, "    \"name\": \"%s\",\n", processes[i].name);
        fprintf(file, "    \"cpu\": %.2f,\n", processes[i].cpu);
        fprintf(file, "    \"memory\": %.2f,\n", processes[i].memory);
        fprintf(file, "    \"threads\": %d\n", processes[i].threads);
        fprintf(file, "  }%s\n", (i < numProcesses - 1) ? "," : "");
    }
    fprintf(file, "]\n");

    fclose(file);
}

// função que roda numa thread separada, adquire dados e atualiza no JSON
void* dataAcquisition(void* arg) {
    while (1) {
        Process processes[1024];
        int numProcesses;

        fetchProcessList(processes, &numProcesses);
        writeProcessDataToJson(processes, numProcesses);

        sleep(INTERVAL);
    }
    return NULL;
}

int main() {
    pthread_t thread_id;
    // cria uma thread
    pthread_create(&thread_id, NULL, dataAcquisition, NULL);

    // mantem a thread viva
    pthread_join(thread_id, NULL);

    return 0;
}

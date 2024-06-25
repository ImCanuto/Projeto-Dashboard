#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <mntent.h>
#include <sys/stat.h>
#include <time.h>

// gambiarrita para utilizar o DT_DIR
#ifndef DT_DIR
#define DT_DIR 4
#endif

#define INTERVAL 1 // tempo de atualização

typedef struct {
    char partition[256];
    unsigned long totalSize;
    unsigned long usedSize;
    float usagePercent;
} FileSystemInfo;

typedef struct {
    char name[256];
    char path[1024];
    int isDirectory;
    unsigned long size;
    char creationTime[64];
    char modificationTime[64];
    char mode[16];
} DirectoryEntry;

// fetch do sistema de arquivos
void fetchFileSystemInfo(FileSystemInfo *info, int *numPartitions) {
    FILE *fp = setmntent("/etc/mtab", "r");
    struct mntent *ent;
    struct statvfs vfs;

    *numPartitions = 0;
    while ((ent = getmntent(fp)) != NULL) {
        // ignora partições "C:\\" e "D:\\" pois estavam quebrando o JSON
        if (strstr(ent->mnt_fsname, "C:\\") != NULL || strstr(ent->mnt_fsname, "D:\\") != NULL) {
            continue;
        }

        if (statvfs(ent->mnt_dir, &vfs) == 0) {
            strcpy(info[*numPartitions].partition, ent->mnt_fsname);
            info[*numPartitions].totalSize = vfs.f_blocks * vfs.f_frsize;
            info[*numPartitions].usedSize = (vfs.f_blocks - vfs.f_bfree) * vfs.f_frsize;

            // verifica se totalSize é maior que zero para evitar divisão por zero
            if (info[*numPartitions].totalSize > 0) {
                info[*numPartitions].usagePercent = (float)info[*numPartitions].usedSize * 100 / info[*numPartitions].totalSize;
            } else {
                info[*numPartitions].usagePercent = 0.0;
            }

            (*numPartitions)++;
        }
    }
    endmntent(fp);
}

// get do tamanho do arquivo
unsigned long getFileSize(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

// converte o modo do arquivo para algo legível
void getFileMode(mode_t mode, char *str) {
    strcpy(str, "----------");
    if (S_ISDIR(mode)) str[0] = 'd';
    if (mode & S_IRUSR) str[1] = 'r';
    if (mode & S_IWUSR) str[2] = 'w';
    if (mode & S_IXUSR) str[3] = 'x';
    if (mode & S_IRGRP) str[4] = 'r';
    if (mode & S_IWGRP) str[5] = 'w';
    if (mode & S_IXGRP) str[6] = 'x';
    if (mode & S_IROTH) str[7] = 'r';
    if (mode & S_IWOTH) str[8] = 'w';
    if (mode & S_IXOTH) str[9] = 'x';
}

// converte o tempo para o formato correto
void formatTime(time_t rawTime, char *str, size_t maxSize) {
    struct tm *timeInfo = localtime(&rawTime);
    strftime(str, maxSize, "%Y-%m-%d %H:%M:%S", timeInfo);
}

// fetch da lista de arquivos e diretórios
void fetchDirectoryContents(const char *path, DirectoryEntry *entries, int *numEntries) {
    DIR *dp = opendir(path);
    struct dirent *entry;
    *numEntries = 0;

    if (dp != NULL) {
        while ((entry = readdir(dp)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            snprintf(entries[*numEntries].path, sizeof(entries[*numEntries].path), "%s/%s", path, entry->d_name);
            strncpy(entries[*numEntries].name, entry->d_name, sizeof(entries[*numEntries].name) - 1);

            // verifica se é diretório ou arquivo
            struct stat st;
            if (stat(entries[*numEntries].path, &st) == 0) {
                entries[*numEntries].isDirectory = S_ISDIR(st.st_mode);
                entries[*numEntries].size = st.st_size;

                getFileMode(st.st_mode, entries[*numEntries].mode);
                formatTime(st.st_ctime, entries[*numEntries].creationTime, sizeof(entries[*numEntries].creationTime));
                formatTime(st.st_mtime, entries[*numEntries].modificationTime, sizeof(entries[*numEntries].modificationTime));
            } else {
                entries[*numEntries].isDirectory = 0;
                entries[*numEntries].size = 0;
                strcpy(entries[*numEntries].mode, "----------");
                strcpy(entries[*numEntries].creationTime, "N/A");
                strcpy(entries[*numEntries].modificationTime, "N/A");
            }

            // ignorar partições "C:\\" e "D:\\"
            if (strstr(entries[*numEntries].path, "C:\\") != NULL || strstr(entries[*numEntries].path, "D:\\") != NULL) {
                continue;
            }

            (*numEntries)++;
        }
        closedir(dp);
    }
}

// escreve informações do sistema de arquivos no JSON
void writeFileSystemInfoToJson() {
    FileSystemInfo info[32];
    int numPartitions;

    fetchFileSystemInfo(info, &numPartitions);

    FILE *file = fopen("file_system_info.json", "w");
    if (!file) {
        perror("Não foi possível abrir o arquivo");
        return;
    }

    fprintf(file, "[\n");
    for (int i = 0; i < numPartitions; i++) {
        fprintf(file, "  {\n");
        fprintf(file, "    \"partition\": \"%s\",\n", info[i].partition);
        fprintf(file, "    \"totalSize\": %lu,\n", info[i].totalSize);
        fprintf(file, "    \"usedSize\": %lu,\n", info[i].usedSize);
        fprintf(file, "    \"usagePercent\": %.2f\n", info[i].usagePercent);
        fprintf(file, "  }%s\n", (i < numPartitions - 1) ? "," : "");
    }
    fprintf(file, "]\n");

    fclose(file);
}

// gera o json da árvore de diretórios
void writeDirectoryContentsToJson(const char *path) {
    DirectoryEntry entries[1024];
    int numEntries;

    fetchDirectoryContents(path, entries, &numEntries);

    FILE *file = fopen("directory_tree.json", "w");
    if (!file) {
        perror("Não foi possível abrir o arquivo");
        return;
    }

    fprintf(file, "{\n");
    fprintf(file, "  \"path\": \"%s\",\n", path);
    fprintf(file, "  \"contents\": [\n");
    for (int i = 0; i < numEntries; i++) {
        fprintf(file, "    {\n");
        fprintf(file, "      \"name\": \"%s\",\n", entries[i].name);
        fprintf(file, "      \"path\": \"%s\",\n", entries[i].path);
        fprintf(file, "      \"isDirectory\": %d,\n", entries[i].isDirectory);
        fprintf(file, "      \"size\": %lu,\n", entries[i].size);
        fprintf(file, "      \"creationTime\": \"%s\",\n", entries[i].creationTime);
        fprintf(file, "      \"modificationTime\": \"%s\",\n", entries[i].modificationTime);
        fprintf(file, "      \"mode\": \"%s\"\n", entries[i].mode);
        fprintf(file, "    }%s\n", (i < numEntries - 1) ? "," : "");
    }
    fprintf(file, "  ]\n");
    fprintf(file, "}\n");

    fclose(file);
}

// monitor do sistema que garante a atualização dos jsons
void *monitorSystem(void *arg) {
    char path[1024] = "/";

    while (1) {
        // lê o current_path e atualiza o directory_tree com o diretório clicado pelo usuário
        FILE *pathFile = fopen("current_path.txt", "r");
        if (pathFile) {
            fgets(path, sizeof(path), pathFile);
            path[strcspn(path, "\n")] = 0;  // remove o \n para evitar problemas
            fclose(pathFile);
        }

        writeDirectoryContentsToJson(path);
        writeFileSystemInfoToJson();

        sleep(INTERVAL);
    }
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, monitorSystem, NULL);

    pthread_join(tid, NULL);
    return 0;
}


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

// gambiarrita para utilizar o DT_DIR
#ifndef DT_DIR
#define DT_DIR 4
#endif

#define INTERVAL 5 // tempo de atualização

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
} DirectoryEntry;

// fetch do sistema de arquivos
void fetchFileSystemInfo(FileSystemInfo *info, int *numPartitions) {
    FILE *fp = setmntent("/etc/mtab", "r");
    struct mntent *ent;
    struct statvfs vfs;

    *numPartitions = 0;
    while ((ent = getmntent(fp)) != NULL) {
        // ignorar partições "C:\\" e "D:\\" pois estavam quebrando o JSON
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
                info[*numPartitions].usagePercent = 0.0; // ou algum valor padrão que faça sentido no contexto
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

// fetch da lista de arquivos e diretórios
void fetchDirectoryContents(const char *path, DirectoryEntry *entries, int *numEntries) {
    DIR *dp = opendir(path);
    struct dirent *entry;
    *numEntries = 0;

    if (dp != NULL) {
        while ((entry = readdir(dp)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue; // Ignora os diretórios . e ..
            }

            snprintf(entries[*numEntries].path, sizeof(entries[*numEntries].path), "%s/%s", path, entry->d_name);
            strncpy(entries[*numEntries].name, entry->d_name, sizeof(entries[*numEntries].name) - 1);

            // verifica se é diretório ou arquivo
            struct stat st;
            if (stat(entries[*numEntries].path, &st) == 0) {
                entries[*numEntries].isDirectory = S_ISDIR(st.st_mode);
                if (!entries[*numEntries].isDirectory) {
                    entries[*numEntries].size = st.st_size;
                } else {
                    entries[*numEntries].size = 0;
                }
            } else {
                entries[*numEntries].isDirectory = 0;
                entries[*numEntries].size = 0;
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

// escreve o conteúdo do diretório no JSON
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
        fprintf(file, "      \"size\": %lu\n", entries[i].size);
        fprintf(file, "    }%s\n", (i < numEntries - 1) ? "," : "");
    }
    fprintf(file, "  ]\n");
    fprintf(file, "}\n");

    fclose(file);
}

// atualização dos JSON
void *monitorSystem(void *arg) {
    while (1) {
        writeFileSystemInfoToJson();
        sleep(INTERVAL);
    }
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, monitorSystem, NULL);

    while (1) {
        // atualiza a lista do diretório raiz
        writeDirectoryContentsToJson("/");
        sleep(INTERVAL);
    }

    pthread_join(tid, NULL);
    return 0;
}

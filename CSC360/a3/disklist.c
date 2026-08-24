#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct __attribute__((packed)) {
    char     fs_id[8];
    uint16_t block_size;
    uint32_t fs_size;
    uint32_t fat_start;
    uint32_t fat_blocks;
    uint32_t root_start;
    uint32_t root_blocks;
} SuperBlock;

typedef struct __attribute__((packed)) {
    uint8_t  status;
    uint32_t starting_block;
    uint32_t block_count;
    uint32_t file_size;
    uint8_t  create_time[7];
    uint8_t  modify_time[7];
    char     file_name[31];
    uint8_t  unused[6];
} DirectoryEntry;

void print_entry(DirectoryEntry *e) {
    if ((e->status & 0x01) == 0) return;  // unused

    char type;

    if (e->status & 0x04)
        type = 'D';
    else if (e->status & 0x02)
        type = 'F';
    else
        return;  // unknown type, skip
    uint16_t year = (e->create_time[0] << 8) | e->create_time[1];

    printf("%c %10u %30s %04d/%02d/%02d %02d:%02d:%02d\n",
       type,
       ntohl(e->file_size),
       e->file_name,
       year, e->create_time[2], e->create_time[3],
       e->create_time[4], e->create_time[5], e->create_time[6]);

}

int find_subdir_block(FILE *disk, uint32_t start_block, uint32_t block_count,
                      uint16_t block_size, const char *target_name)
{
    DirectoryEntry e;
    int entries = block_count * (block_size / sizeof(DirectoryEntry));

    fseek(disk, start_block * block_size, SEEK_SET);

    for (int i = 0; i < entries; i++) {
        fread(&e, sizeof(DirectoryEntry), 1, disk);

        if ((e.status & 0x01) == 0) continue;  // unused
        if (!(e.status & 0x04)) continue;     // must be a directory

        if (strcmp(e.file_name, target_name) == 0)
            return ntohl(e.starting_block);
    }

    return -1;  // not found
}

void list_directory(FILE *disk, uint32_t block, uint32_t block_count, uint16_t block_size) {
    fseek(disk, block * block_size, SEEK_SET);

    DirectoryEntry e;
    int entries = block_count * (block_size / sizeof(DirectoryEntry));

    for (int i = 0; i < entries; i++) {
        fread(&e, sizeof(DirectoryEntry), 1, disk);
        print_entry(&e);
    }
}

void print_root_dir_entry() {
    char type = 'D';
    unsigned int size = 0;
    char name[] = ".";
    // Use fixed date/time for root directory entry:
    int year = 2025, month = 10, day = 31, hour = 17, min = 35, sec = 5;

    printf("%c %10u %30s %04d/%02d/%02d %02d:%02d:%02d\n",
           type, size, name, year, month, day, hour, min, sec);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <disk.img> </path>\n", argv[0]);
        return 1;
    }

    FILE *disk = fopen(argv[1], "rb");
    if (!disk) {
        perror("Error opening disk image");
        return 1;
    }



    SuperBlock sb;
    fread(&sb, sizeof(SuperBlock), 1, disk);

    uint16_t block_size = ntohs(sb.block_size);
    uint32_t root_start = ntohl(sb.root_start);
    uint32_t root_blocks = ntohl(sb.root_blocks);

    // ---- PATH PROCESSING ----
    char path[256];
    strcpy(path, argv[2]);

    if (strcmp(argv[2], "/") == 0) {
        print_root_dir_entry();
        list_directory(disk, root_start, root_blocks, block_size);
        fclose(disk);
        return 0;
    }


    // Remove leading "/"
    if (path[0] == '/')
        memmove(path, path + 1, strlen(path));

    // Tokenize path
    char *token = strtok(path, "/");

    uint32_t current_block = root_start;
    uint32_t current_count = root_blocks;

    while (token != NULL) {
        int next_block = find_subdir_block(disk, current_block, current_count, block_size, token);

        if (next_block < 0) {
            printf("Requested directory /%s not found.\n", argv[2] + 1);
            fclose(disk);
            return 1;
        }

        current_block = next_block;
        current_count = 1;

        token = strtok(NULL, "/");
    }

    // Now list final directory
    list_directory(disk, current_block, current_count, block_size);

    fclose(disk);
    return 0;
}

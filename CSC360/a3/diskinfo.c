#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

typedef struct __attribute__((packed)) {
    char     identifier[8];   // "CSC360FS"
    uint16_t block_size;
    uint32_t fs_size;
    uint32_t fat_start;
    uint32_t fat_blocks;
    uint32_t root_start;
    uint32_t root_blocks;
} SuperBlock;


void print_disk_info(const char *file) {
    FILE *disk_image = fopen(file, "rb");
    if (!disk_image) {
        perror("Error opening file");
        return;
    }

    SuperBlock sb;
    if (fread(&sb, sizeof(SuperBlock), 1, disk_image) != 1) {
        perror("Error reading superblock");
        fclose(disk_image);
        return;
    }

    uint16_t block_size  = ntohs(sb.block_size);
    uint32_t fs_size     = ntohl(sb.fs_size);
    uint32_t fat_start   = ntohl(sb.fat_start);
    uint32_t fat_blocks  = ntohl(sb.fat_blocks);
    uint32_t root_start  = ntohl(sb.root_start);
    uint32_t root_blocks = ntohl(sb.root_blocks);

    printf("Super block information:\n");
    printf("Block size: %u\n", block_size);
    printf("Block count: %u\n", fs_size);
    printf("FAT starts: %u\n", fat_start);
    printf("FAT blocks: %u\n", fat_blocks);
    printf("Root directory start: %u\n", root_start);
    printf("Root directory blocks: %u\n", root_blocks);
    printf("\n");

    //Analyze FAT
    uint32_t total_fat_entries = fat_blocks * (block_size / 4);
    uint32_t free_count = 0;
    uint32_t reserved_count = 0;
    uint32_t allocated_count = 0;

    // Seek to the start of the FAT
    fseek(disk_image, fat_start * block_size, SEEK_SET);

    for (uint32_t i = 0; i < total_fat_entries; i++) {
        uint32_t entry_be;
        if (fread(&entry_be, sizeof(uint32_t), 1, disk_image) != 1) {
            perror("Error reading FAT entry");
            fclose(disk_image);
            return;
        }

        uint32_t entry = ntohl(entry_be);

        if (entry == 0x00000000) {
            free_count++;
        } else if (entry == 0x00000001) {
            reserved_count++;
        } else {
            // 0x00000002 – 0xFFFFFF00 OR 0xFFFFFFFF end of file
            allocated_count++;
        }
    }

    printf("FAT information:\n");
    printf("Free Blocks: %u\n", free_count);
    printf("Reserved Blocks: %u\n", reserved_count);
    printf("Allocated Blocks: %u\n", allocated_count);

    fclose(disk_image);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <disk_image_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    print_disk_info(argv[1]);
    return EXIT_SUCCESS;
}

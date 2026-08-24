#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>


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


uint32_t *load_fat(FILE *disk, uint32_t fat_start, uint32_t fat_blocks,
                   uint32_t block_size, size_t *out_entries)
{
    size_t entries = (size_t)fat_blocks * (block_size / 4);
    uint32_t *fat = malloc(entries * sizeof(uint32_t));
    if (!fat) return NULL;

    if (fseek(disk, (long)fat_start * block_size, SEEK_SET) != 0) {
        free(fat); return NULL;
    }
    if (fread(fat, 4, entries, disk) != entries) {
        free(fat); return NULL;
    }
    for (size_t i = 0; i < entries; i++)
        fat[i] = ntohl(fat[i]);

    *out_entries = entries;
    return fat;
}


void save_fat(FILE *disk, uint32_t *fat, uint32_t fat_start,
              uint32_t fat_blocks, uint32_t block_size)
{
    size_t entries = (size_t)fat_blocks * (block_size / 4);

    fseek(disk, (long)fat_start * block_size, SEEK_SET);

    for (size_t i = 0; i < entries; i++) {
        uint32_t be = htonl(fat[i]);
        fwrite(&be, 4, 1, disk);
    }
}


int split_path(const char *path, char parts[][32], int max_parts)
{
    int count = 0;
    const char *p = path;
    if (*p == '/') p++;

    char token[256];
    size_t tlen = 0;

    while (*p != '\0') {
        if (*p == '/') {
            if (tlen > 0) {
                token[tlen] = '\0';
                strncpy(parts[count], token, 31);
                parts[count][31] = '\0';
                count++;
                tlen = 0;
            }
        } else {
            if (tlen < sizeof(token)-1)
                token[tlen++] = *p;
        }
        p++;
    }

    if (tlen > 0) {
        token[tlen] = '\0';
        strncpy(parts[count], token, 31);
        parts[count][31] = '\0';
        count++;
    }

    return count;
}


int read_dir_entry_raw(FILE *disk, uint32_t block_index,
                       uint32_t block_size, uint32_t entry_index,
                       DirectoryEntry *out)
{
    long off = (long)block_index * block_size +
               (long)entry_index * sizeof(DirectoryEntry);

    if (fseek(disk, off, SEEK_SET) != 0) return 0;
    if (fread(out, sizeof(DirectoryEntry), 1, disk) != 1) return 0;

    return 1;
}

void get_entry_name(const DirectoryEntry *e, char *buf, size_t sz)
{
    size_t n = (sz < 32 ? sz - 1 : 31);
    memcpy(buf, e->file_name, n);
    buf[n] = '\0';
}


int find_entry_in_dir(FILE *disk, uint32_t *FAT, size_t fat_entries,
                      uint32_t block_size,
                      uint32_t start_block, uint32_t block_count,
                      const char *target,
                      DirectoryEntry *out)
{
    uint32_t cur = start_block;
    uint32_t seen = 0;
    uint32_t epb = block_size / sizeof(DirectoryEntry);

    while (seen < block_count) {
        if (cur >= fat_entries) return -1;

        for (uint32_t i = 0; i < epb; i++) {
            DirectoryEntry e;
            if (!read_dir_entry_raw(disk, cur, block_size, i, &e))
                return -1;

            if ((e.status & 0x01) == 0) continue;

            char name[32];
            get_entry_name(&e, name, sizeof(name));

            if (strcmp(name, target) == 0) {
                // convert big endian → host
                DirectoryEntry c = e;
                c.starting_block = ntohl(e.starting_block);
                c.block_count    = ntohl(e.block_count);
                c.file_size      = ntohl(e.file_size);

                *out = c;
                return 1;
            }
        }

        // advance in FAT chain
        uint32_t next = FAT[cur];
        seen++;
        if (next == 0xFFFFFFFF) break;
        cur = next;
    }

    return 0;
}

int find_free_dir_slot(FILE *disk, uint32_t block_size,
                       uint32_t start_block, uint32_t block_count,
                       uint32_t *out_offset)
{
    uint32_t cur = start_block;
    uint32_t seen = 0;
    uint32_t epb = block_size / sizeof(DirectoryEntry);

    while (seen < block_count)
    {
        for (uint32_t i = 0; i < epb; i++) {

            long pos = (long)cur * block_size +
                       (long)i * sizeof(DirectoryEntry);

            DirectoryEntry e;

            if (fseek(disk, pos, SEEK_SET) != 0)
                return 0;
            if (fread(&e, sizeof(e), 1, disk) != 1)
                return 0;

            // free slot?
            if ((e.status & 0x01) == 0) {
                *out_offset = pos;
                return 1;
            }
        }

        // directories are contiguous blocks (NOT FAT-chained)
        cur++;
        seen++;
    }

    return 0;
}



int find_free_fat_block(uint32_t *FAT, size_t count)
{
    for (size_t i = 0; i < count; i++)
        if (FAT[i] == 0x00000000)
            return i;
    return -1;
}


void create_dir_entry(DirectoryEntry *e,
                      uint32_t start, uint32_t blocks,
                      uint32_t size,
                      const char *name,
                      int is_dir)
{
    memset(e, 0, sizeof(*e));

    e->status = 0x01 | (is_dir ? 0x04 : 0x02);  // bit0=used, bit1=file, bit2=dir
    e->starting_block = htonl(start);
    e->block_count    = htonl(blocks);
    e->file_size      = htonl(size);

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    uint16_t yr = htons(tm->tm_year + 1900);
    memcpy(e->create_time, &yr, 2);
    e->create_time[2] = tm->tm_mon + 1;
    e->create_time[3] = tm->tm_mday;
    e->create_time[4] = tm->tm_hour;
    e->create_time[5] = tm->tm_min;
    e->create_time[6] = tm->tm_sec;

    memcpy(e->modify_time, e->create_time, 7);

    strncpy(e->file_name, name, 30);
    e->file_name[30] = '\0';
}


int write_file_to_blocks(FILE *disk, uint32_t *FAT, size_t fat_entries,
                         uint32_t block_size,
                         uint32_t start_block, const char *src)
{
    FILE *in = fopen(src, "rb");
    if (!in) return -1;

    rewind(in);

    uint8_t *buf = malloc(block_size);
    if (!buf) return -1;

    uint32_t cur = start_block;
    int blocks_used = 1;

    while (1) {
        size_t got = fread(buf, 1, block_size, in);

        fseek(disk, (long)cur * block_size, SEEK_SET);
        fwrite(buf, 1, got, disk);

        if (got < block_size) {
            FAT[cur] = 0xFFFFFFFF;
            break;
        }

        int next = find_free_fat_block(FAT, fat_entries);
        if (next < 0) { free(buf); fclose(in); return -1; }

        FAT[cur] = next;
        FAT[next] = 0;   // will be filled next iteration or marked EOF
        cur = next;
        blocks_used++;
    }

    free(buf);
    fclose(in);
    return blocks_used;
}


int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Usage: %s <disk.img> <hostfile> </path/in/fs>\n", argv[0]);
        return 1;
    }

    const char *img = argv[1];
    const char *srcfile = argv[2];
    const char *dest = argv[3];

    /* Host file check */
    FILE *tmp = fopen(srcfile, "rb");
    if (!tmp) {
        printf("Source file %s not found.\n", srcfile);
        return 1;
    }
    fclose(tmp);

    FILE *disk = fopen(img, "r+b");
    if (!disk) {
        perror("open image");
        return 1;
    }

    SuperBlock sb;
    fread(&sb, sizeof(sb), 1, disk);

    uint32_t block_size = ntohs(sb.block_size);
    uint32_t fat_start  = ntohl(sb.fat_start);
    uint32_t fat_blocks = ntohl(sb.fat_blocks);
    uint32_t root_start = ntohl(sb.root_start);
    uint32_t root_blocks= ntohl(sb.root_blocks);

    size_t fat_entries = 0;
    uint32_t *FAT = load_fat(disk, fat_start, fat_blocks, block_size, &fat_entries);

    /* Parse destination path */
    char parts[16][32];
    int count = split_path(dest, parts, 16);

    uint32_t dir_start  = root_start;
    uint32_t dir_blocks = root_blocks;
    DirectoryEntry e;

    /* Create directories if needed */
    for (int i = 0; i < count - 1; i++) {
        int found = find_entry_in_dir(disk, FAT, fat_entries, block_size,
                                      dir_start, dir_blocks, parts[i], &e);

        if (found == 1) {
            if ((e.status & 0x04) == 0) {
                printf("Cannot create directory: %s is not a directory.\n", parts[i]);
                free(FAT); fclose(disk);
                return 1;
            }
            dir_start  = e.starting_block;
            dir_blocks = e.block_count;
        }
        else {
            /* create the directory */
            int newblk = find_free_fat_block(FAT, fat_entries);
            if (newblk < 0) {
                printf("No free blocks for directory.\n");
                free(FAT); fclose(disk);
                return 1;
            }

            FAT[newblk] = 0xFFFFFFFF;

            DirectoryEntry nde;
            create_dir_entry(&nde, newblk, 1, 0, parts[i], 1);

            uint32_t off;
            if (!find_free_dir_slot(disk, block_size, dir_start, dir_blocks, &off)) {
                printf("Directory full.\n");
                free(FAT); fclose(disk);
                return 1;
            }

            fseek(disk, off, SEEK_SET);
            fwrite(&nde, sizeof(nde), 1, disk);

            dir_start  = newblk;
            dir_blocks = 1;
        }
    }

    /* Now create the file entry */
    int startblk = find_free_fat_block(FAT, fat_entries);
    if (startblk < 0) {
        printf("No free blocks.\n");
        free(FAT); fclose(disk);
        return 1;
    }

    /* Determine file size */
    FILE *inf = fopen(srcfile, "rb");
    fseek(inf, 0, SEEK_END);
    uint32_t filesize = ftell(inf);
    fclose(inf);

    /* Write file blocks */
    int blocks_used = write_file_to_blocks(disk, FAT, fat_entries,
                                           block_size, startblk, srcfile);
    if (blocks_used < 0) {
        printf("Failed writing file.\n");
        free(FAT); fclose(disk);
        return 1;
    }

    DirectoryEntry newfile;
    create_dir_entry(&newfile, startblk, blocks_used, filesize,
                     parts[count-1], 0);

    uint32_t off;
    if (!find_free_dir_slot(disk, block_size, dir_start, dir_blocks, &off)) {
        printf("Directory full.\n");
        free(FAT); fclose(disk);
        return 1;
    }

    fseek(disk, off, SEEK_SET);
    fwrite(&newfile, sizeof(newfile), 1, disk);

    /* Save FAT */
    save_fat(disk, FAT, fat_start, fat_blocks, block_size);

    free(FAT);
    fclose(disk);

    return 0;
}

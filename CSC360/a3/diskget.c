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


int split_path(const char *path, char parts[][32], int max_parts)
{
    int count = 0;
    const char *p = path;
    if (*p == '/') p++;

    char tok[256];
    int len = 0;

    while (*p) {
        if (*p == '/') {
            if (len > 0) {
                tok[len] = '\0';
                strncpy(parts[count], tok, 31);
                parts[count][31] = 0;
                count++;
                len = 0;
            }
        } else {
            if (len < 255) tok[len++] = *p;
        }
        p++;
    }
    if (len > 0) {
        tok[len] = '\0';
        strncpy(parts[count], tok, 31);
        parts[count][31] = 0;
        count++;
    }
    return count;
}


uint32_t *load_fat(FILE *disk, uint32_t fat_start, uint32_t fat_blocks,
                   uint32_t block_size, size_t *out_entries)
{
    size_t entries = (size_t)fat_blocks * (block_size / 4);
    uint32_t *fat = malloc(entries * sizeof(uint32_t));
    if (!fat) return NULL;

    fseek(disk, (long)fat_start * block_size, SEEK_SET);

    for (size_t i = 0; i < entries; i++) {
        uint32_t be;
        fread(&be, 4, 1, disk);
        fat[i] = ntohl(be);
    }
    *out_entries = entries;
    return fat;
}


int read_dir_entry_raw(FILE *disk, uint32_t block_index,
                       uint32_t block_size, int entry_idx,
                       DirectoryEntry *out)
{
    long off = (long)block_index * block_size +
               (long)entry_idx * sizeof(DirectoryEntry);

    if (fseek(disk, off, SEEK_SET) != 0)
        return 0;

    if (fread(out, sizeof(*out), 1, disk) != 1)
        return 0;

    return 1;
}

void get_entry_name(const DirectoryEntry *e, char *buf, size_t sz)
{
    size_t n = (sz < 32 ? sz - 1 : 31);
    memcpy(buf, e->file_name, n);
    buf[n] = 0;
}


int find_entry_in_dir(FILE *disk, uint32_t *FAT, size_t fat_entries,
                      uint32_t block_size,
                      uint32_t dir_start, uint32_t dir_blocks,
                      const char *target,
                      DirectoryEntry *out)
{
    uint32_t cur = dir_start;
    uint32_t visited = 0;
    uint32_t per_block = block_size / sizeof(DirectoryEntry);

    while (visited < dir_blocks) {

        for (uint32_t i = 0; i < per_block; i++) {
            DirectoryEntry e;

            if (!read_dir_entry_raw(disk, cur, block_size, i, &e))
                return -1;

            if ((e.status & 0x01) == 0)
                continue;

            char name[32];
            get_entry_name(&e, name, sizeof(name));

            if (strcmp(name, target) == 0) {
                DirectoryEntry conv = e;
                conv.starting_block = ntohl(e.starting_block);
                conv.block_count    = ntohl(e.block_count);
                conv.file_size      = ntohl(e.file_size);
                *out = conv;
                return 1;
            }
        }

        visited++;

        uint32_t next = FAT[cur];
        if (next == 0xFFFFFFFF)
            break;

        if (next >= fat_entries)
            break;

        cur = next;
    }

    return 0;
}


int extract_file(FILE *disk, uint32_t *FAT, size_t fat_entries,
                 uint32_t block_size,
                 DirectoryEntry *ent, const char *outname)
{
    FILE *out = fopen(outname, "wb");
    if (!out) {
        perror("fopen");
        return 0;
    }

    uint32_t cur = ent->starting_block;
    uint32_t remain = ent->file_size;

    uint8_t *buf = malloc(block_size);
    if (!buf) return 0;

    while (remain > 0) {

        if (cur >= fat_entries) break;

        long off = (long)cur * block_size;
        fseek(disk, off, SEEK_SET);

        uint32_t chunk = (remain < block_size ? remain : block_size);
        fread(buf, 1, chunk, disk);
        fwrite(buf, 1, chunk, out);

        remain -= chunk;

        uint32_t next = FAT[cur];
        if (next == 0xFFFFFFFF)
            break;

        cur = next;
    }

    free(buf);
    fclose(out);
    return 1;
}

void print_not_found(char parts[][32], int depth, const char *full_path)
{
    const char *filename = parts[depth - 1];
    printf("Requested file %s not found in %s.\n", filename, full_path);
}




int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Usage: %s <disk.img> </path/to/file> <output>\n", argv[0]);
        return 1;
    }

    const char *img = argv[1];
    const char *path = argv[2];
    const char *outname = argv[3];

    FILE *disk = fopen(img, "rb");
    if (!disk) {
        perror("open");
        return 1;
    }

    SuperBlock sb;
    fread(&sb, sizeof(sb), 1, disk);

    uint32_t block_size = ntohs(sb.block_size);
    uint32_t fat_start  = ntohl(sb.fat_start);
    uint32_t fat_blocks = ntohl(sb.fat_blocks);
    uint32_t root_start = ntohl(sb.root_start);
    uint32_t root_blocks= ntohl(sb.root_blocks);

    /* Load FAT */
    size_t fat_entries;
    uint32_t *FAT = load_fat(disk, fat_start, fat_blocks,
                             block_size, &fat_entries);

    /* Split path */
    char parts[16][32];
    int depth = split_path(path, parts, 16);
    if (depth == 0) {
    // No parts parsed, so just print the whole path as not found
    printf("Requested file %s not found in %s.\n", path, path);
    return 1;
    }


    /* Traverse directories */
    uint32_t cur_start = root_start;
    uint32_t cur_blocks= root_blocks;
    DirectoryEntry ent;

    for (int i = 0; i < depth - 1; i++) {
        int r = find_entry_in_dir(disk, FAT, fat_entries,
                                  block_size,
                                  cur_start, cur_blocks,
                                  parts[i], &ent);

        if (r != 1 || !(ent.status & 0x04)) {
            print_not_found(parts, depth, path);
            return 1;
        }

        cur_start = ent.starting_block;
        cur_blocks= ent.block_count;
    }

    /* Find file */
    int r = find_entry_in_dir(disk, FAT, fat_entries,
                              block_size,
                              cur_start, cur_blocks,
                              parts[depth - 1], &ent);

    if (r != 1 || !(ent.status & 0x02)) {
        print_not_found(parts, depth, path);
        return 1;
    }

    /* Extract */
    extract_file(disk, FAT, fat_entries, block_size, &ent, outname);

    free(FAT);
    fclose(disk);
    return 0;
}

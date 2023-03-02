#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "lfs.h"
#include "bd/lfs_rambd.h"
// #include "bd/lfs_filebd.h"

#define LFS_READ_SIZE   256
#define LFS_PROG_SIZE   256
#define LFS_BLOCK_SIZE  4096
#define LFS_BLOCK_COUNT 1024
#define LFS_CACHE_SIZE  256
#define LFS_LOOKAHEAD_SIZE  256
#define LFS_BLOCK_CYCLES    500

#define LFS_FILE_BUFFER_SIZE        LFS_CACHE_SIZE
#define LFS_READ_BUFFER_SIZE        LFS_CACHE_SIZE
#define LFS_PROG_BUFFER_SIZE        LFS_CACHE_SIZE
#define LFS_LOOKAHEAD_BUFFER_SIZE   LFS_CACHE_SIZE

lfs_t lfs;
lfs_file_t file;

static uint8_t file_buffer[LFS_FILE_BUFFER_SIZE];
static uint8_t read_buffer[LFS_READ_BUFFER_SIZE];
static uint8_t prog_buffer[LFS_PROG_BUFFER_SIZE];
static uint8_t lookahead_buffer[LFS_LOOKAHEAD_BUFFER_SIZE];

uint8_t *mem;

struct lfs_file_config f_cfg = {
    .buffer = file_buffer,
    .attr_count = 0,
    .attrs = 0,
};

void traverse_directory(const char *path) {
    lfs_dir_t dir;
    struct lfs_info info;

    int err = lfs_dir_open(&lfs, &dir, path);
    if (err) {
        printf("Failed to open directory: %s\n", path);
        return;
    }

    // if (lfs_dir_read(&lfs, &dir, &info) == 0)
    //     printf("")

    while (lfs_dir_read(&lfs, &dir, &info) != 0) {
        // 跳过 . 和 .. 目录
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) {
            continue;
        }

        if (info.type == LFS_TYPE_REG) {
            printf("File: %s/%s (%u bytes)\n", path, info.name, info.size);
            if (strcmp(path, "tmp") == 0 && strcmp(info.name, "test15") == 0)
                printf("=====\n========\n========\n");
            if (strcmp(path, "proc") == 0 && strcmp(info.name, "meminfo") == 0)
                printf("=====\nmeminfo\n============\n");
        } else if (info.type == LFS_TYPE_DIR) {
            printf("Directory: %s/%s\n", path, info.name);
            traverse_directory(info.name);
        } else {
            printf("Unknown type: %s/%s\n", path, info.name);
        }
    }

    lfs_dir_close(&lfs, &dir);
}

int main()
{
    printf("Hello world\n");

    mem = (uint8_t *)malloc(LFS_BLOCK_SIZE * LFS_BLOCK_COUNT);
    // memset(mem, 0, LFS_BLOCK_SIZE * LFS_BLOCK_COUNT);

    struct lfs_rambd_config rambd_cfg = {
        .buffer = mem,
    };

    lfs_rambd_t rambd = {
        .buffer = mem,
        .cfg = &rambd_cfg,
    };

    const struct lfs_config cfg = {
        .context = &rambd,

        .read = lfs_rambd_read,
        .prog = lfs_rambd_prog,
        .erase = lfs_rambd_erase,
        .sync = lfs_rambd_sync,

        // .read = lfs_filebd_read,
        // .prog = lfs_filebd_prog,
        // .erase = lfs_filebd_erase,
        // .sync = lfs_filebd_sync,

        .read_size = LFS_READ_SIZE,
        .prog_size = LFS_PROG_SIZE,
        .block_size = LFS_BLOCK_SIZE,
        .block_count = LFS_BLOCK_COUNT,
        .cache_size = LFS_CACHE_SIZE,
        .lookahead_size = LFS_LOOKAHEAD_SIZE,
        .block_cycles = LFS_BLOCK_CYCLES,

        .read_buffer = read_buffer,
        .prog_buffer = prog_buffer,
        .lookahead_buffer = lookahead_buffer,
    };

    lfs_rambd_createcfg(&cfg, &rambd_cfg);

    int err = lfs_mount(&lfs, &cfg);

    printf("require a format ops\n");
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    err = lfs_mkdir(&lfs, "/tmp");
    if (err) {
        printf("failed to create dir\n");
    }

    err = lfs_mkdir(&lfs, "/proc");
    if (err) {
        printf("failed to create dir\n");
    }

    int count = 0;
    while (1) {
        uint32_t boot_count = 20;
        char filename[20];
        sprintf(filename, "/tmp/test%d", count++);
        
        lfs_file_opencfg(&lfs, &file, filename, LFS_O_RDWR | LFS_O_CREAT, &f_cfg);
        // lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

        boot_count += 5;
        lfs_file_rewind(&lfs, &file);
        lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

        lfs_file_close(&lfs, &file);
        // printf("boot_count : %d, %d\n", boot_count, size);
        // if (boot_count > 20)
        //     break;
        if (count > 20)
            break;
    }

    int meminfo = (1 << 12);
    lfs_file_opencfg(&lfs, &file, "/proc/meminfo", LFS_O_RDWR | LFS_O_CREAT, &f_cfg);
    // lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &meminfo, sizeof(meminfo));

    count = 0;
    while (1) {
        uint32_t boot_count;
        char filename[20];
        sprintf(filename, "/tmp/test%d", count++);
        
        lfs_file_opencfg(&lfs, &file, filename, LFS_O_RDWR | LFS_O_CREAT, &f_cfg);
        lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
        lfs_file_close(&lfs, &file);
        printf("%s : %d\n", filename, boot_count);
        if (count > 20)
            break;
    }

    traverse_directory("/");

    lfs_unmount(&lfs);


    return 0;
}


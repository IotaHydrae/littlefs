#include <stdio.h>
#include <unistd.h>

#include "lfs.h"
#include "bd/lfs_rambd.h"
// #include "bd/lfs_filebd.h"

#define LFS_READ_SIZE   16
#define LFS_PROG_SIZE   16
#define LFS_BLOCK_SIZE  64
#define LFS_BLOCK_COUNT 4
#define LFS_CACHE_SIZE  16
#define LFS_LOOKAHEAD_SIZE  16
#define LFS_BLOCK_CYCLES    500

#define LFS_READ_BUFFER_SIZE        LFS_CACHE_SIZE
#define LFS_PROG_BUFFER_SIZE        LFS_CACHE_SIZE
#define LFS_LOOKAHEAD_BUFFER_SIZE   LFS_CACHE_SIZE

lfs_t lfs;
lfs_file_t file;

static uint8_t read_buffer[LFS_READ_BUFFER_SIZE];
static uint8_t prog_buffer[LFS_PROG_BUFFER_SIZE];
static uint8_t lookahead_buffer[LFS_LOOKAHEAD_BUFFER_SIZE];

uint8_t *mem;

struct lfs_file_config f_cfg;

int main()
{
    printf("Hello world\n");

    mem = (uint8_t *)malloc(LFS_BLOCK_SIZE * LFS_BLOCK_COUNT);

    printf("%p \n", mem);
    // struct lfs_config cfg;

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

    uint32_t boot_count = 0;

    while (1) {
        lfs_file_opencfg(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT, &f_cfg);
        lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
        printf("boot_count : %d\n", boot_count);

        boot_count += 1;
        lfs_file_rewind(&lfs, &file);
        lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

        lfs_file_close(&lfs, &file);
        usleep(10 * 1000);
    }

    lfs_unmount(&lfs);


    return 0;
}


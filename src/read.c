#include "read.h"

#include <hardware/spi.h>

#include "f_util.h"
#include "ff.h"
#include "pico/platform.h"

bool get_filename_n(int n, char* filename) {
    DIR dir;
    FILINFO fno;
    FRESULT res = f_opendir(&dir, "/");
    if (res != FR_OK) {
        panic("f_opendir on \"/\" error: %s (%d)\n", FRESULT_str(res), res);
    }
    res = f_readdir(&dir, &fno);
    if (res != FR_OK) {
        panic("f_readdir error: %s (%d)\n", FRESULT_str(res), res);
    }
    if (fno.fname[0] == 0) {
        panic("There are no files in the \"/\" directory\n");
    }
    for (int i = 0; i < n && fno.fname[0] != 0; i++) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK) {
            panic("f_readdir error: %s (%d)\n", FRESULT_str(res), res);
        }
    }
    f_closedir(&dir);
    if (fno.fname[0] == 0) {
        return false;
    }
    for (size_t i = 0; i < MAX_FILENAME_SIZE; i++) {
        filename[i] = fno.fname[i];
    }
    return true;
}

void read_and_write(enum state_machine* control, const char* filename) {
    FIL fil;
    uint8_t out_buf[SPI_BUFFER_SIZE];
    uint8_t in_buf[SPI_BUFFER_SIZE];
    FRESULT res = f_open(&fil, filename, FA_READ);
    if (res != FR_OK) {
        panic("f_open error: %s (%d)\n", FRESULT_str(res), res);
    }
    while (f_gets((char*)out_buf, sizeof(out_buf), &fil)) {
        spi_write_read_blocking(spi0, out_buf, in_buf, SPI_BUFFER_SIZE);
        for (size_t i = 0; i < SPI_BUFFER_SIZE; i++) {
            switch (in_buf[i]) {
                case 0:
                    continue;
                case 1:
                    *control = PAUSE;
                    break;
                case 2:
                    *control = SKIP;
                    break;
                case 3:
                    *control = RETURN;
                    break;
                default:
                    panic("Unknown byte received from SPI0: (%d)", in_buf[i]);
            }
            f_close(&fil);
            return;
        }
    }
    f_close(&fil);
    *control = END;
}

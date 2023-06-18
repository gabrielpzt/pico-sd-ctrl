#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <stdbool.h>
#include <stdio.h>

#include "f_util.h"
#include "ff.h"
#include "hw_config.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "read.h"
#include "rtc.h"

// SPI 0 slave configuration
#define SPI_RX_PIN 4
#define SPI_TX_PIN 3
#define SPI_CSN_PIN 5
#define SPI_SCK_PIN 6
#define SPI_FREQ 1000 * 1000

// EOF GPIO
#define EOF_PIN 7

static void pause(void) {
    for (;;) {
        uint8_t dst[SPI_BUFFER_SIZE];
        spi_read_blocking(spi0, 0, dst, SPI_BUFFER_SIZE);
        for (size_t i = 0; i < SPI_BUFFER_SIZE; i++) {
            if (dst[i] == 0) {
                return;
            }
        }
    }
}

static void setup(void) {
    stdio_init_all();
    time_init();

    gpio_init(EOF_PIN);
    gpio_set_dir(EOF_PIN, GPIO_OUT);

    spi_init(spi0, SPI_FREQ);
    spi_set_slave(spi0, true);
    gpio_set_function(SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_CSN_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
}

int main(void) {
    setup();
    sd_card_t* sd = sd_get_by_num(0);
    FRESULT res = f_mount(&sd->fatfs, sd->pcName, 1);
    if (res != FR_OK) {
        panic("f_mount error: %s (%d)", FRESULT_str(res), res);
    }
    int track_i = 0;
    char filename[MAX_FILENAME_SIZE];
    get_filename_n(track_i, filename);
    enum state_machine control = END;
    for (;;) {
        read_and_write(&control, filename);
        switch (control) {
            case PAUSE:
                pause();
                continue;
            case SKIP:
                track_i++;
                get_filename_n(track_i, filename);
                continue;
            case RETURN:
                if (track_i > 0) {
                    track_i--;
                }
                get_filename_n(track_i, filename);
                continue;
            case END:
                track_i++;
                get_filename_n(track_i, filename);
                gpio_put(EOF_PIN, 1);
                sleep_ms(1000);
                gpio_put(EOF_PIN, 0);
                continue;
        }
    }
    f_unmount(sd->pcName);
    return 0;
}

#include <hardware/gpio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "f_util.h"
#include "ff.h"
#include "hardware/pwm.h"
#include "hw_config.h"
#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "rtc.h"

/** Max filename size (chars) in the SD card formatted in FAT32 */
#define MAX_FILENAME_SIZE 256
/** Size of the buffer that reads from the SD card */
#define READ_BUFFER_SIZE 512

/** Sample rate (in Hz) of the tracks stored in the SD card */
#define SAMPLE_RATE 22050

// PWM to play the tracks
#define PWM_PIN 2
#define PWM_RANGE 255

#define SKIP_PIN 3   /**< GPIO pin to skip one track */
#define RETURN_PIN 4 /**< GPIO pin to go back one track */
#define PAUSE_PIN 5  /**< GPIO pin to pause the current track */

/**
 * Values for the state machine that controls what track is currently playing.
 */
enum state {
    OK,    /**< continue playing the current track */
    PAUSE, /**< pause until PAUSE_PIN is low */
    SKIP,  /**< move to the next track */
    RETURN /**< move to the previous track */
};

/**
 * Plays the .wav PCM encoded data feeded by the SD card using a PWM.
 * @param pcm_data bytes readed from the SD card.
 */
static void play_pcm_pwm(const uint8_t* pcm_data) {
    const uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    for (size_t i = 0; i < READ_BUFFER_SIZE; i++) {
        pwm_set_chan_level(slice_num, PWM_CHAN_A, pcm_data[i]);
        sleep_us(1000 * 1000 / SAMPLE_RATE);
    }
}

/**
 * Gets the nth filename, ordered alphabetically, from the root ("/") dir,
 * starting from 0.
 * @param n index of the filename to get, starting from 0.
 * @param filename array where the filename will be copied. If there is no file
 * in the @p n index, this parameter won't be changed.
 * @return true, if there is a file in the index passed by @p n, or false, if
 * the file doesn't exist
 */
static bool get_filename_n(int n, char* filename) {
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
    printf("%s\n", filename);
    return true;
}

/**
 * Reads the @p filename from the filesystem, then uses the configured PWM to
 * play it.
 * @param filename filename to read from the filesystem.
 * @return the next state
 */
static enum state read_track(const char* filename) {
    FIL fil;
    uint8_t pcm_data[READ_BUFFER_SIZE];
    FRESULT res = f_open(&fil, filename, FA_READ);
    if (res != FR_OK) {
        panic("f_open error: %s (%d)\n", FRESULT_str(res), res);
    }
    while (f_gets((char*)pcm_data, sizeof(pcm_data), &fil)) {
        if (gpio_get(PAUSE_PIN)) {
            return PAUSE;
        } else if (gpio_get(SKIP_PIN)) {
            return SKIP;
        } else if (gpio_get(RETURN_PIN)) {
            return RETURN;
        }
        play_pcm_pwm(pcm_data);
    }
    return OK;
}

/**
 * Waits 1000 ms until @p pin is low.
 * @param pin GPIO pin to wait.
 */
static void wait_low(int pin) {
    for (;;) {
        if (gpio_get(pin)) {
            sleep_ms(1000);
            continue;
        }
        return;
    }
}

static void setup(void) {
    stdio_init_all();
    time_init();

    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    const uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_wrap(slice_num, PWM_RANGE);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num, true);

    gpio_init(SKIP_PIN);
    gpio_set_dir(SKIP_PIN, GPIO_IN);

    gpio_init(RETURN_PIN);
    gpio_set_dir(RETURN_PIN, GPIO_IN);

    gpio_init(PAUSE_PIN);
    gpio_set_dir(PAUSE_PIN, GPIO_IN);
}

int main(void) {
    setup();
    sleep_ms(10000);  // waits so that I can setup the serial console on my end
    sd_card_t* sd = sd_get_by_num(0);
    FRESULT res = f_mount(&sd->fatfs, sd->pcName, 1);
    if (res != FR_OK) {
        panic("f_mount error: %s (%d)", FRESULT_str(res), res);
    }
    int track_i = 0;
    char filename[MAX_FILENAME_SIZE];
    get_filename_n(track_i, filename);
    for (;;) {
        enum state control = read_track(filename);
        switch (control) {
            case OK:
                track_i++;
                get_filename_n(track_i, filename);
                break;
            case PAUSE:
                printf("Paused\n");
                wait_low(PAUSE_PIN);
                break;
            case SKIP:
                printf("Skipped one track\n");
                wait_low(SKIP_PIN);
                track_i++;
                get_filename_n(track_i, filename);
                break;
            case RETURN:
                printf("Returned one track\n");
                wait_low(RETURN_PIN);
                if (track_i > 0) {
                    track_i--;
                }
                get_filename_n(track_i, filename);
                break;
        }
    }
    f_unmount(sd->pcName);
    return 0;
}

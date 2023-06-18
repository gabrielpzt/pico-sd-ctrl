#ifndef READ_H
#define READ_H

#include <stdbool.h>

#define MAX_FILENAME_SIZE 256
#define SPI_BUFFER_SIZE 512

/**
 * Parameters passed from the FPGA MOSI line to control what is sent by the
 * MISO.
 */
enum state_machine { PAUSE, SKIP, RETURN, END };

/**
 * Gets the nth filename, ordered alphabetically, from the root ("/") dir,
 * starting from 0.
 * @param n index of the filename to get, starting from 0.
 * @param filename array where the filename will be copied. If there is no file
 * in the @p n index, this parameter won't be changed.
 * @returns true, if there is a file in the index passed by @p n, or false, if
 * the file doesn't exist
 */
bool get_filename_n(int n, char* filename);

/**
 * Reads the @p filename from the filesystem, then writes and read from the SPI
 * line to the FPGA, changing the @p control parameter based on what is
 * received.
 * @param control state machine's next value.
 * @param filename filename to read from the filesystem.
 */
void read_and_write(enum state_machine* control, const char* filename);

#endif

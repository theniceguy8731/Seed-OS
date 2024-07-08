/*
 * No changes are allowed to this file
 */
#define _XOPEN_SOURCE 700
#define __USE_XOPEN_EXTENDED
#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>

void load_and_run_elf(char** argv);
void loader_cleanup();

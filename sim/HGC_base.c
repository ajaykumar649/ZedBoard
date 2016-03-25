#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/types.h>               //__u8, __u16      ...
#include <string.h>                    //strlen()         ...
#include <sys/mman.h>                  //mmap(), munmap() ...
#include <unistd.h>                    //gets()           ...
#include "hgc_util.h"

int main(void)
{
    int memfd;
    char str[255];
    volatile void * zed_ch0;
    unsigned int Data32Write;
    unsigned int Data32Read;
    //---------------------------------------------------------------------
    // Open /dev/mem file
    //---------------------------------------------------------------------
    memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd == -1)
    {
        printf("Can't open /dev/mem\r\n");
        exit(0);
    }
    printf("/dev/mem opened\r\n");
    printf("===============\r\n");
    printf("Test loop\r\n");
    printf("===============\r\n");

    //---------------------------------------------------------------------
    // Map the device into memory.
    // Map one page of memory into user space such that the device is
    // in that page, but it may not be at the start of the page.
    //---------------------------------------------------------------------
    unsigned long page_size = sysconf(_SC_PAGESIZE);
    printf("page_size=0x%.8lX\r\n", page_size);

    zed_ch0 = mmap(0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd,
            (BASE_ZED_CH) & ~page_size);
    if (zed_ch0 == (void *) -1)
    {
        printf("Can't map the memory=0x%.8lX to user space\r\n",
                (long unsigned int) (BASE_ZED_CH));
        exit(0);
    }
    printf(
            "HW zed_ch0 Memory=0x%.8lX mapped to user space at mapped_base=%p\r\n",
            (long unsigned int) (BASE_ZED_CH), (void *) zed_ch0);

    Data32Write = 0;
    int i = 0;
    int looping = 1;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x8000;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    printf("***reset fpga*\r\n");
    //
    //	while (looping < 5) {
    //			*((volatile unsigned long *)(zed_ch0 + CMD_AND_STATUS)) = 0;
    //			Data32Read = *((volatile unsigned long *)(zed_ch0 + CMD_AND_STATUS));
    //			printf("reading status:%x \r\n",(Data32Read));
    //		looping++;
    //	}
    //
    //	fflush(stdin);
    //	printf("Enter to continue....\r\n");
    //	gets(str);

    while (looping < 2)
    {
        *((volatile unsigned long *) (zed_ch0 + CMD_AND_STATUS)) = 0;
        Data32Read = *((volatile unsigned long *) (zed_ch0 + CMD_AND_STATUS));
        printf("reading status:%x \r\n", (Data32Read));
        looping++;
    }
    looping = 1;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x100;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    printf("***clear stat***\r\n");

    while (looping < 2)
    {
        *((volatile unsigned long *) (zed_ch0 + CMD_AND_STATUS)) = 0;
        Data32Read = *((volatile unsigned long *) (zed_ch0 + CMD_AND_STATUS));
        printf("reading status:%x \r\n", (Data32Read));
        looping++;
    }
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x200;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    printf("***Send SYNC***\r\n");
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x100;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    printf("***clear stat***\r\n");

    *str = '\0';
    printf("Enter again....\r\n");
    gets(str);
    while (looping < 2)
    {
        *((volatile unsigned long *) (zed_ch0 + CMD_AND_STATUS)) = 0;
        Data32Read = *((volatile unsigned long *) (zed_ch0 + CMD_AND_STATUS));
        printf("reading status:%x \r\n", (Data32Read));
        looping++;
    }

    printf("Now ready to write config....\r\n");
    process_cmd("H", zed_ch0);
    looping = 1;

    //	unsigned int test_data=0;
    //	unsigned int read_data=0;
    //	unsigned int *ret_ptr = &read_data;

    while (looping < 50)
    {
        printf(">");
        gets(str);
        process_cmd(str, zed_ch0);

    }

    //---------------------------------------------------------------------
    // Unmap the memory before exiting
    //---------------------------------------------------------------------
    (munmap((void *) zed_ch0, page_size));
    //---------------------------------------------------------------------
    // Closing /dev/mem file
    //---------------------------------------------------------------------
    if (close(memfd) == -1)
    {
        printf("Can't close /dev/mem\r\n");
        exit(0);
    }
    printf("/dev/mem closed\r\n");
    return 0;
}

//unsigned long ReadStatus(volatile void * zed_ch0)
//{
//
//}


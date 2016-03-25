#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <ctype.h>
#include "hgc_util.h"

/*
 * See http://stackoverflow.com/questions/266357/tokenizing-strings-in-c
 */

/* Tokenize the string, parse the first token as string, parse the other tokens as HEX */
void tokenize_first_str(const char * str, char * myCmd, unsigned int parCount[], unsigned int myVal[])
{
    char buffer[256];
    strcpy(buffer, str);
    char* token = strtok(buffer, " ");

    unsigned int count = 0;

    char * end;
    int i = 0;

    while (token != NULL) {
        /* Do stuff */
        if (i == 0) {
            memcpy(myCmd, token, strlen(token) + 1); //myCmd[0] = token[0];
        } else {
            /* Parse as HEX */
            myVal[i - 1] = strtoul(token, &end, 16);
        }
        token = strtok(NULL, " ");
        ++i;
        count = i - 1;
    }
    parCount[0] = count;
}

void TriggerAndReadoutCycle(long zed_ch0) //split off here for readability
{
    unsigned int myVal[256];
    unsigned int retVal[256];
    unsigned int i = 0;
    //reset the SKIROC
    myVal[0] = 0x5;
    myVal[1] = 0x20;
    WriteArtix(myVal, retVal, zed_ch0);
    //--    printf("WRITE: 0x%08X  0x%08X\n", myVal[0], myVal[1]);
    //-- a little delay here

    //arm trig
    myVal[0] = 0x5;
    myVal[1] = 0x28;
    WriteArtix(myVal, retVal, zed_ch0);
    myVal[0] = 0x5;
    myVal[1] = 0x128;
    WriteArtix(myVal, retVal, zed_ch0);

    //delay
    myVal[0] = 2;
    for (i = 0; i < 10; i++) {	
        ReadZed(myVal, retVal, zed_ch0);	
    }
    //trig
    myVal[0] = 0x5;
    myVal[1] = 0x12c;
    WriteArtix(myVal, retVal, zed_ch0);

    //delay
    myVal[0] = 2;
    for (i = 0; i < 10; i++) {	
        ReadZed(myVal, retVal, zed_ch0);	
    }
    SoftwareTrig(zed_ch0);
    // waste a bit of time
    myVal[0] = 2;
    for (i = 0; i < 1000; i++) {
        ReadZed(myVal, retVal, zed_ch0);
        if (i == 0) {
            printf("t= 0x%08X\n", retVal[0]);
        }
        if (i == 999) {
            printf("t= 0x%08X\n", retVal[0]);
        }
    }
    // transfer data from FMC_IO
    myVal[0] = 6; //dev for data mem
    myVal[1] = 0; //starting addr
    myVal[2] = 0x84; //len of xfer
    ReadArtix(myVal, retVal, zed_ch0);
    printf("0: 0x%08X\n", retVal[0]);

    // spit the data out to screen
    //need a little delay here?
    myVal[0] = 2;
    for (i = 0; i < 10; i++)
    {
        ReadZed(myVal, retVal, zed_ch0);
    }
    printf("START\n");
    myVal[0] = 0x84;
    {
        int i = 0;
        int j = 0;
        ReadData(myVal, retVal, zed_ch0);
        for (i = 0; i < myVal[0]; i++) {
            printf("0x%02X  0x%08X \n", 256 * j + i, retVal[i]);
        }
        printf("DONE\n");
    }
}

//Check if rec'd command is valid and handle it.
int process_cmd(const char * str, long zed_ch0)
{
    char myCmd[25];
    unsigned int parCount[1];
    unsigned int myVal[256];
    unsigned int retVal[256];
    tokenize_first_str(str, myCmd, parCount, myVal);
    //unsigned int parErr = 0;
    char fl = myCmd[0];
    char f2 = myCmd[1];
    fl = toupper(fl);
    f2 = toupper(f2);

    switch (fl) {
        //command parameter search
        case 'H':
            //if (!strcmp(tok, "H")) //help
            {
                printf("HELP:\n");
                printf("	N: TIME:\n");
                printf("	T: SOFTWARE TRIG:\n");
                printf("	R: SKIROC digital reset:\n");
                printf("	TR: SOFTWARE TRIG and READOUT:\n");
                printf("	F: FW Version for Artix on FMC_IO:\n");
                printf("	WA: WRITE to Artix:{0}=dev num, {1}=value (addr is always 0)\n");
                printf("	WZ: WRITE to Zed: Addr Value\n");
                printf("	RA: READ from Artix:{0}=dev num, {1}=addr, {2}=length :\n");
                printf("	RZ: READ from Zed: Addr\n");
                printf("	RD: READ DATA:{0}=length:\n");

                printf("	X: SYNC: Force SYNC state \n");
                printf("	C: CLEAR: clear ZED status\n");
                printf("	S: STATUS: read ZED Status\n");

                printf("	D: DOWNLOAD: meant for machine, not YOU\n");
                printf("	Note:\n");
                printf("	to reset Slow Control, write WA 2 0, WA 2 2\n");
            }
            break;

        case 'T':
            if (f2 == 'R') //trig end read
            {
                TriggerAndReadoutCycle(zed_ch0);
            } else {
                SoftwareTrig(zed_ch0);
            }
            break;

        case 'F': //FA
            //	if (!strcmp(f2, "A")) //read FW version for Artix
            {
                FirmwareVerA(myVal, zed_ch0);
                printf("FW: 0x%08X\n", myVal[0]);
            }
            break;

        case 'W': //WZ
            if (f2 == 'Z') //write zed
            {
                WriteZed(myVal, zed_ch0);
                printf("WRITE: 0x%08X  0x%08X\n", myVal[0], myVal[1]);
            }
            if (f2 == 'A') //write artix
            {
                WriteArtix(myVal, retVal, zed_ch0);
                printf("WRITE: 0x%08X  0x%08X\n", myVal[0], myVal[1]);
            }
            break;

        case 'R': //RZ
            if (f2 == 'Z') //read zed
            {
                ReadZed(myVal, retVal, zed_ch0);
                printf("READ 0x%08X 0x%08X\n", myVal[0], retVal[0]);
            }
            if (f2 == 'A') //Read artix
            {
                ReadArtix(myVal, retVal, zed_ch0);
                printf("READ: 0x%08X  0x%08X =>\n", myVal[0], myVal[1]);
                printf("0: 0x%08X\n", retVal[0]);
            }
            if (f2 == 'D') //Read data
            {
                int i = 0;
                int len = myVal[0];
                int j = 0;
                while (len > 256)
                {
                    myVal[0] = 256;
                    ReadData(myVal, retVal, zed_ch0);
                    for (i = 0; i < myVal[0]; i++)
                    {
                        printf("0x%02X  0x%08X =>\n", 256 * j + i, retVal[i]);
                    }
                    len = len - 256;
                    j++;
                }
                if (len > 0)
                {
                    myVal[0] = len;
                    ReadData(myVal, retVal, zed_ch0);
                    for (i = 0; i < myVal[0]; i++)
                    {
                        printf("0x%02X  0x%08X \n", 256 * j + i, retVal[i]);
                    }
                }
                printf("DONE\n");
            }
            break;

        case 'D':
            {
                int i = 0;
                WriteConfig(myVal, retVal, zed_ch0);
                for (i = 0; i < myVal[0]; i++)
                {
                    printf("CONFIG 0x%02X 0x%08X\n", i, retVal[i]);
                }
            }
            break;

        case 'C':
            {
                Clear(zed_ch0);
                printf("CLEAR\n");
            }
            break;

        case 'X':
            {
                SYNC(zed_ch0);
                printf("SYNC\n");
            }
            break;

        case 'S':
            //
            //if (!strcmp(tok, "STAT"))                 //read
            {
                if (myVal[0] == 0)
                {
                    myVal[0] = 1;
                }
                ReadStatus(myVal, retVal, zed_ch0);
                printf("STATUS 0x%08X 0x%08X\n", myVal[0], retVal[0]);
            }
            break;
        case 'N':
            {
                system("date");
            }

        case 'Q':
            break;

        default:
            printf("\r\nsyntax error?\r\n");      //send to current active port
            break;
    }      //end switch(*tok)

    return 0;
}


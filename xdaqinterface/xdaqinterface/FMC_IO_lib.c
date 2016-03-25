/*
 * FMC_IO_lib.c
 *
 *  Created on: Feb 11, 2016
 *      Author: rubinov
 */
#include "hgc_util.h"

//THINGS YOU MIGHT WANT TO KNOW:
//	constant HEADER_INDEX_INFOCODE_LOW          : integer := 0 ;
//	constant HEADER_INDEX_INFOCODE_HIGH         : integer := 3 ;
//	constant HEADER_INDEX_TYPEID_LOW            : integer := 4 ;
//	constant HEADER_INDEX_TYPEID_HIGH           : integer := 7 ;
//	constant HEADER_INDEX_LENGTH32_LOW          : integer := 8 ;
//	constant HEADER_INDEX_LENGTH32_HIGH         : integer := 15 ;
//	constant HEADER_INDEX_ADDRESS32_LOW         : integer := 16 ;
//	constant HEADER_INDEX_ADDRESS32_HIGH        : integer := 27 ;
//	constant HEADER_INDEX_DEVICE_LOW            : integer := 28 ;
//	constant HEADER_INDEX_DEVICE_HIGH           : integer := 31 ;
//	constant HEADER_INDEX_LOW                   : integer := 0 ;
//	constant HEADER_INDEX_HIGH                  : integer := 31 ;
//
//	--
//	constant PACKET_HEADER_INFOCODE_RESPONSE_OK : std_logic_vector(3 downto 0):=X"0";
//	constant PACKET_HEADER_INFOCODE_BAD_HEADER  : std_logic_vector(3 downto 0):=X"1";
//	constant PACKET_HEADER_INFOCODE_TIMEOUT     : std_logic_vector(3 downto 0):=X"2";
//	constant PACKET_HEADER_INFOCODE_REQUEST     : std_logic_vector(3 downto 0):=X"F";
//	--
//	constant PACKET_HEADER_TYPEID_WRITE         : std_logic_vector(3 downto 0):=X"1";
//	constant PACKET_HEADER_TYPEID_READ          : std_logic_vector(3 downto 0):=X"2";
//	--
//	constant PACKET_HEADER_ADDR_DEV_ERROR       : std_logic_vector(3 downto 0):=X"0";
//	constant PACKET_HEADER_ADDR_DEV_STATUS      : std_logic_vector(3 downto 0):=X"1";
//	constant PACKET_HEADER_ADDR_DEV_CONFIG      : std_logic_vector(3 downto 0):=X"2";
//	constant PACKET_HEADER_ADDR_DEV_FW_VERSION  : std_logic_vector(3 downto 0):=X"3";
//	constant PACKET_HEADER_ADDR_DEV_PARAMETERS  : std_logic_vector(3 downto 0):=X"4";
//	constant PACKET_HEADER_ADDR_DEV_CONTROL     : std_logic_vector(3 downto 0):=X"5";
//	constant PACKET_HEADER_ADDR_DEV_READOUT     : std_logic_vector(3 downto 0):=X"6";

void SYNC(long zed_ch0)
{
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x200;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
}

void Clear(long zed_ch0)
{
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x100;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
}

void SKIROC_Reset(long zed_ch0)
{

}

void SoftwareTrig(long zed_ch0)
{
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x400;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
}

void ReadStatus(unsigned int rep[], unsigned int ret[], long zed_ch0)
{
    unsigned int Data32Read1;
    //    unsigned int i = rep[0];
    //    unsigned int j = 0;
    Data32Read1 = *((volatile unsigned long *) (zed_ch0 + CMD_AND_STATUS));
    ret[0] = Data32Read1;
}

void WriteZed(unsigned int xData[], long zed_ch0)
{
    long addr = (long) xData[0];
    unsigned int val = xData[1];
    addr = addr * 4;
    *((volatile unsigned int *) (zed_ch0 + addr)) = val;
}

void ReadZed(unsigned int xData[], unsigned int ret[], long zed_ch0)
{
    unsigned int Data32Read = 0;
    long addr = (long) xData[0];
    addr = addr * 4;
    Data32Read = *((volatile unsigned long *) (zed_ch0 + addr));
    ret[0] = Data32Read;
}

void ReadData(unsigned int xData[], unsigned int ret[], long zed_ch0)
{
    unsigned long Hdr32Read = 0;
    unsigned long rd[256];
    int i = 0;
    int len = (int) xData[0];
    //assuming received, read mem pointer
    //reset rcv mem ptr
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x10;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    //read mem
    //    Hdr32Read = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
    //    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x20;

    //strobe inc
    for (i = 0; i < len; i++)
    {
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x20;
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
        rd[i] = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
        //printf("read:%x %x\r\n",Data32Read1,Data32Read2);
    }
    for (i = 0; i < len; i++)
    {
        ret[i] = rd[i];
    }
}

void WriteConfigTest(unsigned int xData[], unsigned int ret[], long zed_ch0)
{
    // unsigned int Data32Write;
    unsigned int Data32Read2;
    unsigned int Data32Read1;
    unsigned int FMC_IO_CONFIG_header = 0x2000021F;
    //reset send mem
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x1;
    //put data on bus
    *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) =
        FMC_IO_CONFIG_header;
    //strobe write
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
    //now the data
    *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = xData[0];
    //strobe write
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
    //send pack
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x800;

    //printf("sending: %x \r\n",(xData));
    //read status

    //assuming received, read mem pointer
    //reset rcv mem ptr
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x10;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    //read mem
    Data32Read1 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x20;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    Data32Read2 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
    //printf("read:%x %x\r\n",Data32Read1,Data32Read2);
    ret[0] = Data32Read2;

}

void WriteArtix(unsigned int xData[], unsigned int ret[], long zed_ch0)
{
    // unsigned int Data32Write;
    unsigned int Data32Read2;
    unsigned int Data32Read1;
    unsigned int dev_num = xData[0];
    unsigned int FMC_IO_header = 0x0000021F;    //(21f=len+write+req)
    FMC_IO_header = FMC_IO_header | (dev_num << 28);
    //reset send mem
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x1;
    //put data on bus
    *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = FMC_IO_header;
    //strobe write
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
    //now the data
    *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = xData[1];
    //strobe write
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
    //send pack
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x800;

    //printf("sending: %x \r\n",(xData));
    //read status

    //assuming received, read mem pointer
    //reset rcv mem ptr
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x10;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    //read mem
    Data32Read1 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x20;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    Data32Read2 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
    //printf("read:%x %x\r\n",Data32Read1,Data32Read2);
    ret[0] = Data32Read2;
}

void ReadArtix(unsigned int xData[], unsigned int ret[], long zed_ch0)
{
    // unsigned int Data32Write;
    unsigned int Data32Read2;
    unsigned int Data32Read1;
    unsigned int dev_num = xData[0];
    unsigned int start_addr = xData[1];
    unsigned int req_len = xData[2];
    unsigned int FMC_IO_header = 0x0000002F;    //(21f=len+write+req)

    start_addr = (start_addr & 0xfff); //12 bits
    dev_num = (dev_num & 0xf); //4 bits
    req_len = (req_len & 0xff); //8 bits

    FMC_IO_header = FMC_IO_header | (dev_num << 28);
    FMC_IO_header = FMC_IO_header | (req_len << 8);
    FMC_IO_header = FMC_IO_header | (start_addr << 16);
    //reset send mem
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x1;
    //put data on bus
    *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = FMC_IO_header;
    //strobe write
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
    //now the data
    *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = xData[1];
    //strobe write
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
    //send pack
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x800;

    //printf("sending: %x \r\n",(xData));
    //read status

    //assuming received, read mem pointer
    //reset rcv mem ptr
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x10;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    //read mem
    Data32Read1 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x20;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    Data32Read2 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
    //printf("read:%x %x\r\n",Data32Read1,Data32Read2);
    ret[0] = Data32Read2;
}

void WriteConfig(unsigned int xData[], unsigned int ret[], long zed_ch0)
{
    // unsigned int Data32Write;
    unsigned int len;
    unsigned int i = 0;
    unsigned int Data32Read1;
    unsigned int Data32Read2;
    unsigned int FMC_IO_CONFIG_header = 0x2000021F;

    len = xData[0];

    for (i = 0; i < len; i++)
    {
        //reset send mem
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x1;
        //put data on bus
        *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) =FMC_IO_CONFIG_header;
        //strobe write
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
        //strobe inc
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
        //now the data
        if (xData[i + 1]==1) {
            *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = 0xd;
        } else {
            *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = 0x9;
        }
        //strobe write
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
        //strobe inc
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
        //send pack
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x800;

        //assuming received, read mem pointer
        //reset rcv mem ptr
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x10;
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
        //read mem
        Data32Read1 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
        //strobe inc
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x20;
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
        Data32Read1 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
        //reset send mem
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x1;
        //put data on bus
        *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) =FMC_IO_CONFIG_header;
        //strobe write
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
        //strobe inc
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
        //now the data
        if (xData[i + 1]==1)
        {*((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = 0xf;}
        else
        {*((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = 0xb;}
        //strobe write
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
        //strobe inc
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
        //send pack
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x800;

        //assuming received, read mem pointer
        //reset rcv mem ptr
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x10;
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
        //read mem
        Data32Read2 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
        //strobe inc
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x20;
        *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
        Data32Read2 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
        ret[i] = Data32Read2;
    }
}

void FirmwareVerA(unsigned int ret[], long zed_ch0)
{
    unsigned int Data32Read1;
    unsigned int Data32Read2;
    unsigned int FMC_IO_FWV_header = 0x3000012F; //dev=3 (12f=len+read+req)
    //reset send mem
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x1;
    //put data on bus
    *((volatile unsigned int *) (zed_ch0 + SEND_MEM_DATA)) = FMC_IO_FWV_header;
    //strobe write
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x4;
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x2;
    //send pack
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x800;

    //printf("sending: %x \r\n",(xData));
    //read status

    //assuming received, read mem pointer
    //reset rcv mem ptr
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x10;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    //read mem
    Data32Read1 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
    //strobe inc
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0x20;
    *((volatile unsigned int *) (zed_ch0 + CMD_AND_STATUS)) = 0;
    Data32Read2 = *((volatile unsigned long *) (zed_ch0 + RECV_MEM_DATA));
    //printf("read:%x %x\r\n",Data32Read1,Data32Read2);
    ret[0] = Data32Read2;
}


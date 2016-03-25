/*
 * hgc_util.h
 *
 *  Created on: Feb 11, 2016
 *      Author: rubinov
 */

#ifndef HGC_UTIL_H_
#define HGC_UTIL_H_


/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define BASE_ZED_CH              0x43c00000
//from file xgpio_l.h ....
#define SEND_MEM_DATA            0   /**< Data register for 1st channel */
#define RECV_MEM_DATA            4   /** */
#define SCRATCH		             8   /** */
#define CMD_AND_STATUS           0xC   /** */

void SYNC(long zed_ch0);
void Clear(long zed_ch0);
void SoftwareTrig(long zed_ch0);
void FirmwareVerA(unsigned int ret[], long zed_ch0);
void ReadArtix(unsigned int xData[], unsigned int ret[], long zed_ch0);
void WriteArtix(unsigned int xData[], unsigned int ret[], long zed_ch0);
void ReadStatus(unsigned int rep[], unsigned int ret[], long zed_ch0);
void ReadZed(unsigned int xData[], unsigned int ret[], long zed_ch0);
void WriteZed(unsigned int xData[], long zed_ch0);
void ReadStatus(unsigned int rep[], unsigned int ret[], long zed_ch0);
void ReadData(unsigned int xData[], unsigned int ret[], long zed_ch0);
void WriteConfigTest(unsigned int xData[], unsigned int ret[], long zed_ch0);
int process_cmd(const char * str,long zed_ch0);



#endif /* HGC_UTIL_H_ */

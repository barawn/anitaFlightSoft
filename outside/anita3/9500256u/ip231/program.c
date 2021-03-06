
#include "../carrier/carrier.h"
#include "ip231.h"


/*
{+D}
        SYSTEM:		IP231

        NAME:       WriteCoefficients.c

        VERSION:        B

        CREATION DATE:  09/24/04
        
        DESIGNED BY:    F.J.M.

        CODED BY:       F.J.M.

        ABSTRACT:

        CALLING
	 SEQUENCE:      status = int WriteCoefficients( c_blk )
			ptr (pointer to structure)
			    Pointer to the configuration block structure.

        MODULE TYPE:    int

        I/O RESOURCES:          

        SYSTEM RESOURCES:       

        MODULES CALLED:

        REVISIONS:

  DATE	     BY	    PURPOSE
  --------  ----    ------------------------------------------------
  04/01/11   FJM    Changed carrier include to carrier.h

{-D}
*/

/*
	MODULE FUNCTIONAL DETAILS:

    ****************************************************************
    * NOTE: IT IS RECOMMENDED THAT THESE  VALUES  NOT  BE  CHANGED *
    * UNLESS  YOU  ARE   FAMILIAR  WITH  MAKING   THESE  TYPES  OF *
    * CALCULATIONS   AND  HAVE   A  VOLTMETER  CAPABLE  OF  16-BIT *
    * RESOLUTION AND ACCURACY. IF THESE VALUES ARE CHANGED AND NOT *
    * PROPERLY CALCULATED, THE  BOARD  WILL HAVE TO BE RETURNED TO *
    * THE FACTORY FOR CALIBRATION.  FOR THIS REASON, THIS FUNCTION *
    * IS NOT SUPPORTED FROM THE DEMONSTRATION PROGRAM.             * 
    ****************************************************************
*/



/* Write the gain and offset error coefficients to IP231 EEPROM */


int WriteCoefficients( c_blk )
struct cblk231 *c_blk;
{
    int i, count;
    unsigned char readback;

    output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_control, 3);	/* Enable eewrites */
	usleep( 12 );				/* sleep time */

    /* Write the calibration coefficients to EEPROM */
    for(i = 0; i < 16; i++)
    {
       /* Write offset error MSbyte */
	readback = count = 0;
	output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->cal_map[i].offset_msb, (byte)( c_blk->off_buf[i] >> 8));
	while(readback == 0)
	{
		usleep( 2000 );					/* sleep time */
		if(++count > 100)
		{
		   output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_control, 0);	/* disable eewrites */
		   usleep( 12 );				/* sleep time */
		   output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->control_reg, 0x80);	/* reset */
		   return -1;
		}
		readback = input_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_status);
	}
    usleep( 12 );				/* sleep time */

       /* Write offset error LSbyte */
	readback = count = 0;
	output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->cal_map[i].offset_lsb, (byte)c_blk->off_buf[i]);
	while(readback == 0)
	{
		usleep( 2000 );					/* sleep time */
		if(++count > 100)
		{
		   output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_control, 0);	/* disable eewrites */
		   usleep( 12 );				/* sleep time */
		   output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->control_reg, 0x80);	/* reset */
		   return -2;
		}
		readback = input_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_status);
	}
    usleep( 12 );				/* sleep time */



       /* Write gain error MSbyte */
	readback = count = 0;
	output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->cal_map[i].gain_msb, (byte)( c_blk->gain_buf[i] >> 8));
	while(readback == 0)
	{
		usleep( 2000 );					/* sleep time */
		if(++count > 100)
		{
		   output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_control, 0);	/* disable eewrites */
		   usleep( 12 );				/* sleep time */
		   output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->control_reg, 0x80);	/* reset */
		   return -3;
		}
		readback = input_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_status);
	}
    usleep( 12 );				/* sleep time */



       /* Write gain error LSbyte */
	readback = count = 0;
	output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->cal_map[i].gain_lsb, (byte)c_blk->gain_buf[i]);
	while(readback == 0)
	{
		usleep( 2000 );					/* sleep time */
		if(++count > 100)
		{
		   output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_control, 0);	/* disable eewrites */
		   usleep( 12 );				/* sleep time */
		   output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->control_reg, 0x80);	/* reset */
		   return -4;
		}
		readback = input_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_status);
	}
    usleep( 12 );				/* sleep time */
    }


    output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->ee_control, 0);	/* disable eewrites */
    usleep( 12 );				/* sleep time */
    output_byte( c_blk->nHandle, (byte*)&c_blk->brd_ptr->control_reg, 0x80);	/* reset */
	usleep( 2000 );					/* sleep time */
    return(0);
}

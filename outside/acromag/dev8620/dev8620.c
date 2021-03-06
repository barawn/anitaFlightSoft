
/*
{+D}
   SYSTEM:          Acromag PCI carrier

   MODULE NAME:     dev8620.c

   VERSION:         G

   CREATION DATE:   07/03/02

   CODED BY:        FJM

   ABSTRACT:        8620 carrier device.

   CALLING
	SEQUENCE:

   MODULE TYPE:

   I/O RESOURCES:

   SYSTEM
	RESOURCES:

   MODULES
	CALLED:

   REVISIONS:

 DATE      BY       PURPOSE
-------- ----  ------------------------------------------------
07/03/02 FJM   Fixed unregister bug in cleanup_module().
09/10/03 FJM   Red Hat Linux 9 update.
12/22/04 FJM   Fedora core FC3 update.
01/20/05 FJM   Extended carrier function library interface.
05/25/05 FJM   Add multiple carrier support.
12/06/06 FJM   Fedora core FC6 update.
01/11/07 FJM   Add support x86_64
06/12/07 FJM   Fedora 7 update.

{-D}
*/


/* APC8620 device */





#ifndef BUILDING_FOR_KERNEL
#define BUILDING_FOR_KERNEL	/* controls conditional inclusion in file apc8620.h */
#endif


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>


#include "../carrier/apc8620.h"
/*
#include "../ip1k100/ip1k100.h"
#include "../ip1k110/ip1k110.h"
#include "../ip220/ip220.h"
#include "../ip230/ip230.h"
#include "../ip231/ip231.h"
#include "../ip236/ip236.h"
#include "../ip320/ip320.h"
#include "../ip330/ip330.h"
#include "../ip340/ip340.h"
#include "../ip400/ip400.h"
#include "../ip405/ip405.h"
*/
#include "../ip408/ip408.h"
/*
#include "../ip409/ip409.h"
#include "../ip445/ip445.h"
#include "../ip470/ip470.h"
#include "../ip480/ip480.h"
#include "../ip482/ip482.h"
*/


#define DEVICE_NAME	"apc8620_"	/* the name of the device */
#define MAJOR_NUM	46


int open_dev[MAX_CARRIERS];
unsigned int board_irq[MAX_CARRIERS];
unsigned long carrier_address[MAX_CARRIERS];
unsigned long ip_mem_address[MAX_CARRIERS];
int ret_val;



struct file;
struct inode;
struct pt_regs;



static int
open( struct inode *inode, struct file *fp )
{
   int minor;

   minor = inode->i_rdev & 0xf;
   if( minor > 3 )
	   return( -ENODEV );
  
   if( open_dev[minor] )
	   return( -EBUSY );

   open_dev[minor] = 1;

   return( 0 );
}

static int
release( struct inode *inode, struct file *fp )
{
   int minor;

   minor = inode->i_rdev & 0xf;
   if( minor > 3 )
	   return( -ENODEV );

   if( open_dev[minor] )
   {
	   open_dev[minor] = 0;
	   return( 0 );
   }
   return( -ENODEV );
}


static ssize_t
read( struct file *fp, char *buf, size_t length, loff_t *offset )
{ 
	unsigned long adata, ldata ;
	unsigned short sdata;
	unsigned char cdata;

	get_user( adata, (unsigned long *)buf );		/* pickup address */
	switch( length )
	{
		case 1:	/* 8 bit */
		   cdata = readb( (void *) adata );
		   ldata = ( unsigned long )cdata;		/* convert to long */
		break;
		case 2:	/* 16 bit */
		   sdata = readw( (void *) adata );
		   ldata = ( unsigned long )sdata;		/* convert to long */
		break;
		case 4:	/* 32 bit */
		   ldata = readl( (void *) adata );
		break;
		default:
		    cdata = sdata = adata = ldata = 0;
		    return( -EINVAL );
		break;
	}
	put_user( ldata,(unsigned long *)( buf + (sizeof(unsigned long)) ) );	/* update user data */
	return( length );
}



static ssize_t
write( struct file *fp, const char *buf, size_t length, loff_t *offset )
{ 
	unsigned long adata, ldata ;

	get_user( adata, (unsigned long *)buf );				/* pickup address */
	get_user( ldata, (unsigned long *)( buf + (sizeof(unsigned long)) ) );	/* pickup data */
	switch( length )
	{
		case 1:	/* 8 bit */
		   writeb( (int)ldata, (void *)adata );
		break;
		case 2:	/* 16 bit */
		   writew( (int)ldata, (void *)adata );
		break;
		case 4:	/* 32 bit */
		   writel( (int)ldata, (void *)adata );
		break;
		default:
		    return( -EINVAL );
		break;
	}
    return( length );
}



static int
ioctl( struct inode *inode, struct file *fp, unsigned int cmd, unsigned long arg)
{
    unsigned long ldata;
    int i;
	switch( cmd )
	{
		case 0:	/* reserved for future use */
		case 1:
		case 2:
		case 3:
		case 4:/* return IP MEM address */
            for(i = 0; i < MAX_CARRIERS; i++)                   /* get all boards */
            {
               ldata = ( unsigned long )ip_mem_address[i];      /* convert to long */
               put_user( ldata, (unsigned long *)(arg+(i*(sizeof(unsigned long)))) );	/* update user data */
            }    
		break;
		case 5:/* return IP I/O address */
            for(i = 0; i < MAX_CARRIERS; i++)                    /* get all boards */
            {
               ldata = ( unsigned long )carrier_address[i];      /* convert to long */
               put_user( ldata, (unsigned long *)(arg+(i*(sizeof(unsigned long)))) );	/* update user data */
            }    
		break;
		case 6:/* return IRQ number */
            for(i = 0; i < MAX_CARRIERS; i++)                    /* get all boards */
            {
               ldata = ( unsigned long )board_irq[i];            /* convert IRQ to long */
               put_user( ldata, (unsigned long *)(arg+(i*(sizeof(unsigned long)))) );	/* update user data */
            }
		break;
	}
	return( cmd );
}


static struct file_operations apc8620_ops = {
  .owner = THIS_MODULE, /* owner of the world */
  NULL,                 /* seek */
  .read = read,         /* read */
  .write = write,       /* write */
  NULL,                 /* readdir */
  NULL,                 /* poll */
  .ioctl = ioctl,       /* ioctl */
  NULL,                 /* mmap */
  .open = open,         /* open */
  NULL,                 /* flush */
  .release = release,   /* release */
  NULL,                 /* fsync */
  NULL,                 /* fasync */
  NULL,                 /* lock */ 
  NULL,                 /* readv */
  NULL,                 /* writev */
  NULL,                 /* sendpage */
  NULL                  /* get_unmapped_area */
};


static irqreturn_t
apc8620_handler( int irq, void *did, struct pt_regs *cpu_regs )
{ 
   volatile unsigned short dummyWord, nValue;
   volatile PCI_BOARD_MEMORY_MAP* pPCICard;
   int int_status;
   struct isr_data idata;		/* ISR routine handler structure */			
   struct handler_data hdata;	/* wrapper structure for isr_data */



   hdata.hd_ptr = (char *)&idata;/* get address of isr_data struct to handler data struct */
   int_status = 0;	/* indicate interrupt not handled */
 
   if( open_dev[0] )	/*	Check the 1st board open_dev[0] */
   {
     pPCICard = (PCI_BOARD_MEMORY_MAP*)carrier_address[0];
     nValue = readw((unsigned short*)&pPCICard->intPending);

	 if( nValue & CARRIER_INT_MASK )/* non-zero if this carrier is interrupting */
	 {
/*
printk("\nopen_dev[0] pending %X",nValue);
*/
		/* Check each IP slot for an interrupt pending */
		/* Call interrupt handler for any pending interrupts */

	    if(nValue & IPA_INT0_PENDING || nValue & IPA_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_A_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_A_MEM_OFFSET];/* this slots mem address */

			/* Slot A ISR code goes in here */
			isr_408( (void *)&hdata );

		   /* read IP A Interrupt Select Space */
		   dummyWord = readw((unsigned short*)&pPCICard->slotAInt0);
		   dummyWord = readw((unsigned short*)&pPCICard->slotAInt1);
		   int_status = 1;
		}			

	    if(nValue & IPB_INT0_PENDING || nValue & IPB_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_B_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_B_MEM_OFFSET];/* this slots mem address */

			/* Slot B ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP B Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotBInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotBInt1);
			int_status = 1;
		}			

		if(nValue & IPC_INT0_PENDING || nValue & IPC_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_C_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_C_MEM_OFFSET];/* this slots mem address */

			/* Slot C ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP C Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotCInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotCInt1);
			int_status = 1;
		}			

		if(nValue & IPD_INT0_PENDING || nValue & IPD_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_D_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_D_MEM_OFFSET];/* this slots mem address */

			/* Slot D ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP D Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotDInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotDInt1);
			int_status = 1;
		}			

		if(nValue & IPE_INT0_PENDING || nValue & IPE_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_E_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_E_MEM_OFFSET];/* this slots mem address */

			/* Slot E ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP E Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotEInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotEInt1);
			int_status = 1;
		}
	 }
   }



   if( open_dev[1] )		/* Check the 2nd board open_dev[1] */
   {
     pPCICard = (PCI_BOARD_MEMORY_MAP*)carrier_address[1];
     nValue = readw((unsigned short*)&pPCICard->intPending);

	 if( nValue & CARRIER_INT_MASK )/* non-zero if this carrier is interrupting */
	 {
/*
printk("\nopen_dev[1] pending %X",nValue);
*/
		/* Check each IP slot for an interrupt pending */
		/* Call interrupt handler for any pending interrupts */

	    if(nValue & IPA_INT0_PENDING || nValue & IPA_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_A_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_A_MEM_OFFSET];/* this slots mem address */

			/* Slot A ISR code goes in here */
		   isr_408( (void *)&hdata );

		   /* read IP A Interrupt Select Space */
		   dummyWord = readw((unsigned short*)&pPCICard->slotAInt0);
		   dummyWord = readw((unsigned short*)&pPCICard->slotAInt1);
		   int_status = 1;
		}			

	    if(nValue & IPB_INT0_PENDING || nValue & IPB_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_B_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_B_MEM_OFFSET];/* this slots mem address */

			/* Slot B ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP B Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotBInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotBInt1);
			int_status = 1;
		}			

		if(nValue & IPC_INT0_PENDING || nValue & IPC_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_C_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_C_MEM_OFFSET];/* this slots mem address */

			/* Slot C ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP C Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotCInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotCInt1);
			int_status = 1;
		}			

		if(nValue & IPD_INT0_PENDING || nValue & IPD_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_D_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_D_MEM_OFFSET];/* this slots mem address */

			/* Slot D ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP D Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotDInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotDInt1);
			int_status = 1;
		}			

		if(nValue & IPE_INT0_PENDING || nValue & IPE_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_E_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_E_MEM_OFFSET];/* this slots mem address */

			/* Slot E ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP E Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotEInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotEInt1);
			int_status = 1;
		}
	 }
   }




   if( open_dev[2] )		/* Check the 3rd board open_dev[2] */
   {
     pPCICard = (PCI_BOARD_MEMORY_MAP*)carrier_address[2];
     nValue = readw((unsigned short*)&pPCICard->intPending);

	 if( nValue & CARRIER_INT_MASK )/* non-zero if this carrier is interrupting */
	 {
/*
printk("\nopen_dev[2] pending %X",nValue);
*/
		/* Check each IP slot for an interrupt pending */
		/* Call interrupt handler for any pending interrupts */

	    if(nValue & IPA_INT0_PENDING || nValue & IPA_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_A_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_A_MEM_OFFSET];/* this slots mem address */

			/* Slot A ISR code goes in here */
		   isr_408( (void *)&hdata );

		   /* read IP A Interrupt Select Space */
		   dummyWord = readw((unsigned short*)&pPCICard->slotAInt0);
		   dummyWord = readw((unsigned short*)&pPCICard->slotAInt1);
		   int_status = 1;
		}			

	    if(nValue & IPB_INT0_PENDING || nValue & IPB_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_B_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_B_MEM_OFFSET];/* this slots mem address */

			/* Slot B ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP B Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotBInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotBInt1);
			int_status = 1;
		}			

		if(nValue & IPC_INT0_PENDING || nValue & IPC_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_C_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_C_MEM_OFFSET];/* this slots mem address */

			/* Slot C ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP C Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotCInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotCInt1);
			int_status = 1;
		}			

		if(nValue & IPD_INT0_PENDING || nValue & IPD_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_D_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_D_MEM_OFFSET];/* this slots mem address */

			/* Slot D ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP D Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotDInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotDInt1);
			int_status = 1;
		}			

		if(nValue & IPE_INT0_PENDING || nValue & IPE_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_E_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_E_MEM_OFFSET];/* this slots mem address */

			/* Slot E ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP E Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotEInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotEInt1);
			int_status = 1;
		}
	 }
   }




   if( open_dev[3] )		/* Check the 4th board open_dev[3] */
   {
     pPCICard = (PCI_BOARD_MEMORY_MAP*)carrier_address[3];
     nValue = readw((unsigned short*)&pPCICard->intPending);

	 if( nValue & CARRIER_INT_MASK )/* non-zero if this carrier is interrupting */
	 {
/*
printk("\nopen_dev[3] pending %X",nValue);
*/
		/* Check each IP slot for an interrupt pending */
		/* Call interrupt handler for any pending interrupts */

	    if(nValue & IPA_INT0_PENDING || nValue & IPA_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_A_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_A_MEM_OFFSET];/* this slots mem address */

			/* Slot A ISR code goes in here */
		   isr_408( (void *)&hdata );

		   /* read IP A Interrupt Select Space */
		   dummyWord = readw((unsigned short*)&pPCICard->slotAInt0);
		   dummyWord = readw((unsigned short*)&pPCICard->slotAInt1);
		   int_status = 1;
		}			

	    if(nValue & IPB_INT0_PENDING || nValue & IPB_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_B_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_B_MEM_OFFSET];/* this slots mem address */

			/* Slot B ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP B Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotBInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotBInt1);
			int_status = 1;
		}			

		if(nValue & IPC_INT0_PENDING || nValue & IPC_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_C_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_C_MEM_OFFSET];/* this slots mem address */

			/* Slot C ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP C Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotCInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotCInt1);
			int_status = 1;
		}			

		if(nValue & IPD_INT0_PENDING || nValue & IPD_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_D_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_D_MEM_OFFSET];/* this slots mem address */

			/* Slot D ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP D Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotDInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotDInt1);
			int_status = 1;
		}			

		if(nValue & IPE_INT0_PENDING || nValue & IPE_INT1_PENDING)
		{
			/* This slot is pending... load up the interrupt data structure and call the ISR for this slot*/
			idata.slot_io_address = (unsigned long)&((char *)pPCICard)[SLOT_E_IO_OFFSET];/* this slots I/O address */
			idata.slot_mem_address = (unsigned long)&((char *)pPCICard)[SLOT_E_MEM_OFFSET];/* this slots mem address */

			/* Slot E ISR code goes in here */
			/*     isr_xxx( (void *)&hdata );*/
			/* read IP E Interrupt Select Space */
			dummyWord = readw((unsigned short*)&pPCICard->slotEInt0);
			dummyWord = readw((unsigned short*)&pPCICard->slotEInt1);
			int_status = 1;
		}
	 }
   }

   if( int_status )
       return( IRQ_HANDLED);
   else
       return( IRQ_NONE);
}



int
init_module( void ) 
{ 
  struct pci_dev *p8620;
  int i,j;
  char devnamebuf[32];
  char devnumbuf[8];

  memset( &open_dev[0], 0, sizeof(open_dev));
  memset( &board_irq[0], 0, sizeof(board_irq));
  memset( &ip_mem_address[0], 0, sizeof(ip_mem_address));
  memset( &carrier_address[0], 0, sizeof(carrier_address));

  p8620 = NULL;
  for( i = 0, j = 0; i < MAX_CARRIERS; i++ )
  {
/*  Use pci_find_device() for earlier versions FC3/4/5/6 */
/*  p8620 = ( struct pci_dev *)pci_find_device( 0x10B5, 0x1024, p8620 ); */
    p8620 = ( struct pci_dev *)pci_get_device( 0x10B5, 0x1024, p8620 );
    if( p8620 )
	{ 
	  carrier_address[i] = (unsigned long)p8620->resource[2].start;
      carrier_address[i]= (unsigned long)ioremap_nocache( carrier_address[i], 4096); /* no cache */

      if( carrier_address[i] )
	  {
        memset( &devnamebuf[0], 0, sizeof(devnamebuf));
        memset( &devnumbuf[0], 0, sizeof(devnumbuf));
	strcpy(devnamebuf, DEVICE_NAME);
        sprintf(&devnumbuf[0],"%d",i);
	strcat(devnamebuf, devnumbuf);
        board_irq[i] = p8620->irq;
/*  Used for earlier versions FC3/4/5/6 */
/*	ret_val = request_irq ( board_irq[i], apc8620_handler, IRQF_DISABLED | IRQF_SHARED, devnamebuf, ( void *)carrier_address[i] ); */
	ret_val = request_irq ( board_irq[i], (irq_handler_t)apc8620_handler, IRQF_DISABLED | IRQF_SHARED, devnamebuf, ( void *)carrier_address[i] );

        printk("%s mapped   I/O=%08lX IRQ=%02X Rv=%X\n",devnamebuf,(unsigned long)carrier_address[i], board_irq[i],ret_val);

		/* 8620a may have an additional BAR register if it supports IP memory */
		/* if the additional region is present map it into memory */
        ip_mem_address[i] = (unsigned long)p8620->resource[3].start;	/* get IP mem region if present */
        if( ip_mem_address[i] )
		{
		    ip_mem_address[i] = (unsigned long)ioremap_nocache( ip_mem_address[i], 0x4000000); /* no cache */

          if( ip_mem_address[i] )
              printk("%s mapped   MEM=%08lX\n",devnamebuf, (unsigned long)ip_mem_address[i]);

		}
        j++;
	  }
	}
	else
		break;
  }
  if( j )	/* found at least one device */
  {
	ret_val = register_chrdev ( MAJOR_NUM, DEVICE_NAME, &apc8620_ops );

	if( ret_val < 0)
	{
		printk(DEVICE_NAME);
	    printk(" Failed to register error = %d\n", ret_val);
	}
	else
		return( 0 );
  }
  return( -ENODEV );
}


void
cleanup_module( void ) 
{
  char devnamebuf[32];
  char devnumbuf[8];
  int i;


  if( ret_val >= 0 )
  {
    unregister_chrdev( MAJOR_NUM, DEVICE_NAME );
    for( i = 0; i < MAX_CARRIERS; i++ )
	{
      if( carrier_address[i] )
	  {
        memset( &devnamebuf[0], 0, sizeof(devnamebuf));
        memset( &devnumbuf[0], 0, sizeof(devnumbuf));
        strcpy(devnamebuf, DEVICE_NAME);
        sprintf(&devnumbuf[0],"%d",i);
        strcat(devnamebuf, devnumbuf);

        free_irq( board_irq[i], (void *)carrier_address[i] );
        iounmap( (void *)carrier_address[i] );
        printk("%s unmapped I/O=%08lX IRQ=%02X\n",devnamebuf,(unsigned long)carrier_address[i], board_irq[i]);

   	    if( ip_mem_address[i] )
		{
	       iounmap( (void *)ip_mem_address[i] );
           printk("%s unmapped MEM=%08lX\n",devnamebuf,(unsigned long)ip_mem_address[i]);
		}
	  }
	}
  }
}

MODULE_LICENSE("GPL");


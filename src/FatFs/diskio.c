/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "ff.h"         /* Obtains integer types */
#include "diskio.h"     /* Declarations of disk functions */
#include "sdcard_new.h"


static void RoughDelay(uint32_t t)
{
    volatile int32_t delay;

    delay = t;

    while (delay-- >= 0);
}


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
    BYTE pdrv       /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS sta1 = RES_OK;

    if (pdrv)
        sta1 = STA_NOINIT;

    return sta1;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
    BYTE pdrv               /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS sta = RES_OK;
    uint32_t timeout = 0;

    /* Card detect pin is PC.14. */
    GPIO_SetMode(PD, BIT13, GPIO_PMD_INPUT);

    printf("Check SD Card insert ?\n");

    /* Waiting for card is inserted */
    while (PD13 == 1)
    {
        if (timeout++ > 0x200)
            break;
    }

    if (PD13 != 1)
    {
        printf("Card Detected!!\n");

        /* Power pin is PC.15. */
        GPIO_SetMode(PD, BIT12, GPIO_PMD_OUTPUT);
        PD12 = 0;

        RoughDelay(100000);

        /* Open SD card */
        if (SDCARD_Open() == SD_SUCCESS)
        {
            sta =   RES_OK;
            printf("SDCard Open success\n");
        }
        else
        {
            sta = STA_NOINIT;
            printf("SDCard Open failed\n");
        }
    }
    else
    {
        printf("Can't detect card !!\n");
        sta = STA_NOINIT;
    }

    return sta;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Start sector in LBA */
    UINT count      /* Number of sectors to read */
)
{
    DRESULT res;
    uint32_t size;

    if (pdrv)
    {
        res = (DRESULT)STA_NOINIT;
        return res;
    }

    if (count == 0 || count > 2)
    {
        res = (DRESULT)STA_NOINIT;
        return res;
    }

    size = count * 512;
    /* Read data from SD card */
    SpiRead(sector, size, buff);

    res = RES_OK;   /* Clear STA_NOINIT */;

    return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    DRESULT  res;
    uint32_t size;

    if (pdrv)
    {
        res = (DRESULT)STA_NOINIT;
        return res;
    }

    if (count == 0 || count > 2)
    {
        res = (DRESULT)  STA_NOINIT;
        return res;
    }

    size = count * 512;

    /* Write data into SD card */
    SpiWrite(sector, size, (uint8_t *)buff);

    res = RES_OK;

    return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    DRESULT res;

    if (pdrv) return RES_PARERR;

    switch (cmd)
    {
        case CTRL_SYNC :        /* Make sure that no pending write process */
            res = RES_OK;
            break;

        case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
            SDCARD_GetCardSize(buff);
            res = RES_OK;
            break;

        case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
            *(DWORD *)buff = 512;   //512;
            res = RES_OK;
            break;

        case GET_BLOCK_SIZE :   /* Get erase block size in unit of sector (DWORD) */
            *(DWORD *)buff = 1;
            res = RES_OK;
            break;


        default:
            res = RES_PARERR;
    }


    res = RES_OK;


    return res;
}


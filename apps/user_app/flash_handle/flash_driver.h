#ifndef __FLASH_DRIVER_H__
#define __FLASH_DRIVER_H__

// #include "typedef.h"
#include "includes.h"
 
int flash_area_read_4K(u8* buf, u32 read_size);
int flash_area_write_4K(u8* buf, u32 size);
  
#endif 

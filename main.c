/*
* Platform  for: s3c2440
* author  by jiangdou
* you  have anything,plese to QQ:344283973
* time  at: 2009-0801
*
*/




//#include <stdarg.h>    /* va_list */
//#include <boot_zImage.c>
extern void uart0_init(void);
extern void nand_read(unsigned int nand_start, unsigned int ddr_start, unsigned int len);
extern void putc(char c);
extern void puts(char *str);
extern void puthex(unsigned int val);
extern unsigned char getc(void);
//extern int boot_zImage(long from, size_t size);
extern int getc_nowait(unsigned char *pChar);
extern void putbyte(unsigned char val);
extern void nand_erase_block(unsigned long addr);
//extern void nand_read(unsigned char *buf,unsigned long star_addr, unsigned int len);
static int ctrlc_was_pressed = 0;

extern const unsigned char logo;


char * strcpy(char * dest,const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}
int strlen(char *str)	//0x33CBD6D8
{
	int i = 0;
	while (str[i])
	{
		i++;
	}
	return i;
}
void nand_write_test(void)
{
  char buf[20] = {"abcd1234ABCD"};
  unsigned long addr;
  unsigned long size;
  
  puts("enter the start address:0x80000 ");
  //scanf("%s", buf);
  //addr = strtoul(buf, NULL, 0);
  addr = 0x80000;
  puts("enter the string:abcd1234ABCD ");
  //scanf("%s", buf);
  size = strlen(buf) + 1;
  puts(" size= ");
  puthex(size);
  puts("\n\r");
  nand_write(addr, buf, size);
  
}

void nand_read_test(void)

{
  int i;
  char buf[100];
  unsigned long addr;
  unsigned long size;
 
  puts("enter the start address: 0x80000");

  //scanf("%s", buf);

  //addr = strtoul(buf, NULL, 0);
  addr = 0x80000;
  //puts("read addr = 0x%x\n\r", addr);

  puts("enter the size: 0x60");

  //scanf("%s", buf);
  //size = strtoul(buf, NULL, 0);
  size = 0x60;

  if (size > 100)
    {
      puts("the max size is 100\n\r");
      size = 100;
    }
  nand_read(addr, buf, size);

  puts("datas: \n\r");
  for (i = 0; i < size; i++)
    {
      // printf("%02x ", buf[i]);
      putbyte(buf[i]);
      puts("\t");
      if ((i+1) % 8 == 0)
	{
	  puts("\n\r");
	}
    }
  puts("\n\r");
}

void nand_erase_test(void)
{
  //char buf[100];
  unsigned long addr;
  
  puts("enter the start address: ");
  //scanf("%s", buf);
  //addr = strtoul(buf, NULL, 0);
  addr = 0x80000;
  puts("erase addr = ");
  puthex(addr);
  puts("\n\r");
  nand_erase_block(addr);
  
}

void update_program(void)
{
  unsigned char *buf = (unsigned char *)0x30000000;
  //unsigned char *buf = (unsigned char *)0xD0036000;
  unsigned long len = 0;
  int have_begin = 0;
  int nodata_time = 0;
  unsigned long erase_addr;
  char c;
  int i;

  /* 读串口获得数�?*/
  puts("\n\ruse V2.2.exe/gtkterm to send file\n\r");
  while (1)
    {
      if (getc_nowait(&buf[len]) == 0)
	{
	  have_begin = 1;
	  nodata_time = 0;
	  len++;
	}
      else
	{
	  if (have_begin)
	    {
	      nodata_time++;
	    }
	}

      if (nodata_time == 1000)
	{
	  break;
	}
    }
  puts("\n\rhave get data:");
  puthex(len);
  puts(" bytes\n\r");
  puts("the first 16 bytes data: \n\r");
  for (i = 0; i < 16; i++)
    {
      // put("%02x ", buf[i]);
      putbyte(buf[i]);
      puts("\t");
    }
  puts("\n\r");

  puts("Press Y to program the flash: \n\r");

  c = getc();
  putc(c);
  puts("\n\r");
  if (c == 'y' || c == 'Y')
    {
      /* 烧写到nand flash block 0 */
      for (erase_addr = 0; erase_addr < ((len + 0x1FFFF) & ~0x1FFFF); erase_addr += 0x20000)
	{
	  nand_erase_block(erase_addr);
	}
      nand_write(0, buf, len);
      
      puts("update program successful\n\r");
    }
  else
    {
      puts("Cancel program!\n\r");
    }
}

void run_program(void)
{
  unsigned char *buf = (unsigned char *)0x30000000;
  //unsigned char *buf = (unsigned char *)0xD0036000;
  unsigned long len = 0;
  int have_begin = 0;
  int nodata_time = 0;
  void (*theProgram)(void);
  int i;
  /* 读串口获得数�?*/
  puts("\n\r use gtkterm to send file\n\r");
  while (1)
    {
      if (getc_nowait(&buf[len]) == 0)
	{
	  have_begin = 1;
	  nodata_time = 0;
	  len++;
	}
      else
	{
	  if (have_begin)
	    {
	      nodata_time++;
	    }
	}

      if (nodata_time == 10000)
	{
	  break;
	}
    }
  //printf("have get %d bytes data\n\r", len);
  puts("\n\r have get data:");
  puthex(len);
  puts(" bytes\n\r");
  puts("the first 16 bytes data: \n\r");
  for (i = 0; i < 16; i++)
    {
      // put("%02x ", buf[i]);                                                                     
      putbyte(buf[i]);
      puts("\t");
      //putc('\0');
    }
  puts("\n\r");
  puts("jump to 0x32000000 to run it\n\r");
  
  theProgram = (void (*)(void))0x32000000;

  theProgram();
}


int main(void)
{
  char c;
  char cmd_buf[200];
  uart0_init();

  puts("\n\r========================================\n\r");
  puts("The board:TQ2440\n\r");
  puts("The NAND:K9F1216U0A 256MB\n\r");
  puts("The NOR:EN29LV160AB 2MB\n\r");
  puts("The SRAM:HY57V561620 x2 64MB\n\r");
  puts("just bootloader linux        date: 2009.8.19;by jiangdou\n\r");
  puts("===========================================\n\r");


  lcd_Init();
  my_logo();
  PutPixel(20,250,0x001F);
  PutPixel(40,250,0x07E0);
  PutPixel(40,250,0xF800);
  //paint_Bmp(0,0,320,240,logo);
  //lcd_ClearScr(0x001F);
  //paint_Bmp((LCD_XSIZE_TFT -80) / 2, (LCD_YSIZE_TFT -80) / 2, 80, 80, logo);
  puts("Booting Linux ...\n\r");
  boot_zImage(0x00200000, 0x00300000);

  while (1)
    {
      puts("the menu of the update programe:\n\r");
      puts("[w] write the nand flash\n\r");
      puts("[r] read the nand flash\n\r");
      puts("[e] erase the nand flash\n\r");
      puts("[g] get file, and write to nand flash 0 block\n\r");
      puts("[x] get file to ddr(0x32000000), run it\n\r");
      puts("[s] reset the programe\n\r");
      puts("[k] reset the programe\n\r");
      puts("Please enter the chose:\n\r");

      do {
	c = getc();
	if (c == '\n' || c == '\r')
	  {
	    puts("\n\r");
	  }
	else
	  {
	    putc(c);
	  }
      } while (c == '\n' || c == '\r');
      
      switch (c)
	{
	case 'w':
	case 'W':
	  {
	    nand_write_test();
	    break;
	  }

	case 'r':
	case 'R':
	  {
	    nand_read_test();
	    break;
	  }

	case 'e':
	case 'E':
	  {
	    nand_erase_test();
	    break;
	  }

	case 'g':
	case 'G':
	  {
	    update_program();
	    break;
	  }

	case 'x':
	case 'X':
	  {
	    run_program();
	    break;
	  }

	case 's':
	case 'S':
	  {
	    void (*theProgram)(void);
	    theProgram = (void (*)(void))0x33f80000;
	    theProgram();
	    break;
	  }
	
	
	}
    }

  return 0;
}


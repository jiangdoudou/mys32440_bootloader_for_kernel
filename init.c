/*
* Platform  for: s3c2440
* author  by jiangdou
* you  have anything,plese to QQ:344283973
* time  at: 2009-0801
*
*/



typedef unsigned int u32;
typedef volatile u32    S3C24X0_REG32;

//extern ulong _bss_start;	/* code + data end == BSS start */
//extern ulong _bss_end;		/* BSS end */

static inline void delay1(unsigned long loops)
{
	__asm__ volatile ("1:\n"
			"subs %0, %1, #1\n"
			"bne 1b":"=r" (loops):"0" (loops));
}

typedef struct {
    S3C24X0_REG32   LOCKTIME;
    S3C24X0_REG32   MPLLCON;
    S3C24X0_REG32   UPLLCON;
    S3C24X0_REG32   CLKCON;
    S3C24X0_REG32   CLKSLOW;
    S3C24X0_REG32   CLKDIVN;
    S3C24X0_REG32   CAMDIVN;    /* for s3c2440, by www.embedsky.net */
} /*__attribute__((__packed__))*/ S3C24X0_CLOCK_POWER;

/* NAND FLASH控制器 */
#define NFCONF (*((volatile unsigned long *)0x4E000000))
#define NFCONT (*((volatile unsigned long *)0x4E000004))
#define NFCMMD (*((volatile unsigned char *)0x4E000008))
#define NFADDR (*((volatile unsigned char *)0x4E00000C))
#define NFDATA (*((volatile unsigned char *)0x4E000010))
#define NFSTAT (*((volatile unsigned char *)0x4E000020))

/* GPIO */
#define GPHCON              (*(volatile unsigned long *)0x56000070)
#define GPHUP               (*(volatile unsigned long *)0x56000078)

/* UART registers*/
#define ULCON0              (*(volatile unsigned long *)0x50000000)
#define UCON0               (*(volatile unsigned long *)0x50000004)
#define UFCON0              (*(volatile unsigned long *)0x50000008)
#define UMCON0              (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)
#define UFSTAT0             (*(volatile unsigned long *)0x50000018)
#define UTXH0               (*(volatile unsigned char *)0x50000020)
#define URXH0               (*(volatile unsigned char *)0x50000024)
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)

#define TXD0READY   (1<<2)

void nand_read(unsigned char *buf,unsigned long star_addr, unsigned int len);
int isBootFromNorFlash(void)
{
  volatile int *p = (volatile int *)0;
  int val;
  
  val = *p;
  *p = 0x12345678;
  if (*p == 0x12345678)
    {
      /*写成功，是nand启动*/
      *p = val;
      return 0;
    }
  else
    {
      /*Nor不能像内存一样写*/
      return 1;
}
}

void copy_code_to_sdram(unsigned long start_addr, unsigned char *buf, unsigned int len)
{
  int i = 0;
  unsigned char *src_start = (unsigned char *)start_addr;
  unsigned char *dest_start = buf;
  /*如果是Nor启动*/
  if(isBootFromNorFlash())
    {
      while (i < len)
	{
	  //dest[i] = src[i];
	  dest_start[i] = src_start[i];
	  i++;
	}  
    }
  else
    {
      //nand_init();
      //nand_resd(src, dest, len)
      nand_read(buf,start_addr,len);
    }
}


void clear_bss(void)
{
       extern int __bss_start, __bss_end;
	int *p = &__bss_start;
	
	for (; p < &__bss_end; p++)
		*p = 0;
}

void nand_select(void)
{
  NFCONT &= ~(1<<1);	
}

void nand_deselect(void)
{
  NFCONT |= (1<<1);	
}

void nand_cmd(unsigned char cmd)
{
  volatile int i;
  NFCMMD = cmd;
  for (i = 0; i < 10; i++);
}

void nand_addr(unsigned int addr)
{
	unsigned int col  = addr % 2048;
	unsigned int page = addr / 2048;
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	
	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);	
}

void wait_ready(void)
{
	while (!(NFSTAT & 1));
}

unsigned char nand_get_data(void)
{
	return NFDATA;
}

void nand_send_data(unsigned char data)
{
  	NFDATA = data;
}

void nand_reset(void)
{
  /* 选中 */
  nand_select();

  /* 发出0xff命令 */
  nand_cmd(0xff);

  /* 等待就绪 */
  wait_ready();

  /* 取消选中 */
  nand_deselect();
}
//nand_read(unsigned int nand_start, unsigned int ddr_start, unsigned int len)
void nand_read(unsigned char *buf,unsigned long star_addr, unsigned int len)
{
	unsigned int addr = star_addr;
	int col = addr % 2048;
	int i = 0;
	unsigned char *dest = buf;
	
	/* 1. 选中 */
	nand_select();
	while (i < len)
	{
		/* 2. 发出读命令00h */
		nand_cmd(0x00);

		/* 3. 发出地址(分5步发出) */
		nand_addr(addr);

		/* 4. 发出读命令30h */
		nand_cmd(0x30);

		/* 5. 判断状态 */
		wait_ready();

		/* 6. 读数据 */
		for (; (col < 2048) && (i < len); col++)
		{
			dest[i] = nand_get_data();
			i++;
			addr++;
		}
		
		col = 0;
	}

	/* 7. 取消选中 */		
	nand_deselect();
}

void nand_erase_block(unsigned long addr)
{
  int page = addr / 2048;
  
  nand_select();
  nand_cmd(0x60);
  
  nand_addr(page & 0xff);
  nand_addr((page >> 8) & 0xff);
  nand_addr((page >> 16) & 0xff);

  nand_cmd(0xd0);
  wait_ready();

  nand_deselect();
}


void nand_write(unsigned int nand_start, unsigned char * buf, unsigned int len)
{
  unsigned long count = 0;
  unsigned long addr  = nand_start;
  int i = nand_start % 2048;
  //int left = i;
  
  nand_select();
  while (count < len)
    {
      nand_cmd(0x80);
      nand_addr(addr);
      //for (; i < (2048-left) && count < len; i++)
      for (; i < 2048 && count < len; i++)
	{
	  /*if(addr<16384)//写前2K
	    {
	      if(i<(2048-left))//前2页每页只能写2K
		{
		  nand_send_data(buf[count++]);
		}
	    }
	  else
	    {
	      nand_send_data(buf[count++]);
	      }*/
	  nand_send_data(buf[count++]);
	  addr++;
	}

      nand_cmd(0x10);
      wait_ready();
      i = 0;
      //left = i;
    }

  nand_deselect();
  
}
void nand_init(void)
{
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
  /* 设置时序 */
  NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
  /* 使能NAND Flash控制器, 初始化ECC, 禁止片选 */
  NFCONT = (1<<4)|(1<<1)|(1<<0);

  nand_reset();	
}

#define PCLK            50000000    // init.c中的clock_init函数设置PCLK为50MHz
#define UART_CLK        PCLK        //  UART0的时钟源设为PCLK
#define UART_BAUD_RATE  115200      // 波特率
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)

#define ENABLE_FIFO 

static void delay(void)
{
  volatile int i = 10;
  while (i--);
}
/*
 * 初始化UART0
 * 115200,8N1,无流控
 */
void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3用作TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3内部上拉

    ULCON0  = 0x03;     // 8N1(8个数据位，无较验，1个停止位)
    UCON0   = 0x05;     // 查询方式，UART时钟源为PCLK
    #ifdef ENABLE_FIFO
	UFCON0 = 0x07; /* FIFO enable */
    #else
	UFCON0 = 0x00; /* FIFO disable */
    #endif
    UMCON0  = 0x00;     // 不使用流控
    UBRDIV0 = UART_BRD; // 波特率为115200
}

/*
 * 发送一个字符
 */
void putc(unsigned char c)
{
    /* 等待，直到发送缓冲区中的数据已经全部发送出去 */
    //while (!(UTRSTAT0 & TXD0READY));
    #ifdef ENABLE_FIFO
    	while (UFSTAT0 & (1<<14))delay();
    #else
    	while ((UTRSTAT0 & (1<<2)) == 0);
#endif   
    /* 向UTXH0寄存器中写入数据，UART即自动将它发送出去 */
    UTXH0 = c;
}
/*
 * 接收字符
 */
unsigned char getc(void)
{
    /* 等待，直到接收缓冲区中的有数据 */
    //while (!(UTRSTAT0 & RXD0READY));
    #ifdef ENABLE_FIFO
    	while ((UFSTAT0 & (1<<6)) == 0 && (UFSTAT0 & 0x3f) == 0)delay();
    #else
    	while ((UTRSTAT0 & (1<<0)) == 0);
#endif   
    /* 直接读取URXH0寄存器，即可获得接收到的数据 */
    return URXH0;
}

int getc_nowait(unsigned char *pChar)
{
#ifdef ENABLE_FIFO
  if ((UFSTAT0 & (1<<6)) == 0 && (UFSTAT0 & 0x3f) == 0)
#else
    if ((UTRSTAT0 & (1<<0)) == 0)
#endif
      {
	return -1;
      }
    else
      {
	*pChar = URXH0;
	return 0;
      }
}

void puts(char *str)
{
	int i = 0;
	while (str[i])
	{
		putc(str[i]);
		i++;
	}
}

void puthex(unsigned int val)
{
  /* 0x1234abcd */
  int i;
  int j;

  puts("0x");

  for (i = 0; i < 8; i++)
    {
      j = (val >> ((7-i)*4)) & 0xf;
      if ((j >= 0) && (j <= 9))
	putc('0' + j);
      else
	putc('A' + j - 0xa);

    }

}
void putbyte(unsigned char val)
{
  /* 0x1234abcd */
  int i;
  int j;

  puts("0x");

  for (i = 0; i < 2; i++)
    {
      j = (val >> ((1-i)*4)) & 0xf;
      if ((j >= 0) && (j <= 9))
	putc('0' + j);
      else
	putc('A' + j - 0xa);

    }

}


#define S3C2440_MPLL_400MHZ	((0x5c<<12)|(0x01<<4)|(0x01))						//HJ 400MHz
//#define S3C2440_MPLL_405MHZ	((0x7f<<12)|(0x02<<4)|(0x01))						//HJ 405MHz
//#define S3C2440_MPLL_440MHZ	((0x66<<12)|(0x01<<4)|(0x01))						//HJ 440MHz
//#define S3C2440_MPLL_480MHZ	((0x98<<12)|(0x02<<4)|(0x01))						//HJ 480MHz
//#define S3C2440_MPLL_200MHZ	((0x5c<<12)|(0x01<<4)|(0x02))
//#define S3C2440_MPLL_100MHZ	((0x5c<<12)|(0x01<<4)|(0x03))

#define S3C2440_UPLL_48MHZ	((0x38<<12)|(0x02<<4)|(0x02))						//HJ 100MHz

#define S3C2440_CLKDIV		0x05    /* FCLK:HCLK:PCLK = 1:4:8, UCLK = UPLL */		//HJ 100MHz
#define S3C2440_CLKDIV136	0x07    /* FCLK:HCLK:PCLK = 1:3:6, UCLK = UPLL */		//HJ 133MHz
#define S3C2440_CLKDIV188	0x04    /* FCLK:HCLK:PCLK = 1:8:8 */
#define S3C2440_CAMDIVN188	((0<<8)|(1<<9)) /* FCLK:HCLK:PCLK = 1:8:8 */

/* Fin = 16.9344MHz */
#define S3C2440_MPLL_399MHz     		((0x6e<<12)|(0x03<<4)|(0x01))
#define S3C2440_UPLL_48MHZ_Fin16MHz		((60<<12)|(4<<4)|(2))





void clock_init(void)
{
	S3C24X0_CLOCK_POWER *clk_power = (S3C24X0_CLOCK_POWER *)0x4C000000;

	/* FCLK:HCLK:PCLK = ?:?:? */

	clk_power->CLKDIVN = 0x05;				//HJ 1:4:8

	/* change to asynchronous bus mod */
	__asm__(    "mrc    p15, 0, r1, c1, c0, 0\n"    /* read ctrl register   */  
                    "orr    r1, r1, #0xc0000000\n"      /* Asynchronous         */  
                    "mcr    p15, 0, r1, c1, c0, 0\n"    /* write ctrl register  */  
                    :::"r1"
                    );

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	clk_power->LOCKTIME = 0xFFFFFF;

	/* configure UPLL */
	clk_power->UPLLCON = S3C2440_UPLL_48MHZ;		//fin=12.000MHz


	/* some delay between MPLL and UPLL */
	delay1(4000);

	/* configure MPLL */
	clk_power->MPLLCON = S3C2440_MPLL_400MHZ;		//fin=12.000MHz

	delay1(8000);
}





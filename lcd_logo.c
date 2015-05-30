/*
* Platform  for: s3c2440
* author  by jiangdou
* you  have anything,plese to QQ:344283973
* time  at: 2009-0801
*
*/

//#include "bmp.h"

#define M5D(n)			( ( n ) & 0x1fffff )	
#define S3C24X0_LCD_BASE        0x4D000000
#define S3C24X0_GPIO_BASE       0x56000000

#define LCD_XSIZE_TFT	 	(480)	
#define LCD_YSIZE_TFT	 	(272)
#define	VBPD		2
#define	VFPD		4
#define	VSPW		8
#define	HBPD		10
#define	HFPD		19
#define	HSPW		30
#define	CLKVAL_TFT		4
//Timing parameter for LCD
#define MVAL			(13)
#define MVAL_USED 		(0)						//0=each frame   1=rate by MVAL
#define INVVDEN			(1)						//0=normal       1=inverted
#define BSWP			(0)						//Byte swap control
#define HWSWP			(1)	


#define SCR_XSIZE_TFT	 	(LCD_XSIZE_TFT)
#define SCR_YSIZE_TFT	 	(LCD_YSIZE_TFT)

#define HOZVAL_TFT		( LCD_XSIZE_TFT - 1 )
#define LINEVAL_TFT		( LCD_YSIZE_TFT - 1 )

volatile unsigned short LCD_BUFFER[SCR_YSIZE_TFT][SCR_XSIZE_TFT] ;//= { 0, } ;

//volatile char vbpd = 1, vfpd = 1, vspw = 1, hbpd = 1, hfpd = 1, hspw = 1, clkval_tft = 1 ;


typedef struct {
    unsigned int   GPACON;
    unsigned int   GPADAT;
    unsigned int   res1[2];
    unsigned int   GPBCON;
    unsigned int   GPBDAT;
    unsigned int   GPBUP;
    unsigned int   res2;
    unsigned int   GPCCON;
    unsigned int   GPCDAT;
    unsigned int   GPCUP;
    unsigned int   res3;
    unsigned int   GPDCON;
    unsigned int   GPDDAT;
    unsigned int   GPDUP;
    unsigned int   res4;
    unsigned int   GPECON;
    unsigned int   GPEDAT;
    unsigned int   GPEUP;
    unsigned int   res5;
    unsigned int   GPFCON;
    unsigned int   GPFDAT;
    unsigned int   GPFUP;
    unsigned int   res6;
    unsigned int   GPGCON;
    unsigned int   GPGDAT;
    unsigned int   GPGUP;
    unsigned int   res7;
    unsigned int   GPHCON;
    unsigned int   GPHDAT;
    unsigned int   GPHUP;
    unsigned int   res8;

    unsigned int   MISCCR;
    unsigned int   DCLKCON;
    unsigned int   EXTINT0;
    unsigned int   EXTINT1;
    unsigned int   EXTINT2;
    unsigned int   EINTFLT0;
    unsigned int   EINTFLT1;
    unsigned int   EINTFLT2;
    unsigned int   EINTFLT3;
    unsigned int   EINTMASK;
    unsigned int   EINTPEND;
    unsigned int   GSTATUS0;
    unsigned int   GSTATUS1;
    unsigned int   GSTATUS2;
    unsigned int   GSTATUS3;
    unsigned int   GSTATUS4;

    /* s3c2440 */
    unsigned int   res9[4];
    unsigned int   GPJCON;
    unsigned int   GPJDAT;
    unsigned int   GPJUP;

} /*__attribute__((__packed__))*/ S3C24X0_GPIO;

typedef struct {
    unsigned int   LCDCON1;
    unsigned int   LCDCON2;
    unsigned int   LCDCON3;
    unsigned int   LCDCON4;
    unsigned int   LCDCON5;
    unsigned int   LCDSADDR1;
    unsigned int   LCDSADDR2;
    unsigned int   LCDSADDR3;
    unsigned int   REDLUT;
    unsigned int   GREENLUT;
    unsigned int   BLUELUT;
    unsigned int   res[8];
    unsigned int   DITHMODE;
    unsigned int   TPAL;
    unsigned int   LCDINTPND;
    unsigned int   LCDSRCPND;
    unsigned int   LCDINTMSK;
    unsigned int   LPCSEL;

} /*__attribute__((__packed__))*/ S3C24X0_LCD;

static inline S3C24X0_GPIO * const S3C24X0_GetBase_GPIO(void)
{
    return (S3C24X0_GPIO * const)S3C24X0_GPIO_BASE;
}

static inline S3C24X0_LCD * const S3C24X0_GetBase_LCD(void)
{
    return (S3C24X0_LCD * const)S3C24X0_LCD_BASE;
}

void lcd_ClearScr( unsigned int c)
{
	unsigned int x,y ;

	for( y = 0 ; y < LCD_YSIZE_TFT ; y++ )
	{
		for( x = 0 ; x < (SCR_XSIZE_TFT) ; x++ )
		{
			LCD_BUFFER[y][x] = c;
		}
	}
}


void lcd_EnvidOnOff(int onoff)
{
	S3C24X0_LCD * const lcd = S3C24X0_GetBase_LCD() ;

	if( onoff == 1 )
		lcd -> LCDCON1 |= 1 ;						// ENVID=ON
	else
		lcd -> LCDCON1 = lcd -> LCDCON1 & 0x3fffe ;			// ENVID Off
}
void paint_Bmp(int x0,int y0,int width,int height, unsigned char bmp[])
{
	int x, y ;
	unsigned int c ;
	int p = 0 ;

	for( y = y0 ; y < height +y0; y++ )
	{
		for( x = x0 ; x < width + x0; x++ )
		{
			c = bmp[p+1] | (bmp[p] << 8) ;

			if ( ( x < (SCR_XSIZE_TFT) ) && ( y < LCD_YSIZE_TFT ) )
				LCD_BUFFER[y][x] = c;
			p = p + 2 ;
		}
	}

}


void lcd_PowerEnable(int invpwren , int pwren)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO() ;
	S3C24X0_LCD * const lcd = S3C24X0_GetBase_LCD() ;
										//GPG4 is setted as LCD_PWREN
	gpio -> GPGUP = gpio -> GPGUP & (( ~( 1 << 4) ) | ( 1 << 4) );		// Pull-up disable
	gpio -> GPGCON = gpio -> GPGCON & (( ~( 3 << 8) ) | ( 3 << 8) );		//GPG4=LCD_PWREN
	gpio -> GPGDAT = gpio -> GPGDAT | (1 << 4 ) ;
										//invpwren=pwren;
										//Enable LCD POWER ENABLE Function
	lcd -> LCDCON5 = lcd -> LCDCON5 & (( ~( 1 << 3 ) ) | ( pwren << 3 ) );	// PWREN
	lcd -> LCDCON5 = lcd -> LCDCON5 & (( ~( 1 << 5 ) ) | ( invpwren << 5 ) );	// INVPWREN
}

void lcd_Init(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO() ;
	S3C24X0_LCD * const lcd = S3C24X0_GetBase_LCD() ;

	char *s_lcd;

	lcd -> LCDSADDR1 = ( ( ( unsigned int ) LCD_BUFFER >> 22 ) << 21 ) | M5D ( ( unsigned int ) LCD_BUFFER >> 1 ) ;
	lcd -> LCDSADDR2 = M5D( ( ( unsigned int) LCD_BUFFER + ( SCR_XSIZE_TFT * LCD_YSIZE_TFT * 2 ) ) >> 1 );
	lcd -> LCDSADDR3 = ( ( ( SCR_XSIZE_TFT - LCD_XSIZE_TFT ) / 1 ) << 11 ) | ( LCD_XSIZE_TFT /1 ) ;

	
	lcd_ClearScr( 0x0 ) ;

	gpio -> GPCUP  = 0xffffffff ;
	gpio -> GPCCON = 0xaaaaaaaa ;						//Initialize VD[0:7]	 
	 
	gpio -> GPDUP  = 0xffffffff ;
	gpio -> GPDCON = 0xaaaaaaaa ;						//Initialize VD[15:8]

	lcd -> LCDCON1 = ( CLKVAL_TFT << 8 ) | ( MVAL_USED << 7 ) | (3 << 5 ) | ( 12 << 1 ) | 0 ;
										// TFT LCD panel,16bpp TFT,ENVID=off
	lcd -> LCDCON2 = ( VBPD << 24 ) | ( LINEVAL_TFT << 14 ) | ( VFPD << 6 ) | ( VSPW ) ;
	lcd -> LCDCON3 = ( HBPD << 19 ) | ( HOZVAL_TFT << 8 ) | ( HFPD ) ;
	lcd -> LCDCON4 = ( MVAL << 8 ) | ( HSPW ) ;
	lcd -> LCDCON5 = ( 1 << 11) | ( 0 << 10 ) | ( 1 << 9 ) | ( 1 << 8 ) | ( 0 << 7 ) | ( 0 << 6 ) | ( 1 << 3 ) | ( BSWP << 1 ) | ( HWSWP ) ;
	lcd -> LCDINTMSK |= (3) ;						// MASK LCD Sub Interrupt
	lcd -> LPCSEL &= ( ~7 ) ;						// Disable LPC3480
	lcd -> TPAL = 0x0 ;							// Disable Temp Palette

	lcd_PowerEnable( 0, 1 ) ;
	lcd_EnvidOnOff ( 0) ;
}

void PutPixel(short x, short y, unsigned char c)
{
	if ( (x < LCD_XSIZE_TFT) && (y < LCD_YSIZE_TFT) )
		//LCD_BUFFER[(y)][(x)] = color;
		LCD_BUFFER[y][x] = c;
		puts(" PutPixel(...\n\r");
}
//extern char down_mode;
extern unsigned char logo[];
void my_logo(void)
{
	lcd_ClearScr(0);
	//paint_Bmp(0,0,12,80,logo);
	paint_Bmp(200, 96, 80, 80, logo);		//显示进入下载模式，中�?
	//paint_Bmp((LCD_XSIZE_TFT -180) / 2, (LCD_YSIZE_TFT -20) / 2, 180, 20, down_mode);
	lcd_EnvidOnOff ( 1 ) ;
}

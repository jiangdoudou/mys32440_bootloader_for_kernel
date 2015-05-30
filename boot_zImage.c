/*
* Platform  for: s3c2440
* author  by jiangdou
* you  have anything,plese to QQ:344283973
* time  at: 2009-0801
*
*/



//#include <./2440.h>
//#include <setup.h>
struct param_struct {
    union {
	struct {
	    unsigned long page_size;		/*  0 */
	    unsigned long nr_pages;		/*  4 */
	    unsigned long ramdisk_size;		/*  8 */
	    unsigned long flags;		/* 12 */
#define FLAG_READONLY	1
#define FLAG_RDLOAD	4
#define FLAG_RDPROMPT	8
	    unsigned long rootdev;		/* 16 */
	    unsigned long video_num_cols;	/* 20 */
	    unsigned long video_num_rows;	/* 24 */
	    unsigned long video_x;		/* 28 */
	    unsigned long video_y;		/* 32 */
	    unsigned long memc_control_reg;	/* 36 */
	    unsigned char sounddefault;		/* 40 */
	    unsigned char adfsdrives;		/* 41 */
	    unsigned char bytes_per_char_h;	/* 42 */
	    unsigned char bytes_per_char_v;	/* 43 */
	    unsigned long pages_in_bank[4];	/* 44 */
	    unsigned long pages_in_vram;	/* 60 */
	    unsigned long initrd_start;		/* 64 */
	    unsigned long initrd_size;		/* 68 */
	    unsigned long rd_start;		/* 72 */
	    unsigned long system_rev;		/* 76 */
	    unsigned long system_serial_low;	/* 80 */
	    unsigned long system_serial_high;	/* 84 */
	    unsigned long mem_fclk_21285;       /* 88 */
	} s;
	char unused[256];
    } u1;
    union {
	char paths[8][128];
	struct {
	    unsigned long magic;
	    char n[1024 - sizeof(unsigned long)];
	} s;
    } u2;
    char commandline[1024];
};
#define MACH_TYPE_S3C2440              168
#define LINUX_KERNEL_OFFSET			0x8000
#define LINUX_PARAM_OFFSET			0x100
#define LINUX_PAGE_SIZE				0x00001000
#define LINUX_PAGE_SHIFT			12
#define LINUX_ZIMAGE_MAGIC			0x016f2818
#define DRAM_SIZE				0x04000000

//extern char bLARGEBLOCK;			//HJ_add 20090807
//extern char b128MB;				//HJ_add 20090807
//extern int NF_ReadID(void);			//HJ_add 20090807
//extern int nand_read_ll_lcd(unsigned char*, unsigned long, int);
//extern int nand_read_ll_lp_lcd(unsigned char*, unsigned long, int);


void * memset(void * s,int c,unsigned long count)
{
	char *xs = (char *) s;

	while (count--)
		*xs++ = c;

	return s;
}

/*
 * Disable IRQs
 */
#define local_irq_disable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ local_irq_disable\n"	\
"	orr	%0, %0, #128\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

static inline void cpu_arm920_cache_clean_invalidate_all(void)
{
__asm__(
	"	mov	r1, #0\n"
	"	mov	r1, #7 << 5\n"		  /* 8 segments */
	"1:	orr	r3, r1, #63 << 26\n"	  /* 64 entries */
	"2:	mcr	p15, 0, r3, c7, c14, 2\n" /* clean & invalidate D index */
	"	subs	r3, r3, #1 << 26\n"
	"	bcs	2b\n"			  /* entries 64 to 0 */
	"	subs	r1, r1, #1 << 5\n"
	"	bcs	1b\n"			  /* segments 7 to 0 */
	"	mcr	p15, 0, r1, c7, c5, 0\n"  /* invalidate I cache */
	"	mcr	p15, 0, r1, c7, c10, 4\n" /* drain WB */
	);
}

void cache_clean_invalidate(void)
{
	cpu_arm920_cache_clean_invalidate_all();
}

static inline void cpu_arm920_tlb_invalidate_all(void)
{
	__asm__(
		"mov	r0, #0\n"
		"mcr	p15, 0, r0, c7, c10, 4\n"	/* drain WB */
		"mcr	p15, 0, r0, c8, c7, 0\n"	/* invalidate I & D TLBs */
		);
}

void tlb_invalidate(void)
{
	cpu_arm920_tlb_invalidate_all();
}


void  call_linux(long a0, long a1, long a2)
{
 	local_irq_disable();
	cache_clean_invalidate();
	tlb_invalidate();

__asm__(
	"mov	r0, %0\n"
	"mov	r1, %1\n"
	"mov	r2, %2\n"
	"mov	ip, #0\n"
	"mcr	p15, 0, ip, c13, c0, 0\n"	/* zero PID */
	"mcr	p15, 0, ip, c7, c7, 0\n"	/* invalidate I,D caches */
	"mcr	p15, 0, ip, c7, c10, 4\n"	/* drain write buffer */
	"mcr	p15, 0, ip, c8, c7, 0\n"	/* invalidate I,D TLBs */
	"mrc	p15, 0, ip, c1, c0, 0\n"	/* get control register */
	"bic	ip, ip, #0x0001\n"		/* disable MMU */
	"mcr	p15, 0, ip, c1, c0, 0\n"	/* write control register */
	"mov	pc, r2\n"
	"nop\n"
	"nop\n"
	: /* no outpus */
	: "r" (a0), "r" (a1), "r" (a2)
	: "r0","r1","r2","ip"
	);
}


/*
 * pram_base: base address of linux paramter
 */
static void setup_linux_param(long param_base)
{
	struct param_struct *params = (struct param_struct *)param_base; 
	char *linux_cmd;
	linux_cmd = "noinitrd root=/dev/mtdblock2 init=/linuxrc console=ttySAC0";
//	puts("Setup linux parameters at 0x%08lx\n", param_base);
	memset(params, 0, sizeof(struct param_struct));

	params->u1.s.page_size = LINUX_PAGE_SIZE;
	params->u1.s.nr_pages = (DRAM_SIZE >> LINUX_PAGE_SHIFT);
	//params->commandline = "noinitrd root=/dev/mtdblock2 init=/linuxrc console=ttySAC0";
	/* set linux command line */
	//linux_cmd = getenv ("bootargs");
	//if (linux_cmd == NULL) {
		//puts("Wrong magic: could not found linux command line\n\r");
	//} else {
		strcpy((char *)params->commandline, linux_cmd, strlen(linux_cmd) + 1);
		puts("linux command line is:noinitrd root=/dev/mtdblock2 init=/linuxrc console=ttySAC0\n\r");
	//}
}


/*
 * dst: destination address
 * src: source
 * size: size to copy
 * mt: type of storage device
 */


static inline int copy_kernel_img(long dst, const char *src, int size)
{
	int ret = 0;
	ret = nand_read((unsigned char *)dst,(unsigned long)src, (int)size);
	return ret;
}

int boot_zImage(long from, int size)
{
	//puts("BootingLinux2\n\r");
	int ret;
	long boot_mem_base;	/* base address of bootable memory */
	long to;
	long mach_type;

	boot_mem_base = 0x30000000;

	/* copy kerne image */
	to = boot_mem_base + LINUX_KERNEL_OFFSET;
	puts("Copy linux kernel from 0x00200000 to 0x30008000, size = 0x40000 .....\n\r ");
	ret = copy_kernel_img(to, (char *)from, size);
	if (ret) {
		puts("failed\n\r");
		return -1;
	} else {
		puts("Copy Kernel to SDRAM done,\n\r");
	}

	if (*(long *)(to + 9*4) != LINUX_ZIMAGE_MAGIC) {
		puts("Warning: this binary is not compressed linux kernel image\n");
		puts("zImage magic = 0x%08lx\n", *(long *)(to + 9*4));
	} else {
//		puts("zImage magic = 0x%08lx\n", *(long *)(to + 9*4));
		;
	}

	/* Setup linux parameters and linux command line */
	setup_linux_param(boot_mem_base + LINUX_PARAM_OFFSET);

	/* Get machine type */
	mach_type = MACH_TYPE_S3C2440;
	puts("MACH_TYPE = 168\n\r");

	/* Go Go Go */
	puts("NOW, Booting Linux......\n");	
	call_linux(0, mach_type, to);

	return 0;	
}





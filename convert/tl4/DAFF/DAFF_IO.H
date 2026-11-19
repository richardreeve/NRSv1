#include <dos.h>

#ifdef __TURBOC__

# include <setjmp.h>

#else
struct	time	{
	unsigned char	ti_min;		/* Minutes */
	unsigned char	ti_hour;	/* Hours */
	unsigned char	ti_hund;	/* Hundredths of seconds */
	unsigned char	ti_sec;		/* Seconds */
};

struct	date	{
	int		da_year;	/* Year - 1980 */
	char		da_day;		/* Day of the month */
	char		da_mon;		/* Month (1 = Jan) */
};

typedef struct {
	unsigned	j_sp;
	unsigned	j_ss;
	unsigned	j_flag;
	unsigned	j_cs;
	unsigned	j_ip;
	unsigned	j_bp;
	unsigned	j_di;
	unsigned	j_es;
	unsigned	j_si;
	unsigned	j_ds;
}	jmp_buf[1];

void	longjmp(jmp_buf jmpb, int retval);
int	setjmp(jmp_buf jmpb);
#endif

typedef unsigned int word;
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned short ushort;

typedef enum {
   dt_STRUC,dt_SSTRUC,         /* unknown structure */
   dt_STRING,dt_SSTRING,       /* ascii chars       */
   dt_BYTE,dt_SBYTE,           /* (un)signed Byte   */
   dt_WORD,dt_SWORD,           /* (un)signed Word   */
   dt_LONG,dt_SLONG,           /* (un)signed Long   */
   dt_REAL4,dt_SREAL4,         /* (un)signed Real*4 */
   dt_REAL8,dt_SREAL8,         /* (un)signed Real*8 */
   dt_BROKEN,dt_SBROKEN,       /* (un)signed Real*6 */
   dt_BIT8,dt_SBIT8,           /* (un)signed Bit8   */
   dt_BIT16,dt_SBIT16,         /* (un)signed Bit16  */
   dt_TEXTF,dt_STEXTF          /* (un)signed TextF  */
   } DataTyp;

typedef unsigned char BYTE;
typedef   signed char SBYTE;
typedef unsigned int  WORD;
typedef   signed int  SWORD;
typedef unsigned long LONG;
typedef   signed long SLONG;
typedef float         REAL4;
typedef double        REAL8;

typedef struct broken {
 long mantissa;
 word exponent;
 } Broken;

#define YES    1
#define NO     0

#define DAFF_SMALL_SIZE 0xFFFF
#define DAFF_LONG_SIZE  0xFFFFFFFFL
#define DAFF_DONT_KNOW  DAFF_LONG_SIZE
#define DAFF_CONTEXT    0x8000
#define DAFF_ECTX       0xFFFF
#define DAFF_EOF        0xFFFE

#define DAFF_ID "SWGBSMBWS"
#define DAFF_SEX_INTEL    (('L') | (('H')<<8))
#define DAFF_SEX_MOTOROLA (('H') | (('L')<<8))
#define DAFF_VERSION 0x0100

/*
 * Error codes
 */
enum { DAFF_MEM_ERR=1,          /*  Memory allocation error   */
       DAFF_FNF_ERR,            /*  File not found            */
       DAFF_CNC_ERR,            /*  Cannot create a file      */
       DAFF_RD_ERR,             /*  Read Error                */
       DAFF_WR_ERR,             /*  Write Error               */
       DAFF_PAR_ERR,            /*  Parameter (Internal) err  */
       DAFF_NESTING_ERR,        /*  Illegal Ctx nesting err   */
       DAFF_ILL_ERR,            /*  Illegal Header (No DAFF)  */
       DAFF_SEX_ERR };          /*  Unupported File (Byte sex)*/

/*
 * Predefined Context IDs
 */
enum { ctx_ERR=0,               /* Illegal context    */
       ctx_CTXDEF,              /* Context Definition */
       ctx_HEADER,              /* File Header        */
       ctx_DATADEF,             /* Data Definition    */
       ctx_DATA,                /* Data               */
       ctx_ENTITY,              /* Scaling, etc.      */
       ctx_DOCUM,               /* Documentation      */
       ctx_RESULTS};            /* Analysis results   */
/*
 * Predefined Tag IDs
 */
enum { tag_ERR=0,               /* Illegal Tag        */
       tag_NAME=1,              /* Identifier         */
       tag_LABEL,               /* additional text    */
       tag_DATE,                /* Date/Time          */
       tag_VALUE,               /* value,result,etc.  */

       tag_FTYP=128,            /* File Type          */
       tag_PROD,                /* Producer           */
       tag_FACT,                /* Factor             */
       tag_OFFS,                /* Offset             */
       tag_SPEC,                /* Specification      */
       tag_MINV,                /* Minimum Value      */
       tag_MAXV,                /* Maximum Value      */
       tag_MEAN,                /* Arithmetic mean    */
       tag_SDEV,                /* Standard deviation */
       tag_SWEEP,               /* Sweep duration     */
       tag_SFRQ,                /* Sampling frequency */

       tag_SOURCE=150,          /* Source of Data     */
       tag_SRCOFFS,             /* Header in source   */
       tag_BLKSIZE,             /* Size of Block/Scan */
       tag_DSIZE,               /* Data Size (Bytes)  */
       tag_SRCTYP,              /* Format (ASCII/Bin) */

       tag_BLKOFFS=160,         /* Offset in Block    */
       tag_DEMUX,               /* Demultiplex        */
       tag_DATAT,               /* Data Type          */
       tag_DTYPS,               /* Size (add. args 1) */
       tag_DARGS,               /* Addit. args 2      */
       tag_WPBLK };             /* values per block   */


/*
 * Definition of a Tag structure
 */
typedef struct header_tag {
 word  tag_id;            /* id_byte of this tag/context */
 word  tag_num;           /* entity_id or specific param */
 DataTyp tag_dtyp;        /* see enum above              */
 word  tag_size;          /* size in bytes/0xFFFF        */
                          /* Data 8 Bytes                */
 union {                  /* either ...                  */
   struct {
    long size,temp;
    } longs;              /* 2 Longs (2x4=8 byte)        */
   struct broken broken;  /* 8 Bytes                     */
   struct dattim {
     struct date date;    /* 4 byte (dos.h)              */
     struct time time;    /* 4 byte (dos.h)              */
     } dattim;            /* = 8 byte                    */
   char bytes[8];         /* 8 byte                      */
   double        dval;    /* 8 byte                      */
   } tag_parm;
 } Tag,*TAG;

/*
 * Definition of a DAF file structure
 */
typedef struct daff_file {
 FILE *f;                 /* associated file pointer */
 Tag t;                   /* 1 work buffer (a tag)   */
 int error;               /* error code              */
 jmp_buf *err_jmp;        /* ptr to longjump area    */
 int blklvl;              /* current context nesting */
 struct daff_header {     /* FILE HEADER:            */
  char  swg[10];          /* "SWGMBBSWS" */
  word  sex;              /* 'LH'        */
  word  version;          /* 0x100       */
  word  flags;            /* 0           */
  long  size;             /* 0           */
  char  res[8];           /* reserved    */
  } header;               /* ------------------------*/
 Tag blk_tags[16];        /* Tag nesting             */
 } Daf, *DAF;

/*---------------------------------------------------
 | function prototypes and macro definition         |
 ---------------------------------------------------*/

#define daff_get_pos(hf) ((hf->f) ? ftell(hf->f) : hf->header.size)
#define daff_seek_pos(hf,n) if (hf->f) fseek(hf->f,n,SEEK_SET)
#define daff_skip_pos(hf,n) if (hf->f) fseek(hf->f,n,SEEK_CUR)
#define TAG_(f,n) (f->t.tag_##n)

/* convert double to broken and vice versa          */
void d_2_broken(double d,Broken *b);
double broken_2_d(Broken *b);

/* Error Handling with DAFF-Files                  */
jmp_buf *daff_enable_jmp(DAF f, jmp_buf where);
int daff_error(DAF f,int err);

/* DAFF file operations                            */
DAF daff_create(char *fn,int *err,int flags);
DAF daff_open(char *fn,int *err);
DAF daff_append(char *fn,int *err);
void daff_close(DAF f);

/* Output to DAFF files                           */
int daff_begin_ctx(DAF f,int ctx_type,int num,ulong size);
int daff_end_ctx(DAF f,word ctx_type);

int write_tag(DAF f,void *d);
int write_void_tag(DAF f,word id,word num,word len,void *val);
int write_str_tag(DAF f,word id,word num,char *s);
int write_typ_tag(DAF f,word id,word num,word dtyp,void *val);
int write_int_tag(DAF f,word id,word num,long val);
int write_dbl_tag(DAF f,word id,word num,double val);
int write_name_tag(DAF f,word num,char *s);
int write_label_tag(DAF f,word num,char *l);
int write_dos_date_tag(DAF f,struct dattim *dattim);
int write_unix_date_tag(DAF f,long unix);
int daff_end_header(DAF f);


/* Input from DAFF files                                 */
int daff_read_ctx(DAF f);
int daff_skip_ctx(DAF f);

int daff_read_tag(DAF f);
int daff_skip_tag(DAF f);

int daff_read_str_tag(DAF f,char **s);
int daff_read_int_tag(DAF f,long *v);
int daff_read_dbl_tag(DAF f,double *v);
int daff_read_void_tag(DAF f, void **v);
int daff_read_date_tag(DAF f,struct dattim *dattim);
int daff_read_unix_tag(DAF f,long *l);

long daff_tag_size(DAF f);

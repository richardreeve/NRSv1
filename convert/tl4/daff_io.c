#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "daff_io.h"

#define tag_set0(f) memset(&f->t,0,sizeof(struct header_tag))
#define tag_write(f) (w_rite(f,&f->t,sizeof(Tag))) ? DAFF_WR_ERR : 0
#define data_write(f,p,n) (w_rite(f,p,n)) ? DAFF_WR_ERR : 0
#define tag_read(f)  (r_ead(f,&f->t,sizeof(Tag))) ? DAFF_RD_ERR : 0
#define data_read(f,p,n) (r_ead(f,p,n)) ? DAFF_RD_ERR : 0
#define daff_eof_pos(f)    if (f->f) fseek(f->f,0L,SEEK_END)
#define set_tag(f,id,p1,p2,sz) f->t.tag_id=id;f->t.tag_par1=p1;\
                               f->t.tag_par2=p2;f->t.tag_size=sz

/*
 * ACHTUNG: Die Verwaltung von dynamischem Speicher geschieht mit
 *          "daff_alloc(size)"     und
 *          "daff_free(pointer)"
 * Falls eine eigene Memory-Verwaltung verwendet wird, bitte
 * hier einsetzen:
 */
void *malloc();
#define daff_alloc(size) malloc(size)
#define daff_free(ptr)   free(ptr)

#define s_alloc(name) (struct name *)xalloc(sizeof(struct name))

static int r_ead(Daf *f,void *buf,unsigned n)
{
  if (f->f==NULL) f->header.size+=n;
  else if (fread(buf,1,n,f->f)!=n || ferror(f->f)) return YES;
  return NO;
}

static int w_rite(Daf *f,void *buf,unsigned n)
{
  if (f->f==NULL) f->header.size+=n;
  else if (fwrite(buf,1,n,f->f)!=n || ferror(f->f)) return YES;
  return NO;
}
static char *xalloc(unsigned size)
{
  char *x=(char *)daff_alloc(size);
  if (x) memset(x,0,size);
  return x;
}

/*********************************************
 * convert double to broken & vice versa     *
 *********************************************/

void d_2_broken(double d,Broken *b)
{
  if (d==0)
  {
    b->exponent=0;
    b->mantissa=0;
  }
  else
  {
    d=frexp(d,(int *)&b->exponent);
    b->mantissa=(long int)d*0x80000000UL;
  }
}

double broken_2_d(Broken *b)
{
  double d=(double)b->mantissa/0x80000000UL;
  return ldexp(d,b->exponent);
}

//#if 0
#ifndef __TURBOC__
void getdate(struct date *date)
{
  struct timeval tv;
  struct tm result;
  time_t tt;
  gettimeofday( &tv, NULL );
  tt = (time_t) tv.tv_sec;
  localtime_r( &tt, &result );
  
  date->da_year = result.tm_year - 80;
  date->da_day  = result.tm_mday;
  date->da_mon  = result.tm_mon;
}
void gettime(struct time *time)
{
  struct timeval tv;
  struct tm result;
  time_t tt;
  gettimeofday( &tv, NULL );
  tt = (time_t) tv.tv_sec;
  localtime_r( &tt, &result );
  
  time->ti_min=result.tm_min;
  time->ti_hour=result.tm_hour;
  time->ti_hund=tv.tv_usec/10000;
  time->ti_sec =result.tm_sec;
}

#define DSTSTART	(24 * 118 + 2)
#define DSTEND		(24 * 296 + 2)

static char Days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

long dostounix(struct date *d, struct time *t)
{
	long		x;
	register int	i;
	register int	days;
	int		hours;

#ifndef WIN32
	x = 24L * 60L * 60L * 3652L + timezone; /* Convert from 1980 to
						   1970 base date GMT */
#else
#warning "No handling of timezone information"
	x = 24L * 60L * 60L * 3652L; /* Convert from 1980 to
					1970 base date GMT */
#endif

	i = d->da_year - 1980;
	x += (i >> 2) * (1461L * 24L * 60L * 60L);
	x += (i & 3) * (24L * 60L * 60L * 365L);
	if (i & 3) x += 24L * 60L * 60L;
	days = 0;
	i = d->da_mon - 1;			/* Add in months */
	while (i > 0)
	{
		i--;
		days += Days[i];
	}
	days += d->da_day - 1;
	hours = days * 24 + t->ti_hour;		/* Find hours */
	if (daylight && hours >= DSTSTART && hours < DSTEND)
		hours--;
	if (d->da_mon > 2 && (d->da_year & 3) == 0)
		hours += 24;			/* Currently in leap year */
	x += hours * 3600L;
	x += 60L * t->ti_min + t->ti_sec;
	return (x);
}

void unixtodos(long time, struct date *d, struct time *t)
{
#ifndef WIN32
	time -= 24L * 60L * 60L * 3652L + timezone;
#else
#warning "No handling of timezone information"
	time -= 24L * 60L * 60L * 3652L;
#endif

	t->ti_hund = 0;
	t->ti_sec = time % 60;
	time /= 60;				/* Time in minutes */
	t->ti_min = time % 60;
	time /= 60;				/* Time in hours */
	d->da_year = 1980 + (int)((time / (1461L * 24L)) << 2);
	time %= 1461L * 24L;
	if (time > 366 * 24)
	{
		time -= 366 * 24;
		d->da_year++;
		d->da_year += time / (365 * 24);
		time %= 365 * 24;
	}
	if (daylight && time >= DSTSTART && time < DSTEND)
		time++;
	t->ti_hour = time % 24;
	time /= 24;				/* Time in days */
	time++;
	if ((d->da_year & 3) == 0)
	{
		if (time > 60)
			time--;
		else
		if (time == 60)
		{
			d->da_mon = 2;
			d->da_day = 29;
			return;
		}
	}
	for (d->da_mon = 0; Days[(int)d->da_mon] < time; d->da_mon++)
		time -= Days[(int)d->da_mon];
	d->da_mon++;
	d->da_day = time;
}

#endif

static int dtyp_sizes[]= {
  0,0,    /* STRUC  */
  0,0,    /* String */
  1,1,    /* Byte   */
  2,2,    /* Word   */
  4,4,    /* Long   */
  4,4,    /* Real4  */
  8,8,    /* Real8  */
  6,6,    /* Real6  */
  1,1,    /* Bit8   */
  2,2,    /* Bit16  */
  0,0 };  /* TextF  */

/*
 * Error Handling with DAFF-Files:
 */

jmp_buf *daff_enable_jmp(Daf *f, jmp_buf berta)
{
  jmp_buf *save=f->err_jmp;
  f->err_jmp=(jmp_buf *)berta;
  return save;
}

int daff_error(Daf *f,int err)
{
  if ((f->error=err)!=0 && f->err_jmp!=NULL)
  {
    longjmp(*f->err_jmp,err);
  }
  return err;
}

/**************************************************************
 * write tags : return is 0 (OKAY) or Error-NR (DAFF_xxx_ERR) *
 *------------------------------------------------------------*
 * str -- write a string (tag_id=id, tag_par1=par1, arg=s)    *
 * int -- write a long(tag_id=id,tag_par1=par1,tag_par2=par2) *
 * dbl -- write a double (convert to broken)                  *
 * void - write some bytes (*ptr,len)                         *
 * date - write DOS-formatted date & time                     *
 **************************************************************/
int write_tag(Daf *f,void *d)
{
  word len,err;
  if ((len=f->t.tag_size)<=8)
  {
    if (len) memcpy(f->t.tag_parm.bytes,d,len);
    return daff_error(f,tag_write(f));
  }
  else
  {
    len=(len+1)&~1;
    err=tag_write(f);
    if (!err && len) err=data_write(f,d,len);
    return daff_error(f,err);
  }
}

int write_void_tag(Daf *f,word id,word num,word len,void *val)
{
  tag_set0(f);
  f->t.tag_id=id; f->t.tag_num=num; f->t.tag_dtyp=dt_STRUC;
  f->t.tag_size=len;
  return write_tag(f,val);
}

int write_str_tag(Daf *f,word id,word num,char *s)
{
  tag_set0(f);
  f->t.tag_id=id; f->t.tag_num=num;
  f->t.tag_dtyp=dt_STRING;
  f->t.tag_size=strlen(s);
  return write_tag(f,s);
}
int write_typ_tag(Daf *f,word id,word num,word dtyp,void *val)
{
  tag_set0(f);
  f->t.tag_id=id; f->t.tag_num=num; f->t.tag_dtyp=dtyp;
  f->t.tag_size=dtyp_sizes[dtyp];
  if (dtyp<dt_BYTE || dtyp>=dt_TEXTF) return daff_error(f,DAFF_PAR_ERR);
  return write_tag(f,val);
}

int write_int_tag(Daf *f,word id,word num,long val)
{
  return write_typ_tag(f,id,num,dt_LONG,&val);
}
int write_dbl_tag(Daf *f,word id,word num,double val)
{
  struct broken b;
  d_2_broken(val,&b);
  return write_typ_tag(f,id,num,dt_BROKEN,&b);
}
int write_name_tag(Daf *f,word num,char *s)
{
  return write_str_tag(f,tag_NAME,num,s);
}
int write_label_tag(Daf *f,word num,char *l)
{
  return write_str_tag(f,tag_LABEL,num,l);
}
/*
 * DOS_formatted DATE & TIME; if "dattim"==NULL, write
 * current date & time
 */
int write_dos_date_tag(Daf *f,struct dattim *dattim)
{
  struct dattim now;
  if (dattim==NULL)
  {
    getdate(&now.date);
    gettime(&now.time);
  }
  else now=*dattim;
  return write_void_tag(f,tag_DATE,0,sizeof(now),&now);
}
/*
 * UNIX_formatted DATE & TIME; if "unix"==0L, write
 * current date & time
 */
int write_unix_date_tag(Daf *f,long theunix)
{
  if (theunix==0L)
  {
    struct date date;
    struct time time;
    getdate(&date); gettime(&time);
    theunix=dostounix(&date,&time);
  }
  return write_typ_tag(f,tag_DATE,1,dt_LONG,&theunix);
}

int daff_begin_ctx(Daf *f,int ctx_type,int num,ulong size)
{
  int lvl; TAG t;
  if (f->blklvl==16) return daff_error(f,DAFF_NESTING_ERR);
  lvl=f->blklvl++; t=f->blk_tags+lvl;
  tag_set0(f);
  f->t.tag_id=ctx_type|DAFF_CONTEXT;
  f->t.tag_num=num;
  f->t.tag_dtyp=0;
  if (size)
  {
    if (size<DAFF_SMALL_SIZE)
    {
      f->t.tag_size=size; size=0;
    }
    else f->t.tag_size=DAFF_SMALL_SIZE;
  }
  f->t.tag_parm.longs.size=size;
  *t=f->t;
  t->tag_parm.longs.temp=daff_get_pos(f);
  return daff_error(f,tag_write(f));
}

int daff_end_ctx(Daf *f,word ctx_type)
{
  int lvl; TAG t; long save,pos,size;
  if (f->blklvl==0) return daff_error(f,DAFF_NESTING_ERR);
  lvl=--f->blklvl; t=f->blk_tags+lvl;
  if (t->tag_id!=(ctx_type|DAFF_CONTEXT)) return daff_error(f,DAFF_NESTING_ERR);
  tag_set0(f);
  f->t.tag_id=DAFF_ECTX; f->t.tag_num=ctx_type;
  if (daff_error(f,tag_write(f))) return f->error;
  if (f->f==NULL) return NO;
  if (t->tag_size==0 ||
     (t->tag_size==(long)DAFF_SMALL_SIZE &&t->tag_parm.longs.size==(long)DAFF_LONG_SIZE))
  {
    save=daff_get_pos(f); pos=t->tag_parm.longs.temp;
    size=save-pos-sizeof(Tag);
    if (size<DAFF_SMALL_SIZE)
    {
      t->tag_size=size;
      t->tag_parm.dval=0;
    }
    else
    {
      t->tag_size=DAFF_SMALL_SIZE;
      t->tag_parm.longs.size=size;
      t->tag_parm.longs.temp=0;
    }
    f->t=*t;
    daff_seek_pos(f,pos);
    if (daff_error(f,tag_write(f))) return f->error;
    daff_seek_pos(f,save);
  }
  return NO;
}

/*
 * daff_create:
 *    fn    = filename
 *    err   = error_flag
 *    flags = cf. header.flags
 */
Daf *daff_create(char *fn,int *err,int flags)
{
  Daf *f=s_alloc(daff_file);
  *err=0;
  if (f==NULL)
  {
    *err=DAFF_MEM_ERR;
    return NULL;
  }
  if (fn==NULL)
  {
    f->header.size=sizeof(struct daff_header);
    return f;
  }
  f->f = fopen(fn,"w+b");
  if (f->f==NULL)
  {
    daff_free(f);
    *err=DAFF_CNC_ERR;
    return NULL;
  }
  strcpy(f->header.swg,DAFF_ID);
  f->header.sex=DAFF_SEX_INTEL;
  f->header.version=DAFF_VERSION;
  f->header.flags=flags;
  if (w_rite(f,&f->header,sizeof(struct daff_header)))
  {
    fclose(f->f); unlink(fn);
    daff_free(f);
    *err=DAFF_CNC_ERR;
    return NULL;
  }
  return f;
}

/*
 * daff_end_header
 *  may be called prior to writing data in order to update
 *  the header.size (which is not DAFF standard!)
 */
int daff_end_header(Daf *f)
{
  if (f->f!=NULL)
  {
    long pos=daff_get_pos(f);
    f->header.size=pos;
    daff_seek_pos(f,0L);
    if (w_rite(f,&f->header,sizeof(struct daff_header)))
       return daff_error(f,DAFF_WR_ERR);
    daff_seek_pos(f,pos);
  }
  return NO;
}

void daff_close(Daf *f)
{
  if (f->f) fclose(f->f);
  daff_free(f);
}


/* ---------------- INPUT ------------------------ */
/* ATTENTION: strings and void types allocate      */
/* dynamic memory with function daff_alloc()       */
/***************************************************/
int daff_read_str_tag(Daf *f,char **s)
{
  word len=f->t.tag_size;
  if (len==0) { *s=NULL; return NO; }
  if (len==0xFFFF) return daff_error(f,DAFF_PAR_ERR);
  if ((*s=xalloc(len+1))==NULL) return daff_error(f,DAFF_MEM_ERR);
  if (len<=8)
  {
    strncpy(*s,f->t.tag_parm.bytes,len);
    return NO;
  }
  else
  {
    len=(len+1)&~1;
    return daff_error(f,data_read(f,*s,len));
  }
}

int daff_read_int_tag(Daf *f,long *v)
{
  void *ptr; int dt; double dv;
  if ((dt=f->t.tag_dtyp)<dt_BYTE || dt>dt_SBROKEN)
       return daff_error(f,DAFF_PAR_ERR);
  if (f->t.tag_size!=dtyp_sizes[dt]) return daff_error(f,DAFF_PAR_ERR);
  ptr=f->t.tag_parm.bytes;
  if (dt<dt_REAL4) switch (dt)
  {
  case dt_BYTE : *v=*(unsigned char *) ptr; break;
  case dt_SBYTE: *v=*(  signed char *) ptr; break;
  case dt_WORD : *v=*(unsigned int  *) ptr; break;
  case dt_SWORD: *v=*(  signed int  *) ptr; break;
  case dt_LONG : *v=*(unsigned long *) ptr; break;
  case dt_SLONG: *v=*(  signed long *) ptr; break;
  }
  else
  {
    if (daff_read_dbl_tag(f,&dv)) return f->error;
    *v=(long int)dv;
  }
  return NO;
}
int daff_read_dbl_tag(Daf *f,double *v)
{
  void *ptr; int dt; unsigned long lv;
  if ((dt=f->t.tag_dtyp)<dt_BYTE || dt>dt_SBROKEN)
        return daff_error(f,DAFF_PAR_ERR);
  if (f->t.tag_size!=dtyp_sizes[dt]) return daff_error(f,DAFF_PAR_ERR);
  ptr=f->t.tag_parm.bytes;
  if (dt>=dt_REAL4 && dt<=dt_SBROKEN) switch (dt)
  {
  case dt_REAL4: case dt_SREAL4:
       *v= *(float *) ptr; break;
  case dt_REAL8: case dt_SREAL8:
       *v= *(double *)ptr; break;
  case dt_BROKEN: case dt_SBROKEN:
       *v= broken_2_d((struct broken *)ptr);
       break;
  }
  else
  {
    if (daff_read_int_tag(f,(long *)&lv)) return f->error;
    if (dt&1) *v=(signed long)lv;
    else *v=lv;
  }
  return NO;
}

int daff_read_void_tag(Daf *f, void **v)
{
  word len=f->t.tag_size;
  if (len==0) { *v=NULL; return NO; }
  if (len==0xFFFF) return daff_error(f,DAFF_PAR_ERR);
  if (len<=8)
  {
    if ((*v=xalloc(len))==NULL) return daff_error(f,DAFF_MEM_ERR);
    memcpy(*v,f->t.tag_parm.bytes,len);
    return NO;
  }
  else
  {
    len=(len+1)&~1;
    if ((*v=xalloc(len))==NULL) return daff_error(f,DAFF_MEM_ERR);
    return daff_error(f,data_read(f,*v,len));
  }
}

int daff_read_date_tag(Daf *f,struct dattim *dattim)
{
  int fmt;
  memset(dattim,0,sizeof(struct dattim));
  if (f->t.tag_id!=tag_DATE || (fmt=f->t.tag_num)>1 || f->t.tag_size>8)
    return daff_error(f,DAFF_PAR_ERR);
  if (fmt==0) /* DOS-format */
  {
    *dattim=f->t.tag_parm.dattim;
  }
  else
  {
    unixtodos(f->t.tag_parm.longs.size,&dattim->date,&dattim->time);
  }
  return NO;
}
int daff_read_unix_tag(Daf *f,long *date)
{
  int fmt;
  if (f->t.tag_id!=tag_DATE || (fmt=f->t.tag_num)>1 || f->t.tag_size>8)
    return daff_error(f,DAFF_PAR_ERR);
  if (fmt==0) /* DOS-format */
  {
    *date=dostounix(&f->t.tag_parm.dattim.date,&f->t.tag_parm.dattim.time);
    return NO;
  }
  else
  {
    *date=f->t.tag_parm.longs.size;
  }
  return NO;
}

int daff_read_tag(Daf *f)
{
  int err;
  if (feof(f->f)) return DAFF_EOF;
  err=tag_read(f);
  if (feof(f->f)) return DAFF_EOF;
  if (daff_error(f,err)) return 0;
  else return f->t.tag_id;
}

int daff_skip_tag(Daf *f)
{
  long size;
  if ((size=f->t.tag_size)==DAFF_SMALL_SIZE)
  {
    if ((size=f->t.tag_parm.longs.size)==(long)DAFF_LONG_SIZE)
    {
       daff_eof_pos(f);
       return NO;
    }
  }
  if (size>8)
  {
    size=(size+1)&~1;
    daff_skip_pos(f,size);
  }
  return NO;
}

int daff_skip_ctx(Daf *f)
{
  int err; word id;
  if (f->t.tag_size!=0) return daff_skip_tag(f);
  /* don't know the exact size in advance - try */
  err=-1;
  do
  {
    id=daff_read_tag(f);
    if (id==0) return f->error;
    if (id==DAFF_ECTX) err=0;               /* END Context   */
    else if (id&DAFF_CONTEXT) err=daff_skip_ctx(f);     /* BEGIN_CONTEXT */
    else err=daff_skip_tag(f);
  } while (err<0);
  return err;
}

Daf *daff_open(const char *fn,int *err)
{
  Daf *f=s_alloc(daff_file);
  *err=0;
  if (f==NULL)
  {
    *err=DAFF_MEM_ERR;
    return NULL;
  }
  f->f=fopen(fn,"rb");
  if (f->f==NULL)
  {
    *err=DAFF_FNF_ERR;
    daff_free(f);
    return NULL;
  }
  if (r_ead(f,&f->header,sizeof(struct daff_header)) ||
      strcmp(f->header.swg,DAFF_ID))
  {
    *err=DAFF_ILL_ERR;
    fclose(f->f);
    daff_free(f);
    return NULL;
  }
  if (f->header.sex!=DAFF_SEX_INTEL)
  {
    *err=DAFF_SEX_ERR;
    fclose(f->f);
    daff_free(f);
    return NULL;
  }
  return f;
}
Daf *daff_append(char *fn,int *err)
{
  Daf *f=s_alloc(daff_file);
  *err=0;
  if (f==NULL)
  {
    *err=DAFF_MEM_ERR;
    return NULL;
  }
  f->f=fopen(fn,"r+b");
  if (f->f==NULL)
  {
    *err=DAFF_FNF_ERR;
    daff_free(f);
    return NULL;
  }
  if (r_ead(f,&f->header,sizeof(struct daff_header)) ||
      strcmp(f->header.swg,DAFF_ID))
  {
    *err=DAFF_ILL_ERR;
    fclose(f->f);
    daff_free(f);
    return NULL;
  }
  if (f->header.sex!=DAFF_SEX_INTEL)
  {
    *err=DAFF_SEX_ERR;
    fclose(f->f);
    daff_free(f);
    return NULL;
  }
  fseek(f->f,0L,SEEK_END);
  return f;
}

int daff_read_ctx(Daf *f)
{
  word id=daff_read_tag(f);
  if (id==DAFF_EOF) return id;
  if (!(id&DAFF_CONTEXT)) return 0;
  else if (id==DAFF_ECTX) return 0;
  else return id&~DAFF_CONTEXT;
}

long daff_tag_size(Daf *f)
{
  if (f->t.tag_size==DAFF_SMALL_SIZE) return f->t.tag_parm.longs.size;
  else return f->t.tag_size;
}

/*-------------------------------------------------------------+
 | Erweitertes Beispiel-Programm zu Lesen von DAFF-Dateien     |
 +-------------------------------------------------------------+
 | (c) 1990 STEMMER Software Gesellschaft mbH  D-8039 Puchheim |
 | Alle Rechte vorbehalten                                     |
 +-------------------------------------------------------------+
 +-------------------------------------------------------------+
 | Zusammen mit DAFF_IO.C linken !                             |
 +-------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "daff_io.h"

#define MAX_STR 255
#define NUM_SIG 3

#define TRUE 1
#define FALSE 0

typedef struct {
  int real_signal;
  int signum;
  char name[MAX_STR];
  char units[MAX_STR];
  double freq;
  double offset, scale, min, max;
  int file_offset;
} SignalInfo;

SignalInfo g_sig_info[NUM_SIG];
int g_curr_sig = -1;
int g_num_sigs = 0;
int g_blocksize = 0;
int g_startdata = 0;
/*-------------------------------------------------------------+
 | Routinen zum Ausgeben von (unbekannten) Tags                |
 +-------------------------------------------------------------*/
void hexdump(unsigned char *s,unsigned len)
{
  char line [17]; int j;
  for(j=0; len--; )
  {
    line[j]=(*s>=' ' && *s!=127) ? *s : '.';
    printf("%02X ",*s++);
    if (++j == 16)
    {
      line[j]=0;
      printf("  %s\n",line);
      j=0;
    }
  }
  if (j)
  {
    line[j]=0;
    while (j++ <16 ) printf("   ");
    printf("  %s\n",line);
  }
}
/*------------------------------------------------------------+
 | log_tag(hf,level)  -- Unbekannten Tag ausgeben             |
 | -> hf = DAF - Filestruktur                                 |
 |    level = FÅr EinrÅckung                                  |
 +------------------------------------------------------------*/

void log_tag(DAF hf,int level)
{
  char *s; long lv; double dv;
  unsigned len;
  printf("%*sTAG %u (%d) =",level," ",TAG_(hf,id),TAG_(hf,num));
  switch (TAG_(hf,dtyp))
  {
  case dt_STRUC:  case dt_SSTRUC:         /* unknown structure */
       len=TAG_(hf,size);
       daff_read_void_tag(hf,(void **)&s);
       printf("\n");
       hexdump(s,len);
       free(s);
       break;
  case dt_STRING: case dt_SSTRING:        /* ascii chars       */
       daff_read_str_tag(hf,&s);
       printf("\"%s\"\n",s);
       free(s);
       break;
  case dt_BYTE:   case dt_SBYTE:          /* (un)signed Byte   */
  case dt_WORD:   case dt_SWORD:          /* (un)signed Word   */
  case dt_LONG:   case dt_SLONG:          /* (un)signed Long   */
       daff_read_int_tag(hf,&lv);
       printf("%ld\n",lv);
       break;
   case dt_REAL4:  case dt_SREAL4:         /* (un)signed Real*4 */
   case dt_REAL8:  case dt_SREAL8:         /* (un)signed Real*8 */
   case dt_BROKEN: case dt_SBROKEN:        /* (un)signed Real*6 */
        daff_read_dbl_tag(hf,&dv);
        printf("%lf\n",dv);
        break;

   case dt_BIT8:   case dt_SBIT8:          /* (un)signed Bit8   */
   case dt_BIT16:  case dt_SBIT16:         /* (un)signed Bit16  */
        daff_read_int_tag(hf,&lv);
        printf("%ld\n",lv);
        break;

   case dt_TEXTF:  case dt_STEXTF:         /* (un)signed TextF  */
        daff_read_dbl_tag(hf,&dv);
        printf("%lf\n",dv);
        break;
   default:
        printf("Unknown DataType\n");
        daff_skip_tag(hf);
   }
}
/*------------------------------------------------------------+
 | log_ctx(hf,level)  -- Unbekannten Kontext ausgeben         |
 | -> hf = DAF - Filestruktur                                 |
 |    level = FÅr EinrÅckung                                  |
 +------------------------------------------------------------*/
void log_ctx(DAF hf,int level)
{
  word tid;
  ++level;
  tid=TAG_(hf,id)&~DAFF_CONTEXT;
  printf("%*s<CONTEXT id=%u, num=%d>\n",level," ",tid,TAG_(hf,num));
  do
  {
    tid=daff_read_tag(hf);
    if (tid==DAFF_ECTX)
    {
      printf("%*s</CONTEXT>\n",level," ");
      return;
    }
    else if (tid&DAFF_CONTEXT)
    {
      log_ctx(hf,level+1);
    }
    else log_tag(hf,level);
  } while (1);
}

/*------------------------------------------------------------+
 | log_header(hf)  -- ctx_HEADER ausgeben                     |
 | -> hf = DAF - Filestruktur                                 |
 +------------------------------------------------------------*/

void log_header(DAF hf)
{
  char *s;
  word tid;
  struct dattim d;
  printf("<HEADER>\n");
  do
  {
    tid=daff_read_tag(hf);
    if (tid==DAFF_ECTX)
    {
      printf("</HEADER>\n");
      return;
    }
    else if (tid&DAFF_CONTEXT)
    {
       log_ctx(hf,1);
    }
    else switch (tid)
    {
    case tag_LABEL:
         daff_read_str_tag(hf,&s);
         printf(" LABEL=%s\n",s);
         free(s);
         break;
    case tag_FTYP:
         daff_read_str_tag(hf,&s);
         printf(" FTYP=%s\n",s);
         free(s);
         break;
    case tag_PROD:
         daff_read_str_tag(hf,&s);
         printf(" PROD=%s\n",s);
         free(s);
         break;
    case tag_NAME:
         daff_read_str_tag(hf,&s);
         printf(" NAME=%s\n",s);
         free(s);
         break;
    case tag_DATE:
         daff_read_date_tag(hf,&d);
         printf(" DATE=%d.%d.%d %d:%02d:%02d\n",
            d.date.da_day,d.date.da_mon,d.date.da_year,
            d.time.ti_hour,d.time.ti_min,d.time.ti_sec);
         break;
    default:
         log_tag(hf,1);
         break;
    }
  } while (1);
}

/*------------------------------------------------------------+
 | log_channnel(hf)  -- ctx_ENTITY ausgeben                   |
 | -> hf = DAF - Filestruktur                                 |
 +------------------------------------------------------------*/
void log_channel(DAF hf)
{
  word tid; char *ss,*s; double v;
  do
  {
    tid=daff_read_tag(hf);
    if (tid==DAFF_ECTX) return;
    else if (tid&DAFF_CONTEXT)
    {
      log_ctx(hf,1);
    }
    else switch (tid)
    {
    case tag_LABEL:
         daff_read_str_tag(hf,&s);
         printf("  LABEL=%s\n",s);
         free(s);
         break;
    case tag_NAME:
         daff_read_str_tag(hf,&s);
         printf("  NAME=%s\n",s);
	 strncpy( g_sig_info[g_curr_sig].name, s, MAX_STR );
         free(s);
         break;
    case tag_SPEC:
         daff_read_str_tag(hf,&ss);
         switch (TAG_(hf,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
          }
          printf("  %s-Units=%s\n",s,ss);
	  strncpy( g_sig_info[g_curr_sig].units, ss, MAX_STR );
          free(ss);
          break;
    case tag_FACT:
         daff_read_dbl_tag(hf,&v);
         switch (TAG_(hf,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
         }
         printf("  %s-Scaling-Factor=%lf\n",s,v);
	 if (finite(v))
	   g_sig_info[g_curr_sig].scale = v;
	 else
	   g_sig_info[g_curr_sig].scale = 1;
         break;
    case tag_OFFS:
         daff_read_dbl_tag(hf,&v);
         switch (TAG_(hf,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
          }
          printf("  %s-Offset=%lf\n",s,v);
	  g_sig_info[g_curr_sig].offset = v;
          break;
    case tag_MINV:
         daff_read_dbl_tag(hf,&v);
         switch (TAG_(hf,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
          }
          printf("  %s-Minimum=%lf\n",s,v);
	  g_sig_info[g_curr_sig].min = v;
          break;
    case tag_MAXV:
         daff_read_dbl_tag(hf,&v);
         switch (TAG_(hf,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
          }
          printf("  %s-Maximum=%lf\n",s,v);
	  g_sig_info[g_curr_sig].max = v;
          break;
    case tag_SFRQ:
         daff_read_dbl_tag(hf,&v);
         printf("  SamplingFrq=%lf\n",v);
	 g_sig_info[g_curr_sig].freq = v;
         break;
    case tag_SWEEP:
         daff_read_dbl_tag(hf,&v);
         printf("  SweepSize=%lf\n",v);
	 //	 printf("Don't know what sweepsize is for, panic!\n");
	 //	 exit(1);
         break;
    default      : log_tag(hf,2);
    }
  } while (1);
}
/*------------------------------------------------------------+
 | log_entity(hf)  -- ctx_ENTITY ausgeben                     |
 | -> hf = DAF - Filestruktur                                 |
 +------------------------------------------------------------*/
void log_entity(DAF hf)
{
  word tid,id;
  printf("<ENTITY>\n");
  do
  {
    tid=daff_read_tag(hf);
    if (tid==DAFF_ECTX)
    {
      printf("</ENTITY>\n");
      return;
    }
    else if (!(tid&DAFF_CONTEXT))
    {
      log_tag(hf,1);
    }
    else if ((tid&~DAFF_CONTEXT)!=ctx_ENTITY)
    {
      log_ctx(hf,1);
    }
    else
    {
      id=TAG_(hf,num);
      printf(" <Signal-# = %d> (0=default)\n",id);
      g_curr_sig++;
      g_sig_info[g_curr_sig].real_signal=TRUE;
      g_sig_info[g_curr_sig].signum=id;
      //      printf("-------- %d:%d\n", g_curr_sig, id);
      if (id!=0)
	{
	  if (g_sig_info[0].signum!=0)
	    {
	      printf("We don't know the defaults yet\n");
	      exit(1);
	    }
	  g_sig_info[g_curr_sig].freq = g_sig_info[0].freq;
	  g_sig_info[g_curr_sig].offset = g_sig_info[0].offset;
	  g_sig_info[g_curr_sig].scale = g_sig_info[0].scale;
	  g_sig_info[g_curr_sig].min = g_sig_info[0].min;
	  g_sig_info[g_curr_sig].max = g_sig_info[0].max;
	  g_num_sigs++;
	}
      else // is zero - ie default
	{
	  g_sig_info[g_curr_sig].freq = 1;
	  g_sig_info[g_curr_sig].offset = 0;
	  g_sig_info[g_curr_sig].scale = 1;
	  g_sig_info[g_curr_sig].min = -10;
	  g_sig_info[g_curr_sig].max = 10;
	}
      log_channel(hf);
    }
  } while (1);
}
/*------------------------------------------------------------+
 | log_signal(hf)  -- ctx_DATADEF                             |
 | -> hf = DAF - Filestruktur                                 |
 +------------------------------------------------------------*/

void log_sigdef(DAF hf)
{
  long lv;
  word tid;
  do
  {
    tid=daff_read_tag(hf);
    if (tid==DAFF_ECTX) return;
    else if (tid&DAFF_CONTEXT)
    {
      log_ctx(hf,1);
    }
    else switch (tid)
    {
    case tag_BLKOFFS: daff_read_int_tag(hf,&lv);
         printf("  BLKOFFS=%ld\n",lv);
	 g_sig_info[g_curr_sig].file_offset = lv;
         break;
    case tag_DEMUX:   daff_read_int_tag(hf,&lv);
         printf("  DEMUX=%ld\n",lv);
	 if (g_blocksize != lv)
	   {
	     printf("demux doesn't match blocksize\n");
	     exit(1);
	   }
         break;
    case tag_DATAT:   daff_read_int_tag(hf,&lv);
         printf("  DATAT=%ld\n",lv);
         break;
    case tag_DTYPS:   daff_read_int_tag(hf,&lv);
         printf("  DTYPS=%ld\n",lv);
         break;
    case tag_DARGS:   daff_read_int_tag(hf,&lv);
         printf("  DARGS=%ld\n",lv);
         break;
    case tag_WPBLK:   daff_read_int_tag(hf,&lv);
         printf("  WPBLK=%ld\n",lv);
         break;
    default: log_tag(hf,2);
    }
  } while (1);
}

int log_datadef(DAF hf)
{
  word tid,id;
  char *ss;
  long lv;
  int datafile;
  int i;
  datafile=NO;
  printf("<DATADEF>\n");
  do
  {
    tid=daff_read_tag(hf);
    if (tid==DAFF_ECTX)
    {
      printf("</DATADEF>\n");
      return datafile;
    }
    if (tid&DAFF_CONTEXT)
    {
      if ((tid&~DAFF_CONTEXT)==ctx_DATADEF)
      {
        id=TAG_(hf,num);
        printf(" <Signal-# = %d>\n",id);
	g_curr_sig = -1;
	for ( i=1; i <= g_num_sigs; i++)
	  {
	    //	    printf( "+++++++ %d:%d\n", i, g_sig_info[i].signum);
	    if (id == g_sig_info[i].signum)
	      g_curr_sig = i;
	  }
	if (g_curr_sig == -1)
	  {
	    printf("Oops; couldn't find right signal\n");
	    exit(1);
	  }
        log_sigdef(hf);
      }
      else log_ctx(hf,2);
    }
    else switch (tid)
    {
    case tag_SOURCE:   /* Datei-Name fÅr Daten-File */
         daff_read_str_tag(hf,&ss);
         printf(" SOURCE = %s\n",ss);
         free(ss);
         datafile=YES;
         break;
    case tag_SRCOFFS:  /* Offset im Daten-File */
         daff_read_int_tag(hf,&lv);
         printf(" SRCOFFS=%ld\n",lv);
         break;
    case tag_BLKSIZE: /* Block-Grî·e (falls Datei geblockt) */
         daff_read_int_tag(hf,&lv);
         printf(" BLKSIZE=%ld\n",lv);
	 g_blocksize = lv;
         break;
    case tag_SRCTYP :  /* TYP=1 fÅr ASCII-Dateien */
         daff_read_int_tag(hf,&lv);
         printf(" SRCTYP=%s\n", (lv)? "ASCII" : "BINARY");
         break;
    default         : log_tag(hf,3); break;
    }
  } while (1);
}

/*-------------------------------------------------------------+
 | Haupt-Programm DAFFLOG.C                                    |
 +-------------------------------------------------------------+
 | Aufruf mit DAFFLOG dateiname                                |
 +-------------------------------------------------------------*/

int main(int argc,char **argv)
{
  DAF hf;
  int err;
  jmp_buf here;
  word ctx;
  int done;
  long datasize;
  char *s;
  int i;
  short data;
  long time;
  int step = 1;
  double start_time = 0;
  double end_time = 0;
  int filearg = 1;
  FILE *outfile;
  char txtfile[MAX_STR];
  char namesfile[MAX_STR];
  int going;
  double smooth_mean[NUM_SIG];
  double smooth_var[NUM_SIG];
  double mean_decay = 0.99;
  double decay = 0.0;
  int active = 0;
  int first = 1;
  double val;

  for (i=0; i< NUM_SIG; i++)
    g_sig_info[i].real_signal=FALSE;

  while (filearg != argc - 2)
    {
      if ((*argv[filearg] != '-') || (filearg > argc))
	{
	  fprintf( stderr, "Usage: DAFFLOG [ -s <step> ] "
		   "[ -b <start time> ] [ -e <end time> ] "
		   "[ -d <decay> ] [ -m <mean decay> ] "
		   "<infile> <outfile root>\n" );
	  exit(1);
	}
      switch (argv[filearg][1])
	{
	case 's':
	  step = atoi( argv[filearg+1] );
	  break;
	case 'b':
	  start_time = atof( argv[filearg+1]);
	  break;
	case 'e':
	  end_time = atof( argv[filearg+1]);
	  break;
	case 'd':
	  decay = atof( argv[filearg+1]);
	  if ((decay >= 1) || (decay < 0))
	    {
	      fprintf( stderr, "decay constant must be in [0,1)\n" );
	      exit(1);
	    }
	  break;
	case 'm':
	  mean_decay = atof( argv[filearg+1]);
	  if ((decay >= 1) || (decay < 0))
	    {
	      fprintf( stderr, "decay constant must be in [0,1)\n" );
	      exit(1);
	    }
	  break;
	default:
	  fprintf( stderr, "Usage: DAFFLOG [ -s <step> ] "
		   "[ -b <start time> ] [ -e <end time> ] "
		   "[ -d <decay> ] [ -m <mean decay> ] "
		   "<infile> <outfile root>\n" );
	  exit(1);
	}
      filearg+=2;
    }

  strcat( strcpy( txtfile, argv[filearg+1] ), ".txt" );
  strcat( strcpy( namesfile, argv[filearg+1] ), ".names" );

  hf=daff_open(argv[filearg],&err);

  if (hf==NULL)
  {
     switch (err)
     {
     case DAFF_MEM_ERR: s="Out of memory error"; break;
     case DAFF_FNF_ERR: s="File not found"; break;
     case DAFF_RD_ERR : s="Read error"; break;
     case DAFF_ILL_ERR: s="Invalid DAFF file"; break;
     case DAFF_SEX_ERR: s="Incompatible DAFF format"; break;
     default:           s="Unknown DAFF error"; break;
     }
     fprintf(stderr,"\aDAFFLOG - error = %s\n",s);
     exit(1);
  }


  if (!(hf->header.flags&1))
  {
    daff_close(hf);
    fprintf(stderr,"\aDAFFLOG - file contains no data!\n");
    exit(1);
  }

  daff_enable_jmp(hf,here);  /* -- */
  if ((err=setjmp(here))!=0)
  {
    daff_close(hf);
    switch (err)
    {
    case DAFF_MEM_ERR: s="Out of memory error"; break;
    case DAFF_FNF_ERR: s="File not found"; break;
    case DAFF_RD_ERR : s="Read error"; break;
    case DAFF_ILL_ERR: s="Invalid DAFF file"; break;
    case DAFF_SEX_ERR: s="Incompatible DAFF format"; break;
    default:           s="Unknown DAFF error"; break;
    }
    fprintf(stderr,"\aDAFFLOG - error = %s\n",s);
    exit(1);
  }
  done=NO;
  do
  {
    ctx=daff_read_ctx(hf);
    if (ctx==0) daff_error(hf,DAFF_ILL_ERR);
    switch (ctx)
    {
    case ctx_HEADER:  /* Header - Informationen */
                      log_header(hf);
                       break;
    case ctx_ENTITY:  /* Signal-Definitionen */
                       log_entity(hf);
                       break;
    case ctx_DATADEF: /* Daten-Definition                   */
                      /* Wenn Dateiname, dann kein ctx_DATA */
                      if (log_datadef(hf)) done=YES;
                      break;
    case ctx_DATA:    /* Daten-Block    */
                      printf("<DATA>\n");
                      printf(" HeaderSize (implicit)=%ld\n",daff_get_pos(hf));
                      printf(" DataSize =");
		      g_startdata=daff_get_pos(hf);
                      datasize=daff_tag_size(hf);
                      if (datasize==0 || datasize==DAFF_DONT_KNOW)
                        printf(" to the end of the file\n");
                      else
                        printf(" %ld Bytes\n",datasize);
                      done=YES;
                      break;
    default:          /* Andere Kontexte - Tags ausgeben */
                      log_ctx(hf,0);
    }
  } while (!done);

  for ( i=1; i <= g_num_sigs; i++ )
    {
      printf( "signal %d: name '%s', units '%s'\nfreq: %f\noffset: %f\n"
	      "scale: %f\nmin: %f\nmax: %f\nfile offset: %d\n\n",
	      g_sig_info[i].signum, g_sig_info[i].name, g_sig_info[i].units,
	      g_sig_info[i].freq, g_sig_info[i].offset, g_sig_info[i].scale, 
	      g_sig_info[i].min, g_sig_info[i].max, g_sig_info[i].file_offset);
    }
  printf( "blocksize: %d\nstart of data: %d\n", g_blocksize,
	  g_startdata );

  going = 1;

  time = 0;

  outfile = fopen( namesfile, "w" );
  fprintf( outfile, "time\t" );
  for ( i=1; i <= g_num_sigs; i++ )
    {
      fprintf( outfile, "%s.iVActual\t", g_sig_info[i].name );
    }
  fclose( outfile );
  
  outfile = fopen( txtfile, "w" );
  going = fread(&data, 2, 1, hf->f);
  while (going)
    {
      if (time >= g_sig_info[0].freq * start_time)
	active = 1;

      if ((end_time) && (time > g_sig_info[0].freq * end_time))
	{
	  active = 0;
	  going = 0;
	}
      
      if ((time % step) != 0)
	active = 0;
      
      if (active)
	fprintf( outfile, "%f\t",
		 ((double) time * 1000.0)/g_sig_info[0].freq);

      for ( i=1; i <= g_num_sigs; i++ )
	{
	  val = g_sig_info[i].min + ( (g_sig_info[i].max - g_sig_info[i].min) *
				      ( (double) data / 65535.0 ) );

	  if (first)
	    {
	      smooth_mean[i] = val;
	      smooth_var[i] = 0;
	    }

	  smooth_mean[i] = ( smooth_mean[i] * mean_decay ) + 
	    (1.0 - mean_decay) * val;
	  smooth_var[i] = (smooth_var[i] * decay) +
	    (1.0 - decay) * (val - smooth_mean[i]) * (val - smooth_mean[i]);

	  if ((going) && (active))
	    fprintf( outfile, "%f\t", smooth_var[i] * 1000.0 );
	  going = fread(&data, 2, 1, hf->f);
	}
      first = 0;
      if ((going) && (active))
	fprintf(outfile, "\n");

      time++;
    }
  fclose( outfile );

  daff_close(hf);
  return 0;
}

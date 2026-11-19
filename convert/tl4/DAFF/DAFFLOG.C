/*-------------------------------------------------------------+
 | Erweitertes Beispiel-Programm zu Lesen von DAFF-Dateien     |
 +-------------------------------------------------------------+
 | (c) 1990 STEMMER Software Gesellschaft mbH  D-8039 Puchheim |
 | Alle Rechte vorbehalten                                     |
 +-------------------------------------------------------------+
 | LauffÑhig mit:                                              |
 |   Borland Turbo C V2.0                                      |
 |   Borland Turbo C++ V1.0                                    |
 |   Microsoft C V5.0, V5.1                                    |
 |   Microsoft C V6.0 DOS mode                                 |
 |   Microsoft C V6.0 Bound & OS/2 (Ersatz von gettime u.      |
 |                                getdate durch DosGetDateTime)|
 +-------------------------------------------------------------+
 | Zusammen mit DAFF_IO.C linken !                             |
 +-------------------------------------------------------------*/

#include <stdio.h>
#include "daff_io.h"

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
      printf("%*s<END CONTEXT>\n",level," ");
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
      printf("<END HEADER>\n");
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
         printf(" DATUM=%d.%d.%d %d:%d:%d\n",
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
          printf("  %s-Einheit=%s\n",s,ss);
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
         printf("  %s-Faktor=%lf\n",s,v);
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
          break;
    case tag_SFRQ:
         daff_read_dbl_tag(hf,&v);
         printf("  SamplingFrqz=%lf\n",v);
         break;
    case tag_SWEEP:
         daff_read_dbl_tag(hf,&v);
         printf("  SweepSize=%lf\n",v);
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
      printf("<END ENTITY>\n");
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
         break;
    case tag_DEMUX:   daff_read_int_tag(hf,&lv);
         printf("  DEMUX=%ld\n",lv);
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
  datafile=NO;
  printf("<DATADEF>\n");
  do
  {
    tid=daff_read_tag(hf);
    if (tid==DAFF_ECTX)
    {
      printf("<END DATADEF>\n");
      return datafile;
    }
    if (tid&DAFF_CONTEXT)
    {
      if ((tid&~DAFF_CONTEXT)==ctx_DATADEF)
      {
        id=TAG_(hf,num);
        printf(" <Signal-# = %d>\n",id);
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

  if (argc!=2)
  {
    fprintf(stderr,"\aAufruf: DAFFLOG dateiname\n");
    exit(1);
  }

  hf=daff_open(argv[1],&err);
  if (hf==NULL)
  {
     switch (err)
     {
     case DAFF_MEM_ERR: s="Speicher-Fehler"; break;
     case DAFF_FNF_ERR: s="Datei nicht gefunden"; break;
     case DAFF_RD_ERR : s="Lese-Fehler"; break;
     case DAFF_ILL_ERR: s="UngÅltige DAFF-Datei"; break;
     case DAFF_SEX_ERR: s="UnvertrÑgliches DAFF-Format"; break;
     default:           s="Unbekannter DAFF-Fehler"; break;
     }
     fprintf(stderr,"\aDAFFLOG - Fehler = %s\n",s);
     exit(1);
  }


  if (!(hf->header.flags&1))
  {
    daff_close(hf);
    fprintf(stderr,"\aDAFFLOG - Datei enthÑlt keine Daten!\n");
    exit(1);
  }

  daff_enable_jmp(hf,here);  /* -- */
  if ((err=setjmp(here))!=0)
  {
    daff_close(hf);
    switch (err)
    {
    case DAFF_MEM_ERR: s="Speicher-Fehler"; break;
    case DAFF_FNF_ERR: s="Datei nicht gefunden"; break;
    case DAFF_RD_ERR : s="Lese-Fehler"; break;
    case DAFF_ILL_ERR: s="UngÅltige DAFF-Datei"; break;
    case DAFF_SEX_ERR: s="UnvertrÑgliches DAFF-Format"; break;
    default:           s="Unbekannter DAFF-Fehler"; break;
    }
    fprintf(stderr,"\aDAFFLOG - Fehler = %s\n",s);
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
                      printf(" HeaderSize (implizit)=%ld\n",daff_get_pos(hf));
                      printf(" DataSize =");
                      datasize=daff_tag_size(hf);
                      if (datasize==0 || datasize==DAFF_DONT_KNOW)
                        printf(" bis zum Datei-Ende\n");
                      else
                        printf(" %ld Bytes\n",datasize);
                      done=YES;
                      break;
    default:          /* Andere Kontexte - Tags ausgeben */
                      log_ctx(hf,0);
    }
  } while (!done);
  daff_close(hf);
  return 0;
}

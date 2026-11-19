/*-------------------------------------------------------------+
 | Erweitertes Beispiel-Programm zu Lesen von DAFF-Dateien     |
 +-------------------------------------------------------------+
 | (c) 1990 STEMMER Software Gesellschaft mbH  D-8039 Puchheim |
 | Alle Rechte vorbehalten                                     |
 +-------------------------------------------------------------+
 +-------------------------------------------------------------+
 | Zusammen mit DAFF_IO.C linken !                             |
 +-------------------------------------------------------------*/
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>
#include "daff_io.h"
#include "Exception.hh"
#include "SignalInfo.hh"

#ifndef WIN32
#pragma implementation
#endif

#include "TL4Reader.hh"

static NRS::Recording::TL4Reader::TL4Reader sTL4R;

#define NUM_SIG 3

/*-------------------------------------------------------------+
 | Routinen zum Ausgeben von (unbekannten) Tags                |
 +-------------------------------------------------------------*/
void NRS::Recording::TL4Reader::hexdump(char *s,unsigned len)
{
  char line [17]; int j;
  for(j=0; len--; )
  {
    line[j]=(*s>=' ' && *s!=127) ? *s : '.';
#ifdef DEBUG
    printf("%02X ",*s++);
#endif
    if (++j == 16)
    {
      line[j]=0;
#ifdef DEBUG
      printf("  %s\n",line);
#endif
      j=0;
    }
  }
  if (j)
  {
    line[j]=0;
#ifdef DEBUG
    while (j++ <16 ) printf("   ");
    printf("  %s\n",line);
#endif
  }
}
/*------------------------------------------------------------+
 | log_tag(hf,level)  -- Unbekannten Tag ausgeben             |
 | -> hf = DAF - Filestruktur                                 |
 |    level = Fuer Einrueckung                                  |
 +------------------------------------------------------------*/

void NRS::Recording::TL4Reader::log_tag(int level)
{
  char *s; long lv; double dv;
  unsigned len;
#ifdef DEBUG
  printf("%*sTAG %u (%d) =",level," ",TAG_(iDAF,id),TAG_(iDAF,num));
#endif
  switch (TAG_(iDAF,dtyp))
  {
  case dt_STRUC:  case dt_SSTRUC:         /* unknown structure */
       len=TAG_(iDAF,size);
       daff_read_void_tag(iDAF,(void **)&s);
#ifdef DEBUG
       printf("\n");
#endif
       hexdump(s,len);
       free(s);
       break;
  case dt_STRING: case dt_SSTRING:        /* ascii chars       */
       daff_read_str_tag(iDAF,&s);
#ifdef DEBUG
       printf("\"%s\"\n",s);
#endif
       free(s);
       break;
  case dt_BYTE:   case dt_SBYTE:          /* (un)signed Byte   */
  case dt_WORD:   case dt_SWORD:          /* (un)signed Word   */
  case dt_LONG:   case dt_SLONG:          /* (un)signed Long   */
       daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
       printf("%ld\n",lv);
#endif
       break;
   case dt_REAL4:  case dt_SREAL4:         /* (un)signed Real*4 */
   case dt_REAL8:  case dt_SREAL8:         /* (un)signed Real*8 */
   case dt_BROKEN: case dt_SBROKEN:        /* (un)signed Real*6 */
        daff_read_dbl_tag(iDAF,&dv);
#ifdef DEBUG
        printf("%lf\n",dv);
#endif
        break;

   case dt_BIT8:   case dt_SBIT8:          /* (un)signed Bit8   */
   case dt_BIT16:  case dt_SBIT16:         /* (un)signed Bit16  */
        daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
        printf("%ld\n",lv);
#endif
        break;

   case dt_TEXTF:  case dt_STEXTF:         /* (un)signed TextF  */
        daff_read_dbl_tag(iDAF,&dv);
#ifdef DEBUG
        printf("%lf\n",dv);
#endif
        break;
   default:
#ifdef DEBUG
        printf("Unknown DataType\n");
#endif
        daff_skip_tag(iDAF);
   }
}
/*------------------------------------------------------------+
 | log_ctx(hf,level)  -- Unbekannten Kontext ausgeben         |
 | -> hf = DAF - Filestruktur                                 |
 |    level = Fuer Einrueckung                                  |
 +------------------------------------------------------------*/
void NRS::Recording::TL4Reader::log_ctx(int level)
{
  word tid;
  ++level;
  tid=TAG_(iDAF,id)&~DAFF_CONTEXT;
#ifdef DEBUG
  printf("%*s<CONTEXT id=%u, num=%d>\n",level," ",tid,TAG_(iDAF,num));
#endif
  do
  {
    tid=daff_read_tag(iDAF);
    if (tid==DAFF_ECTX)
    {
#ifdef DEBUG
      printf("%*s</CONTEXT>\n",level," ");
#endif
      return;
    }
    else if (tid&DAFF_CONTEXT)
    {
      log_ctx(level+1);
    }
    else log_tag(level);
  } while (1);
}

/*------------------------------------------------------------+
 | log_header(hf)  -- ctx_HEADER ausgeben                     |
 | -> hf = DAF - Filestruktur                                 |
 +------------------------------------------------------------*/

void NRS::Recording::TL4Reader::log_header()
{
  char *s;
  word tid;
  struct dattim d;
#ifdef DEBUG
  printf("<HEADER>\n");
#endif
  do
  {
    tid=daff_read_tag(iDAF);
    if (tid==DAFF_ECTX)
    {
#ifdef DEBUG
      printf("</HEADER>\n");
#endif
      return;
    }
    else if (tid&DAFF_CONTEXT)
    {
       log_ctx(1);
    }
    else switch (tid)
    {
    case tag_LABEL:
         daff_read_str_tag(iDAF,&s);
#ifdef DEBUG
         printf(" LABEL=%s\n",s);
#endif
         free(s);
         break;
    case tag_FTYP:
         daff_read_str_tag(iDAF,&s);
#ifdef DEBUG
         printf(" FTYP=%s\n",s);
#endif
         free(s);
         break;
    case tag_PROD:
         daff_read_str_tag(iDAF,&s);
#ifdef DEBUG
         printf(" PROD=%s\n",s);
#endif
         free(s);
         break;
    case tag_NAME:
         daff_read_str_tag(iDAF,&s);
#ifdef DEBUG
         printf(" NAME=%s\n",s);
#endif
         free(s);
         break;
    case tag_DATE:
         daff_read_date_tag(iDAF,&d);
#ifdef DEBUG
         printf(" DATE=%d.%d.%d %d:%02d:%02d\n",
            d.date.da_day,d.date.da_mon,d.date.da_year,
            d.time.ti_hour,d.time.ti_min,d.time.ti_sec);
#endif
         break;
    default:
         log_tag(1);
         break;
    }
  } while (1);
}

/*------------------------------------------------------------+
 | log_channnel(hf)  -- ctx_ENTITY ausgeben                   |
 | -> hf = DAF - Filestruktur                                 |
 +------------------------------------------------------------*/
void NRS::Recording::TL4Reader::log_channel()
{
  word tid; char *ss,*s; double v;
  do
  {
    tid=daff_read_tag(iDAF);
    if (tid==DAFF_ECTX) return;
    else if (tid&DAFF_CONTEXT)
    {
      log_ctx(1);
    }
    else switch (tid)
    {
    case tag_LABEL:
         daff_read_str_tag(iDAF,&s);
#ifdef DEBUG
         printf("  LABEL=%s\n",s);
#endif
         free(s);
         break;
    case tag_NAME:
         daff_read_str_tag(iDAF,&s);
#ifdef DEBUG
         printf("  NAME=%s\n",s);
#endif
	 iSigInfo.setName( s );
         free(s);
         break;
    case tag_SPEC:
         daff_read_str_tag(iDAF,&ss);
         switch (TAG_(iDAF,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
          }
#ifdef DEBUG
          printf("  %s-Units=%s\n",s,ss);
#endif
	  iSigInfo.setUnits( ss );
          free(ss);
          break;
    case tag_FACT:
         daff_read_dbl_tag(iDAF,&v);
         switch (TAG_(iDAF,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
         }
#ifdef DEBUG
         printf("  %s-Scaling-Factor=%lf\n",s,v);
#endif
	 if (finite(v))
	   iSigInfo.setScale( v );
	 else
	   iSigInfo.setScale( 1 );
         break;
    case tag_OFFS:
         daff_read_dbl_tag(iDAF,&v);
         switch (TAG_(iDAF,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
          }
#ifdef DEBUG
          printf("  %s-Offset=%lf\n",s,v);
#endif
	  iSigInfo.setOffset( v );
          break;
    case tag_MINV:
         daff_read_dbl_tag(iDAF,&v);
         switch (TAG_(iDAF,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
          }
#ifdef DEBUG
          printf("  %s-Minimum=%lf\n",s,v);
#endif
	  iSigInfo.setMin( v );
          break;
    case tag_MAXV:
         daff_read_dbl_tag(iDAF,&v);
         switch (TAG_(iDAF,num))
         {
           case 0: s="X";   break;
           case 1: s="AD";  break;
           case 2: s="Y";   break;
           default: s="??"; break;
          }
#ifdef DEBUG
          printf("  %s-Maximum=%lf\n",s,v);
#endif
	  iSigInfo.setMax( v );
          break;
    case tag_SFRQ:
         daff_read_dbl_tag(iDAF,&v);
#ifdef DEBUG
         printf("  SamplingFrq=%lf\n",v);
#endif
	 iSigInfo.setFreq( v );
         break;
    case tag_SWEEP:
         daff_read_dbl_tag(iDAF,&v);
#ifdef DEBUG
         printf("  SweepSize=%lf\n",v);
#endif
	 //	 _ABORT_( "Don't know what sweepsize is for, panic!\n\t" );
         break;
    default      : log_tag(2);
    }
  } while (1);
}
/*------------------------------------------------------------+
 | log_entity(hf)  -- ctx_ENTITY ausgeben                     |
 | -> hf = DAF - Filestruktur                                 |
 +------------------------------------------------------------*/
void NRS::Recording::TL4Reader::log_entity()
{
  word tid,id;
#ifdef DEBUG
  printf("<ENTITY>\n");
#endif
  do
  {
    tid=daff_read_tag(iDAF);
    if (tid==DAFF_ECTX)
    {
#ifdef DEBUG
      printf("</ENTITY>\n");
#endif
      return;
    }
    else if (!(tid&DAFF_CONTEXT))
    {
      log_tag(1);
    }
    else if ((tid&~DAFF_CONTEXT)!=ctx_ENTITY)
    {
      log_ctx(1);
    }
    else
    {
      id=TAG_(iDAF,num);
#ifdef DEBUG
      printf(" <Signal-# = %d> (0=default)\n",id);
#endif
      iSigInfo.newSignal( id, (id != 0) );
      iSigInfo.setSize( 2 );
      iSigInfo.setSigned( false );
      log_channel();
    }
  } while (1);
}
/*------------------------------------------------------------+
 | log_signal(hf)  -- ctx_DATADEF                             |
 | -> hf = DAF - Filestruktur                                 |
 +------------------------------------------------------------*/

void NRS::Recording::TL4Reader::log_sigdef()
{
  long lv;
  word tid;
  do
  {
    tid=daff_read_tag(iDAF);
    if (tid==DAFF_ECTX) return;
    else if (tid&DAFF_CONTEXT)
    {
      log_ctx(1);
    }
    else switch (tid)
    {
    case tag_BLKOFFS: daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
         printf("  BLKOFFS=%ld\n",lv);
#endif
	 _INFO_( iSigInfo.getCurrent() << " : " << lv );
	 if (lv != 0)
	   {
	     iSigInfo.setFileOffset( lv );
	   }
	 else
	   if (iSigInfo.getCurrent() != 0)
	     {
	       _WARN_( "Offset error for signal " << iSigInfo.getCurrent());
	       iSigInfo.setFileOffset( iSigInfo.
				       getFileOffset( iSigInfo.getCurrent()
						      - 1 ) +
				       iSigInfo.getSize( iSigInfo.getCurrent()
							 - 1 ) );
	     }
         break;
    case tag_DEMUX:   daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
         printf("  DEMUX=%ld\n",lv);
#endif
	 if (iSigInfo.getBlockSize() != lv)
	   {
	     _ABORT_( "demux doesn't match blocksize\n\t" );
	   }
         break;
    case tag_DATAT:   daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
         printf("  DATAT=%ld\n",lv);
#endif
         break;
    case tag_DTYPS:   daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
         printf("  DTYPS=%ld\n",lv);
#endif
         break;
    case tag_DARGS:   daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
         printf("  DARGS=%ld\n",lv);
#endif
         break;
    case tag_WPBLK:   daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
         printf("  WPBLK=%ld\n",lv);
#endif
         break;
    default: log_tag(2);
    }
  } while (1);
}

int  NRS::Recording::TL4Reader::log_datadef()
{
  word tid,id;
  char *ss;
  long lv;
  int datafile;

  datafile=NO;
#ifdef DEBUG
  printf("<DATADEF>\n");
#endif
  do
  {
    tid=daff_read_tag(iDAF);
    if (tid==DAFF_ECTX)
    {
#ifdef DEBUG
      printf("</DATADEF>\n");
#endif
      return datafile;
    }
    if (tid&DAFF_CONTEXT)
    {
      if ((tid&~DAFF_CONTEXT)==ctx_DATADEF)
      {
        id=TAG_(iDAF,num);
#ifdef DEBUG
        printf(" <Signal-# = %d>\n",id);
#endif
	try
	  {
	    iSigInfo.getSigRef( id );
	  }
	catch ( NRS::Base::Exception &e )
	  {
	    _RETHROW_( e );
	  }
	iSigInfo.setSigNum( id );
        log_sigdef();
      }
      else log_ctx(2);
    }
    else switch (tid)
    {
    case tag_SOURCE:   /* Datei-Name fuer Daten-File */
         daff_read_str_tag(iDAF,&ss);
#ifdef DEBUG
         printf(" SOURCE = %s\n",ss);
#endif
         free(ss);
         datafile=YES;
         break;
    case tag_SRCOFFS:  /* Offset im Daten-File */
         daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
         printf(" SRCOFFS=%ld\n",lv);
#endif
         break;
    case tag_BLKSIZE: /* Block-Groesse (falls Datei geblockt) */
         daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
         printf(" BLKSIZE=%ld\n",lv);
#endif
	 iSigInfo.setBlockSize( lv );
         break;
    case tag_SRCTYP :  /* TYP=1 fuer ASCII-Dateien */
         daff_read_int_tag(iDAF,&lv);
#ifdef DEBUG
         printf(" SRCTYP=%s\n", (lv)? "ASCII" : "BINARY");
#endif
         break;
    default:
      log_tag(3);
      break;
    }
  } while (1);
}

/*-------------------------------------------------------------+
 | Haupt-Programm DAFFLOG.C                                    |
 +-------------------------------------------------------------+
 | Aufruf mit DAFFLOG dateiname                                |
 +-------------------------------------------------------------*/

bool  NRS::Recording::TL4Reader::openFile( std::string aFile )
{
  int err;
  jmp_buf here;
  word ctx;
  int done;
  long datasize;
  char *s;

  iDAF = daff_open( aFile.c_str(), &err );

  if ( iDAF == NULL )
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
     _ERROR_( "\aDAFFLOG - error = " << s << "\n\t" );
     close();
     return false;
  }


  if (!(iDAF->header.flags&1))
  {
    daff_close(iDAF);
    _ERROR_( "DAFFLOG - file contains no data!\n\t" );
    close();
    return false;
  }

  daff_enable_jmp(iDAF,here);  /* -- */
  if ((err=setjmp(here))!=0)
  {
    daff_close(iDAF);
    switch (err)
    {
    case DAFF_MEM_ERR: s="Out of memory error"; break;
    case DAFF_FNF_ERR: s="File not found"; break;
    case DAFF_RD_ERR : s="Read error"; break;
    case DAFF_ILL_ERR: s="Invalid DAFF file"; break;
    case DAFF_SEX_ERR: s="Incompatible DAFF format"; break;
    default:           s="Unknown DAFF error"; break;
    }
    _ERROR_( "DAFFLOG - error = " << s << "\n\t" );
    close();
    return false;
  }
  done=NO;
  do
  {
    ctx=daff_read_ctx(iDAF);

    if (ctx==0)
      daff_error(iDAF,DAFF_ILL_ERR);

    switch (ctx)
    {
    case ctx_HEADER:  /* Header - Informationen */
                      log_header();
                       break;
    case ctx_ENTITY:  /* Signal-Definitionen */
                       log_entity();
                       break;
    case ctx_DATADEF: /* Daten-Definition                   */
                      /* Wenn Dateiname, dann kein ctx_DATA */
                      if (log_datadef()) done=YES;
                      break;
    case ctx_DATA:    /* Daten-Block    */
#ifdef DEBUG
                      printf("<DATA>\n");
                      printf(" HeaderSize (implicit)=%ld\n",daff_get_pos(iDAF));
                      printf(" DataSize =");
#endif
		      iSigInfo.setStartData( daff_get_pos( iDAF ) );
                      datasize = daff_tag_size( iDAF );
#ifdef DEBUG
                      if (datasize==0 || datasize==(long)DAFF_DONT_KNOW)
			printf(" to the end of the file\n");
                      else
			printf(" %ld Bytes\n",datasize);
#endif
		      done=YES;
                      break;
    default:          /* Andere Kontexte - Tags ausgeben */
      log_ctx(0);
    }
  } while (!done);

  
  iIFS.open( aFile.c_str() );
  char *aTmp = new char[ iSigInfo.getStartData() ];
  iIFS.read( aTmp, iSigInfo.getStartData() );
  delete[] aTmp;
  return true;
}

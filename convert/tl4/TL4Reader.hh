#ifndef _TL4_READER_HH
#define _TL4_READER_HH

#include <string>
#include <iostream>
#include "Exception.hh"
#include "SignalInfo.hh"
#include "Reader.hh"
#include "daff_io.h"

#ifndef WIN32
#pragma interface
#endif

namespace NRS
{
  namespace Recording
  {

    class TL4Reader : public Reader
    {
    public:
      TL4Reader() : Reader( "TL4Reader" ), iDAF( NULL )
      {
      }

      virtual ~TL4Reader()
      {
      }

      virtual bool openFile( std::string aFile );

      virtual void close()
      {
	if (iDAF)
	  {
	    daff_close( iDAF );
	    iDAF = NULL;
	  }
	Reader::close();
      }

    private:
      
      void hexdump( char *s, unsigned len );
      void log_tag(int level);
      void log_ctx(int level);
      void log_header();
      void log_channel();
      void log_entity();
      void log_sigdef();
      int log_datadef();

      Daf *iDAF;
    };
  }
}

#endif //ndef _TL4_READER_HH

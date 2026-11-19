#ifndef _TL3_READER_HH
#define _TL3_READER_HH

#include <string>
#include <iostream>
#include "Exception.hh"
#include "SignalInfo.hh"
#include "Reader.hh"
#include "daff_io.h"

#ifdef WIN32
#pragma interface
#endif

namespace NRS
{
  namespace Recording
  {

    class TL3Reader : public Reader
    {
    public:
      TL3Reader() : Reader( "TL3Reader" )
      {
      }

      virtual ~TL3Reader()
      {
      }

      virtual bool openFile( std::string aFile );

    };
  }
}

#endif //ndef _TL3_READER_HH

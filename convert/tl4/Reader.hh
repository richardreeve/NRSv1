#ifndef _READER_HH
#define _READER_HH

#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include "Exception.hh"
#include "SignalInfo.hh"

#ifndef WIN32
#pragma interface
#endif

namespace NRS
{
  namespace Recording
  {
    class Reader
    {
    public:
      Reader( std::string aType );

      virtual ~Reader()
      {
	close();
      }

      std::string getType()
      {
	return iType;
      }

      NRS::Recording::SignalInfo getSigInfo()
      {
	return iSigInfo;
      }

      virtual bool openFile( std::string aFile ) = 0;

      virtual int getNext( double *aPtr, int maxNum );
      
      virtual void close()
      {
	if (iIFS)
	  iIFS.close();
      }

    protected:

      std::string iType;
      NRS::Recording::SignalInfo iSigInfo;
      std::ifstream iIFS;

    };
  }
}

#endif //ndef _READER_HH

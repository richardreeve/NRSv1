#ifndef _READER_HH
#define _READER_HH

#include <string>
#include <iostream>
#include "Exception.hh"

#pragma interface

namespace NRS
{
  namespace Recording
  {
    class Converter
    {
    public:
      Converter( std::string aType ) : iType( aType )
      {
      }

      virtual ~Converter()
      {
      }

      std::string getType()
      {
	return iType;
      }

      virtual bool checkFile( std::string aFile ) = 0;

      
    };
  }
}

#endif //ndef _READER_HH

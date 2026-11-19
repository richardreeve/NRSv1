#ifndef _MANAGER_HH
#define _MANAGER_HH

#include <string>
#include <list>
#include <iostream>
#include "Exception.hh"
#include "Reader.hh"

#ifdef WIN32
#pragma interface
#endif

namespace NRS
{
  namespace Recording
  {

    class Manager
    {
    public:
      static Manager &getManager()
      {
	static Manager sM;
	return sM;
      }
      void addReader( Reader *aConv )
      {
	iReaderList.push_back( aConv );
      }

      Reader *openFile( std::string aFile )
      {
	for ( std::list< Reader* >::iterator anIter =
		iReaderList.begin();
	      anIter != iReaderList.end();
	      anIter++ )
	  {
	    if ((*anIter)->openFile( aFile ))
	      return (*anIter);
	  }
	return NULL;
      }

    private:
      std::list< Reader* > iReaderList;
    };

  }
}

#endif //ndef _MANAGER_HH

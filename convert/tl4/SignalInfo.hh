#ifndef _SIGNAL_INFO_HH
#define _SIGNAL_INFO_HH

#include <string>
#include <vector>
#include <map>
#include "Exception.hh"

#ifndef WIN32
#pragma interface
#endif
#define DEBUG
namespace NRS
{
  namespace Recording
  {
    class SignalInfo
    {
    public:
      SignalInfo();

      void resize( int num );

      void newSignal( int id, bool isReal = true );

      int getNumSignals()
      {
	return iNumSignals;
      }

      int getSigNum( int ref )
      {
	return iSigNum[ ref ];
      }

      int getSigRef( int id )
      {
	if (iSigMap.find( id ) == iSigMap.end())
	  throw NRS::Base::Exception( "No such id\n\t", _FL_ );

	return iSigMap[ id ];
      }
      
      bool setSigNum( int id )
      {
	if (iSigMap.find( id ) == iSigMap.end())
	  return false;

	iCurrent = iSigMap[ id ];

	return true;
      }
      
      bool isReal( int ref )
      {
	return iRealSignal[ ref ];
      }

      void setName( std::string aName )
      {
	iName[ iCurrent ] = aName;
      }

      std::string getName( int ref )
      {
	return iName[ ref ];
      }
      
      void setUnits( std::string aUnits )
      {
	iUnits[ iCurrent ] = aUnits;
      }

      std::string getUnits( int ref )
      {
	return iUnits[ ref ];
      }
      
      void setFreq( double aFreq )
      {
	iFreq[ iCurrent ] = aFreq;
      }

      double getFreq( int ref )
      {
	return iFreq[ ref ];
      }
      
      void setOffset( double aOffset )
      {
	iOffset[ iCurrent ] = aOffset;
      }

      double getOffset( int ref )
      {
	return iOffset[ ref ];
      }
      
      void setScale( double aScale )
      {
	iScale[ iCurrent ] = aScale;
      }

      double getScale( int ref )
      {
	return iScale[ ref ];
      }
      
      void setMin( double aMin )
      {
	iMin[ iCurrent ] = aMin;
      }

      double getMin( int ref )
      {
	return iMin[ ref ];
      }
      
      void setMax( double aMax )
      {
	iMax[ iCurrent ] = aMax;
      }

      double getMax( int ref )
      {
	return iMax[ ref ];
      }
      
      void setFileOffset( int aFileOffset )
      {
	iFileOffset[ iCurrent ] = aFileOffset;
      }

      int getFileOffset( int ref )
      {
	return iFileOffset[ ref ];
      }

      void setSize( int size )
      {
	iSize[ iCurrent ] = size;
      }

      int getSize( int ref )
      {
	return iSize[ ref ];
      }

      void setSigned( bool sign )
      {
	iSigned[ iCurrent ] = sign;
      }

      bool getSigned( int ref )
      {
	return iSigned[ ref ];
      }

      void setBlockSize( int bs )
      {
	iBlockSize = bs;
      }

      int getBlockSize()
      {
	return iBlockSize;
      }

      void setStartData( int bs )
      {
	iStartData = bs;
      }

      int getStartData()
      {
	return iStartData;
      }

      void setDate( int year, int month, int day, int hour, int minute )
      {
	iYear = year;
	iMonth = month;
	iDay = day;
	iHour = hour;
	iMinute = minute;
      }

      void setBuffLen( int bl )
      {
	iBuffLen = bl;
      }

      int getBuffLen()
      {
	return iBuffLen;
      }

      void externalise( std::ostream& anOStream );

      int getCurrent()
      {
	return iCurrent;
      }

    private:
      int iCurrent;
      int iNumSignals;
      bool iHasDefaults;
      int iDefaults;
      int iBlockSize;
      int iStartData;

      int iYear;
      int iMonth;
      int iDay;
      int iHour;
      int iMinute;

      int iBuffLen;

      std::vector< bool > iRealSignal;
      std::vector< int > iSigNum;
      std::map< int, int > iSigMap;
      std::vector< std::string > iName;
      std::vector< std::string > iUnits;
      std::vector< int > iSize;
      std::vector< bool > iSigned;
      std::vector< double > iFreq;
      std::vector< double > iOffset;
      std::vector< double > iScale;
      std::vector< double > iMin;
      std::vector< double > iMax;
      std::vector< int > iFileOffset;
    };
  }
}

#endif //ndef _SIGNAL_INFO_HH

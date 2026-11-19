#ifndef _SIGNAL_INFO_HH
#define _SIGNAL_INFO_HH

#include <string>
#include <vector>

#pragma interface

namespace NRS
{
  namespace Recording
  {
    class SignalInfo
    {
      SignalInfo() : iNumSignals( 0 ), iHasDefaults( false )
      {
	resize( iNumSignals );
      }

      void resize( int num )
      {
	iRealSignal.resize( num );
	iSigNum.resize( num );
	iName.resize( num );
	iUnits.resize( num );
	iFreq.resize( num );
	iOffset.resize( num );
	iScale.resize( num );
	iMin.resize( num );
	iMax.resize( num );
	iFileOffset.resize( num );
	iNumSignals = num;
      }

    private:
      int iNumSignals;
      bool iHasDefaults;
      int iDefaults;

      std::vector< int > iSigNum;
      std::map< int, int > iSigMap;
      std::vector< std::string > iName;
      std::vector< std::string > iUnits;
      std::vector< double > iFreq, iOffset, iScale, iMin, iMax;
      std::vector< int > iFileOffset;
    };
  }
}

#endif //ndef _SIGNAL_INFO_HH

#include <iostream>

#ifndef WIN32
#pragma implementation
#endif
#include "SignalInfo.hh"

NRS::Recording::SignalInfo::SignalInfo() : iNumSignals( 0 ),
					   iHasDefaults( false ),
					   iBlockSize( 0 ), iStartData( 0 ),
					   iYear( -1 ), iBuffLen( -1 )
{
  resize( iNumSignals );
}

void NRS::Recording::SignalInfo::resize( int num )
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
  iSize.resize( num );
  iSigned.resize( num );
  iNumSignals = num;
}

void NRS::Recording::SignalInfo::newSignal( int id, bool isReal )
{
  int ref = iNumSignals;
  resize( iNumSignals + 1 );
  iSigMap[ id ] = ref;
  iSigNum[ ref ] = id;
  iRealSignal[ ref ] = isReal;
  iCurrent = ref;
  if (iHasDefaults)
    {
      if (!isReal)
	{
	  throw NRS::Base::Exception( "Creating default "
				      "signal properties "
				      "when already created\n\t", _FL_ );
	}
      iFreq[ ref ] = iFreq[ iDefaults ];
      iOffset[ ref ] = iOffset[ iDefaults ];
      iScale[ ref ] = iScale[ iDefaults ];
      iMin[ ref ] = iMin[ iDefaults ];
      iMax[ ref ] = iMax[ iDefaults ];
      iSigned[ ref ] = iSigned[ iDefaults ];
    }
  else
    {
      if (!isReal)
	{
	  iHasDefaults = true;
	  iDefaults = ref;
	}
      iFreq[ ref ] = 1;
      iOffset[ ref ] = 0;
      iScale[ ref ] = 1;
      iMin[ ref ] = -32768;
      iMax[ ref ] = 32767;
      iSigned[ ref ] = true;
    }
}


void NRS::Recording::SignalInfo::externalise( std::ostream& anOStream )
{
  if (iYear != -1)
    {
      anOStream << "Recording from " << iYear << "-" << iMonth
		<< "-" << iDay << " " << iHour << ":" << iMinute
		<< std::endl;
    }
  
  for ( int i=0; i < getNumSignals(); i++ )
    {
      if ( isReal( i ) )
	{
	  anOStream << "signal " << getSigNum( i ) << ":"
		    << " name '" << getName( i ) << "',"
		    << " units '" << getUnits( i ) << "'\n"
		    << "freq: " << getFreq( i ) << "\n"
		    << "offset: " << getOffset( i ) << "\n"
		    << "scale: " << getScale( i ) << "\n"
		    << "min: " << getMin( i ) << "\n"
		    << "max: " << getMax( i ) << "\n"
		    << "size: " << getSize( i ) << "\n"
		    << "signed: " << getSigned( i ) << "\n"
		    << "file offset: " << getFileOffset( i )
		    << std::endl << std::endl;
	}
    }
  anOStream << "blocksize: " << getBlockSize() << "\n"
	    << "start of data: " << getStartData() << std::endl;
  
  if (getBuffLen() != -1)
    {
      anOStream << "Buffer length: " << getBuffLen() << std::endl;
    }
  
}


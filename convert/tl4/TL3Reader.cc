#include <fstream>
#include "Exception.hh"
#include "SignalInfo.hh"
#include <cstdlib>

#ifdef WIN32
#pragma implementation
#endif

#include "TL3Reader.hh"

static NRS::Recording::TL3Reader::TL3Reader sTL3R;

bool  NRS::Recording::TL3Reader::openFile( std::string aFile )
{
  int i;

  iIFS.open( aFile.c_str() );

  if (!iIFS)
    {
      _ERROR_( "File not found by TL3Reader\n\t" );
      return false;
    }

  char ch;
  char anArr[256];
  iIFS.get(ch);
  iIFS.read( anArr, 8 );
  anArr[(int)ch] = 0;
  std::string aS( anArr );
  if ((ch != 8) || (aS != "TurboLab"))
    {
      iIFS.close();
      _ERROR_( "File not in TL3 format\n\t" );
      return false;
    }
  
  iIFS.get(ch);

  int numChannels = ch;

  for ( i = 0; i < 16; i++ )
    {
      iIFS.get(ch);
      if (i < numChannels)
	{
	  iSigInfo.newSignal( i );
	  iSigInfo.setScale( 1.0 / (double) ch );
	  iSigInfo.setFileOffset( 2 * i );
	  iSigInfo.setSize( 2 );
	  iSigInfo.setSigned( true );
	}
    }
  for ( i = 0; i < 16; i++ )
    {
      iIFS.get(ch);
      int j = 0;
      for ( int k = 0; k < 10; k++)
	{
	  iIFS.read(anArr+j, 1 );
	  if (anArr[j]!=' ')
	    j++;
	  else
	    ch--;
	}
      anArr[(int)ch] = 0;
      if ( iSigInfo.setSigNum( i ) )
	iSigInfo.setName( anArr );
    }

  iIFS.get(ch);
  iIFS.read(anArr, 10 );
  anArr[(int)ch] = 0;
  double freq = atof( anArr );

  iIFS.get(ch);
  iIFS.read(anArr, 10 );
  anArr[(int)ch] = 0;
  double freqTot = atof( anArr );

  if ((freqTot/freq - numChannels)*(freqTot/freq - numChannels) > 0.1)
    {
      _ERROR_( "Sampling frequencies do not match number of channels\n\t" );
      return false;
    }

  for ( i = 0; i < numChannels; i++ )
    {
      iSigInfo.setSigNum( i );
      iSigInfo.setFreq( freq );
    }
 
  iIFS.get(ch);
  iIFS.read(anArr, 9 );
  anArr[(int)ch] = 0;
  iSigInfo.setBuffLen( atoi( anArr ) );

  short year;
  char month;
  char day;
  char hour;
  char minute;

  iIFS.read( (char *)&year, 2);
  iIFS.get(month);
  iIFS.get(day);
  iIFS.get(hour);
  iIFS.get(minute);
  
  iSigInfo.setDate( year, month, day, hour, minute );
  
  iIFS.get(ch);
  iIFS.read(anArr, 64 );
  anArr[(int)ch] = 0;

  _INFO_( "Label: '" << anArr << "'\n\t" );

  iIFS.get(ch);
  
  _INFO_( "Board = " << (int) ch << "\n\t" );

  iIFS.read( (char *)&year, 2);
  
  //  _INFO_( "Base = " << year << "\n\t" );

  iIFS.get(ch);
  
  //  _INFO_( "DMA Channel 1 = " << (int) ch << "\n\t" );

  iIFS.get(ch);
  
  //  _INFO_( "DMA Channel 2 = " << (int) ch << "\n\t" );

  iIFS.get(ch);
  
  //  _INFO_( "Interrupt level = " << (int) ch << "\n\t" );

  iIFS.get(ch);
  
  _INFO_( "Input range = " << (int) ch << "\n\t" );

  iIFS.get(ch);
  
  _INFO_( "Coding = " << (int) ch << "\n\t" );

  iIFS.get(ch);
  
  _INFO_( "Input Mode = " << (int) ch << "\n\t" );

  iIFS.read( anArr, 198 );

  iSigInfo.setBlockSize( 2 * numChannels );
  iSigInfo.setStartData( 512 );
  return true;
}

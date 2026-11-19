#include <string>

#ifndef WIN32
#pragma implementation
#endif

#include <cmath>
#include "Reader.hh"
#include "Manager.hh"
#include "SignalInfo.hh"

NRS::Recording::Reader::Reader( std::string aType ) : iType( aType )
{
  NRS::Recording::Manager::getManager().addReader( this );
}

int NRS::Recording::Reader::getNext( double *anArray, int maxNum )
{
  static char *charlist = NULL;
  static int size = 0;
  static bool firstTime = true;

  double *aPtr = anArray;
  double val;

  if (size != iSigInfo.getBlockSize())
    {
      delete[] charlist;
      size = iSigInfo.getBlockSize();
      charlist = new char[ size ];
    }

  if (!iIFS)
    return 0;

  iIFS.read( charlist, size );

  for ( int i=0; i < iSigInfo.getNumSignals(); i++ )
    {
      if (firstTime && ((!finite(iSigInfo.getMin( i ))) ||
			(!finite(iSigInfo.getMax( i )))))
	{
	  _WARN_( "Signal " << i << " has illegal minimum and maximum\n\t" );
	}
      if ( iSigInfo.isReal( i ) )
	{
	  if (aPtr - anArray >= maxNum)
	    { // we've run out of space - bail out
	      return maxNum;
	    }
	  
	  if (iSigInfo.getSigned( i ))
	    {
	      switch (iSigInfo.getSize( i ))
		{
		case 2:
		  val = ( ( (double) 
			    *( (short *) 
			       &charlist[iSigInfo.getFileOffset( i )] ) ) /
			  65536.0 );
		  break;
		case 4:
		  val = ( ( (double)
			    *((int *) 
			      &charlist[iSigInfo.getFileOffset( i )] ) ) /
			  4294967296.0 );
		  break;
		default:
		  _ABORT_( "Only (unsigned) short and int are handled\n\t" );
		  break;
		}
	    }
	  else
	    {
	      switch (iSigInfo.getSize( i ))
		{
		case 2:
		  val = ( ( (double) 
			    *( (unsigned short *) 
			       &charlist[iSigInfo.getFileOffset( i )] ) ) /
			  65536.0 );
		  break;
		case 4:
		  val = ( ( (double) 
			    *( (unsigned int *) 
			       &charlist[iSigInfo.getFileOffset( i )] ) ) /
			  4294967296.0 );
		  break;
		default:
		  _ABORT_( "Only (unsigned) short and int are handled\n\t" );
		  break;
		}
	    }


	  if ( iSigInfo.getSigned( i ) )
	    {
	      if (finite(iSigInfo.getMin( i )) || finite(iSigInfo.getMax( i )))
		{
		  *aPtr = iSigInfo.getScale( i ) * ( ( iSigInfo.getMax( i ) +
						       1 +
						       iSigInfo.getMin( i ) ) /
						     2.0 +
						     ( (iSigInfo.getMax( i ) -
							iSigInfo.getMin( i ) )
						       * val ) );
		}
	      else
		*aPtr = iSigInfo.getScale( i ) * val;
	    }
	  else
	    {
	      if (finite(iSigInfo.getMin( i )) || finite(iSigInfo.getMax( i )))
		{
		  *aPtr = iSigInfo.getScale( i ) * ( iSigInfo.getMin( i ) +
						     ( (iSigInfo.getMax( i ) -
							iSigInfo.getMin( i ) )
						       * val ) );
		}
	      else
		*aPtr = iSigInfo.getScale( i ) * val;
	    }
	  aPtr++;
	}
    }
  firstTime = false;
  return (aPtr - anArray);
}

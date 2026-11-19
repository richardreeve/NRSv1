#include <valarray>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Exception.hh"
#include "SignalInfo.hh"
#include "Reader.hh"
#include "Manager.hh"

#define NUM_SIG 3

int main(int argc,char **argv)
{
  int i;
  long time;
  int step = 1;
  double start_time = 0;
  double end_time = 0;
  FILE *outfile;
  std::string txtfile;
  std::string namesfile;
  std::string delaysfile;
  int numArray;
  bool do_decay = false;
  double decay = 0.0;
  bool limits = false;
  bool find_spikes = false;
  double spike_size = 0;
  bool do_round = false;
  bool calc_spike_delays = false;
  float delay_time = 0;
  std::string neuronName = "Neuron";
  int interval = 50000;

  while ((argc > 1) && (argv[1][0] == '-'))
    {
      switch (argv[1][1])
	{
	case 's': // Timestep
	  step = atoi( argv[2] );
	  argv+=2;
	  argc-=2;
	  break;
	case 'l': // collapse to [0,1]
	  limits = true;
	  argv++;
	  argc--;
	  break;
	case 'f': // Find spikes in neuron
	  find_spikes = true;
	  spike_size = atof( argv[2] );
	  argv+=2;
	  argc-=2;
	  break;
	case 'b': // first timetick to translate
	  start_time = atof( argv[2] );
	  argv+=2;
	  argc-=2;
	  break;
	case 'e': // final timetick to translate
	  end_time = atof( argv[2] );
	  argv+=2;
	  argc-=2;
	  break;
	case 'd': // differentiate neural signal (decay rate perhaps 0.9)
	  do_decay = true;
	  decay = atof( argv[2] );
	  if ((decay >= 1) || (decay < 0))
	    {
	      _ABORT_( "decay constant must be in [0,1)\n\t" );
	    }
	  argv+=2;
	  argc-=2;
	  break;
	case 'i': // averaging interval for normalisation (perhaps 50000)
	  interval = atoi( argv[2] );
	  argv+=2;
	  argc-=2;
	  break;	  
	case 't': // amount of recovery time for first spike on sound
	  calc_spike_delays = true;
	  delay_time = atof( argv[2] );
	  argv+=2;
	  argc-=2;
	  break;
	case 'r': // round sound to 0, 1
	  do_round = true;
	  limits=true;
	  argv++;
	  argc--;
	  break;
	case 'n': // Name of neuron if not Neuron*
	  neuronName = argv[2];
	  argv+=2;
	  argc-=2;
	  break;
	default:
	  std::cerr << "Unrecognised option: " << argv[1] << std::endl;
	case 'h': // help
	  std::cerr << "Usage: converter [ -s <step> ]"
		    << " [ -l ] [ -f <spike size> ]"
		    << " [ -b <start time> ] [ -e <end time> ]"
		    << " [ -d <decay> ]  [ -r ] [ -t <delay time> ]"
		    << " [ -n <neuronName> ]"
		    << " <infile> <outfile root>" << std::endl;
	  exit(0);
	  break;
	}
    }

  if (argc < 3)
    {
	  std::cerr << "Usage: converter [ -s <step> ]"
		    << " [ -l ] [ -f <spike size> ]"
		    << " [ -b <start time> ] [ -e <end time> ]"
		    << " [ -d <decay> ] [ -r ] [ -t <delay time> ]"
		    << " <infile> <outfile root>" << std::endl;
	  exit(0);
    }

  txtfile = argv[2];
  txtfile += ".txt";
  namesfile = argv[2];
  namesfile += ".names";
  delaysfile = argv[2];
  delaysfile += ".delays";

  NRS::Recording::Reader *aReader = 
    NRS::Recording::Manager::getManager().openFile( argv[1] );

  if (!aReader)
    {
      _ABORT_( "No reader found\n\t" );
    }

  NRS::Recording::SignalInfo aSigInfo = aReader->getSigInfo();

  aSigInfo.externalise( std::cout );

  struct stat buf;

  stat( argv[1], &buf );

  int tot = aSigInfo.getBlockSize();

  long filesize = buf.st_size - aSigInfo.getStartData();
  long valSize = filesize/tot;

  std::cout << "Filesize: " << filesize << "\n"
	    << "packet size: " << tot << "\n"
	    << "entries: " << valSize << std::endl;

  long num = 1;
  for (i = 0; i < aSigInfo.getNumSignals(); i++)
    if (aSigInfo.isReal(i))
      num++;

  std::valarray<float> *values = new std::valarray<float>[ num ];
  for (i=0; i<num; i++)
    values[i].resize( valSize );

  time = 0;

  outfile = fopen( namesfile.c_str(), "w" );
  fprintf( outfile, "time\t" );
  for ( i=0; i < aSigInfo.getNumSignals(); i++ )
    {
      if ( aSigInfo.isReal( i ) )
	{
	  fprintf( outfile, "%s.iVActual\t", aSigInfo.getName( i ).c_str() );
	  if (find_spikes &&
	      (aSigInfo.getName(i).find( neuronName ) != std::string::npos))
	    {
	      fprintf( outfile, "%s.iSpikes\t",
		       aSigInfo.getName( i ).c_str() );
	      fprintf( outfile, "%s.iAnalogue\t",
		       aSigInfo.getName( i ).c_str() );
	    }
	  if (do_round &&
	      (aSigInfo.getName(i).find( "SdStim" ) != std::string::npos))
	    fprintf( outfile, "%s_rounded.iVActual\t",
		     aSigInfo.getName( i ).c_str() );
	}
    }
  fclose( outfile );
  
  double *anArray = new double[ aSigInfo.getNumSignals() ];
  double *aPtr = anArray;
  numArray = aReader->getNext( anArray, aSigInfo.getNumSignals());

  int col;
  long pos = 0;
  while (numArray > 0)
    {
      
      if ((((double) time * 1000.0)/aSigInfo.getFreq( 0 ) >= start_time) &&
	  ((!end_time) ||
	   (((double) time * 1000.0)/aSigInfo.getFreq( 0 ) <= end_time)) &&
	  (time % step == 0))
	{
	  values[0][pos] = ((double) time * 1000.0)/aSigInfo.getFreq( 0 );
	  col = 1;
	  for ( i=0; i < aSigInfo.getNumSignals(); i++ )
	    {
	      if ( aSigInfo.isReal( i ) )
		{
		  if (numArray > 0)
		    {
		      values[col][pos] = *(aPtr++);
		    }
		  col++;
		}
	    }
	  pos++;
	}
      else 
	{
	  valSize--;
	}
      numArray = aReader->getNext( anArray, aSigInfo.getNumSignals() );
      time++;
      aPtr = anArray;
    }
  for (i = 0; i<num; i++)
    {
      std::valarray<float> tmp = values[i][std::slice(0, valSize, 1)];
      values[i].resize(valSize);
      values[i] = tmp;
    }

  std::cout << "Real entries: " << valSize << std::endl;

  std::cout << "Limits: " << std::boolalpha << limits << std::endl;
  unsigned int j;
  std::valarray< float > roundedL(valSize);
  std::valarray< float > roundedR(valSize);
  int r_num = -1;
  int r_pos = -1;
  int l_num = -1;
  int l_pos = -1;
  std::cout << "num: " << num << std::endl;
  pos = 1;
  for (i = 0; i < aSigInfo.getNumSignals(); i++)
    {
      if (aSigInfo.isReal(i))
	{
	  std::cout << pos++ << ": " << aSigInfo.getName(i) << std::endl;
	}
      
    }
  pos = 1;
  for (i = 0; i < aSigInfo.getNumSignals(); i++)
    if (aSigInfo.isReal(i))
      {
	if (aSigInfo.getName(i) == "RSdStim" )
	  {
	    r_num = i;
	    r_pos = pos;
	  }
	else if (aSigInfo.getName(i) == "LSdStim" )
	  {
	    l_num = i;
	    l_pos = pos;
	  }
	pos++;
      }
  std::cout << "Limits: " << std::boolalpha << limits << std::endl;

  if (limits)
    {
      float bot=1000000;
      float top=-1000000;
      pos = 1;
      for (i = 0; i < aSigInfo.getNumSignals(); i++)
	{
	  if (aSigInfo.isReal(i))
	    {
	      if (aSigInfo.getName(i).find( "SdStim" ) != std::string::npos)
		{
		  bot = values[pos].min() <? bot;
		  top = values[pos].max() >? top;
		}
	      pos++;
	    }
	} 
      pos = 1;
      for (i = 0; i < aSigInfo.getNumSignals(); i++)
	{
	  if (aSigInfo.isReal(i))
	    {
	      if (aSigInfo.getName(i).find( "SdStim" ) != std::string::npos)
		{
		  values[pos] -= bot;
		  values[pos] /= top-bot;
		  if (do_round)
		    {
		      if (r_num == i)
			{
			  for (j=0; j<roundedR.size(); j++)
			    if (values[pos][j] < 0.5)
			      roundedR[j] = 0;
			    else
			      roundedR[j] = 1;
			}
		      else // l_num == i
			{
			  for (j=0; j<roundedR.size(); j++)
			    if (values[pos][j] < 0.5)
			      roundedL[j] = 0;
			    else
			      roundedL[j] = 1;
			}
		    }
		}
	      else
		{
		  values[pos] -= values[pos].min();
		  values[pos] /= values[pos].max();
		}
	      pos++;
	    }
	}
    }

  std::valarray< int > spikes(valSize);
  std::valarray< float > analogue(valSize);
  spikes = 0;
  analogue = 0;

  int neur_num = -1;
  int neur_pos = -1;
  std::cout << "find_spikes: " << std::boolalpha << find_spikes << std::endl;
  if (find_spikes)
    {
      pos = 1;
      for (i = 0; i < aSigInfo.getNumSignals(); i++)
	{
	  if (aSigInfo.isReal(i))
	    {
	      if (aSigInfo.getName(i).find( neuronName ) != std::string::npos)
		{
		  neur_num = i;
		  neur_pos = pos;
		}
	      pos++;
	    }
	}
      if (neur_num != -1)
	{
	  std::cout << "Neuron " << neur_num << std::endl;
	  analogue = values[neur_pos];
	  std::valarray< float > diff =
	    values[neur_pos][std::slice( 1, values[0].size()-1, 1 )];
	  diff -= values[neur_pos][std::slice( 0, values[0].size()-1, 1 )];
	  values[neur_pos] /= diff.max();
	  bool can_spike = false;
	  if (do_decay)
	  {
	    values[neur_pos][0] = 0;
	    for (j = 0; j < diff.size()-1; j++)
	      {
		values[neur_pos][j+1] = values[neur_pos][j]*decay + diff[j];
	      }
	    values[neur_pos][j+1] = values[neur_pos][j]*decay + diff[j];
	    diff =
	      values[neur_pos][std::slice( 1, values[0].size()-1, 1 )];
	    diff -= values[neur_pos][std::slice( 0, values[0].size()-1, 1 )];
	    std::valarray<float> amp = diff[std::slice(0,interval*2,1)];
	    float minmax = diff.max()/2;
	    float divisor = amp.max() >? minmax;	    
	    for (j = 0; j < values[neur_pos].size()-1; j++)
	      {
		if ((j > interval) && (j < diff.size() - interval) &&
		    (j % 1000 == 0))
		  {
		    amp = diff[std::slice(j-interval,interval*2,1)];
		    divisor = amp.max() >? minmax;
		  }
		values[neur_pos][j+1] = values[neur_pos][j] + diff[j]/divisor;
	      }
	    values[neur_pos][j+1] = values[neur_pos][j] + diff[j]/divisor;
	    diff =
	      values[neur_pos][std::slice( 1, values[0].size()-1, 1 )];
	    diff -= values[neur_pos][std::slice( 0, values[0].size()-1, 1 )];
	  }
	  for (j = 0; j < values[neur_pos].size()-1; j++)
	      {
		if (values[neur_pos][j] < spike_size)
		  can_spike = true;
		if ((values[neur_pos][j] > spike_size) &&
		    (diff[j] < 0) && can_spike)
		  {
		    spikes[j] = 1;
		    can_spike = false;
		  }
	      }
	}
      else
	find_spikes = false;
    }

  std::cout << "calc_spike_delays: " << std::boolalpha << calc_spike_delays
	    << std::endl;

  if (calc_spike_delays)
    {
      outfile = fopen( delaysfile.c_str(), "w" );
      float first_low = 0;
      bool gone_low = false;
      float first_high = -1;
      float threshR = (values[r_pos].max() + values[r_pos].min())/2;
      float threshL = (values[l_pos].max() + values[l_pos].min())/2;
      for (j = 0; j < values[0].size()-1; j++)
      {
	if (!gone_low)
	  {
	    if ((values[r_pos][j] <= threshR) &&
		(values[l_pos][j] <= threshL))
	      {
		first_low = values[0][j];
		gone_low = true;
	      }
	  }
	else if (first_high < first_low)
	{
	  if ((values[r_pos][j] > threshR) ||
	      (values[l_pos][j] > threshL))
	    {
	      if (values[0][j] > first_low + delay_time)
		{
		  first_high = values[0][j];
		}
	      else
		{
		  gone_low = false;
		}
	    }
	  }
	else
	  {
	    if (spikes[j] == 1)
	      {
		fprintf( outfile, "%f\n", values[0][j] - first_high );
		std::cout << first_high << " " << values[0][j]-first_high
			  << std::endl;
		gone_low = false;
	      }
	  }
	
      }
      fclose( outfile );
    }
  std::cout << values[0].size() << std::endl;

  outfile = fopen( txtfile.c_str(), "w" );
  for (j=0; j<values[0].size();j++)
    {
      for (i = 0; i<num; i++)
	{
	  fprintf( outfile, "%f\t", values[i][j]);
	  if (find_spikes && (neur_pos == i))
	    {
	      fprintf( outfile, "%d\t", spikes[j]);
	      fprintf( outfile, "%f\t", analogue[j]);
	    }
	  if (do_round && (r_pos == i))
	    {
	      fprintf( outfile, "%d\t", (int) roundedR[j] );
	    }
	  if (do_round && (l_pos == i))
	    {
	      fprintf( outfile, "%d\t", (int) roundedL[j] );
	    }
	}
      fprintf(outfile, "\n" );
    }

  fclose( outfile );

  delete[] anArray;
  return 0;
}

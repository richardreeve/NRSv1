// $Id: calc.c,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
#include <math.h>

int main(int argc, char* argv[])
{
  double spike_inj=1;
  double spike_gap=4;
  double half_life=16;
  double syll_gap=0;
  double syll_length=21;
  double num_syll=1000;
  double chirp_gap=0;
  double chirp_length;
  double num_chirps=1;

  double syll_result;
  double chirp_result;
  double result;

  for (half_life=1; half_life < 500 ; half_life*=2)
    {
      printf("%3.0f : ", half_life);
      for(syll_length=5, syll_gap=0; syll_length<100; syll_length*=2, syll_gap*=2)
	{
	  
	  syll_result=spike_inj*(((1.0-pow(0.5,syll_length/half_life))
				  /(1.0-pow(0.5,spike_gap/half_life)))
				 *pow(0.5,syll_gap/half_life));
	  
	  chirp_result=syll_result*(((1.0-pow(0.5,num_syll*(syll_gap+syll_length)
					      /half_life))
				     /(1.0-pow(0.5,(syll_gap+syll_length)
					       /half_life)))
				    *pow(0.5,chirp_gap/half_life));
	  
	  chirp_length=(syll_length+syll_gap)*num_syll;
	  
	  result=chirp_result*((1.0-pow(0.5,num_chirps*(chirp_gap+chirp_length)
					/half_life))
			       /(1.0-pow(0.5,(chirp_gap+chirp_length)/half_life)));
      
	  //printf("spinj=%3.0f spgap=%3.0f 1/2life=%3.0f sylen=%3.0f sygap=%3.0f Syll=%f Final=%f\n", spike_inj, spike_gap, half_life, syll_length, syll_gap,  syll_result, chirp_result); 
	  printf("(%3.0f = %.2f) ", syll_length, chirp_result); 
	}
      printf("\n");
    }
  return 0;
}

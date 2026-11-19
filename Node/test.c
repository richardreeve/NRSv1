// $Id: test.c,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "base.h"
#include "NodeConversion.h"

typedef enum
{
  outputSensor=0,
  outputSynapse,
  outputFacDep,
  outputSpike,
  outputMembV,
  outputMembG,
  outputAxon,
  outputMotor,
  outputTrack,
  outputInput
} OutputLevel;

int main( int argc, char** argv )
{
  int theFD;
  int num = 500;
  int step = 1;
  char aBuff[ BUFF_MAX ];

  if (argc >= 2)
    num = atoi( argv[1] );
  if (argc >= 3)
    step = atoi( argv[2] );

  node_initialise( "initialise/initialise.params" );
  
  node_add_router( "" );
  node_add_router( "network" );
  node_save_params();
  node_set_param( "refractT", 3.0, "ms" );
  node_add_neuron( NEURONT_SPIKING, "AN1" );
  node_set_param( "refractT", 6.0, "ms" );
  node_add_neuron( NEURONT_SPIKING, "AN2" );
  
  node_add_router( "extraRouter" );
  
  node_add_neuron( NEURONT_SPIKING, "SN1" );
  node_set_param( "membV", -30.0, "mV" );
  node_add_neuron( NEURONT_SPIKING, "SN2" );
  node_add_neuron( NEURONT_MOTOR, "MN" );
  node_pop_context();
  node_restore_params();
  
  node_add_synapse( SYNAPSET_CONDUCTANCE, "VSyn1", SYNAPSES_EXCITATORY,
		    "extraRouter.SN1", "AN1", -1.0 );
  //  node_set_param( "evSynG", 5.0, "nS" );
  node_set_param( "vSynG", 5.0, "nS" );
  
  node_add_synapse( SYNAPSET_CONDUCTANCE, "VSyn2", SYNAPSES_EXCITATORY,
		    "extraRouter.SN2", "AN1", -1.0 );
  
  node_add_router( "extraRouter" );
  
  node_add_synapse( SYNAPSET_MOTOR, "MSyn", SYNAPSES_EXCITATORY,
		    "SN2", "MN", -1.0 );
  
  node_pop_context();
  node_set_param( "synDelayT", 1.0, "ms" );
  node_add_synapse( SYNAPSET_CURRENT, "CSyn1", SYNAPSES_EXCITATORY,
		    "AN1", "AN2", -1.0 );
  

  node_remove_node( "CSyn1" );
  node_remove_node( "VSyn1" );
  
  printf( node_describe( "extraRouter.MSyn", aBuff ) );

  node_set_param( "stepT", 5.0, "ms" );
  
  node_pop_context();
  
  node_remove_node( "network.extraRouter.SN1" );
  
  node_set_record_level( 1<<outputSynapse | 1<<outputSpike |
			 1<<outputMembV | 1<<outputMembG );
  
  printf( "And network.extraRouter\n\n");
  
  printf( node_describe( "network.extraRouter", aBuff ) );

  node_set_param( "stepT", 5.0, "ms" );

  theFD = node_init_record( "r2" );

  /*      outputSensor=0,
	  outputSynapse,
	  outputFacDep,
	  outputSpike,
	  outputMembV,
	  outputAxon,
	  outputMotor,
	  outputTrack
  */
  
  double dBuff[BUFF_MAX];
  double *dPtr;
  int i;
  for (i=0; i < num; i++)
    {
      if (i%step==0)
	{
	  dBuff[0] = (double)( i );
	  
	  dPtr = node_record_state( dBuff + 1 );
	  write( theFD, dBuff, sizeof( double ) * (dPtr - dBuff) );
	}
      node_update();
    }
  node_free();
  close( theFD );

  return 0;
}

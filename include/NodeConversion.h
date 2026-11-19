// $Id: NodeConversion.h,v 1.6 2003/09/22 09:34:32 darrens Exp $
// C include file for interfacing to C++
#ifndef NODECONVERSION_H
#define NODECONVERSION_H

#ifdef __cplusplus
extern "C"
{
#endif
  
#include "base.h"
#include "peripheral.h"
#include "sensor.h"
  
  typedef enum {
    SYNAPSET_CONDUCTANCE=0,
    SYNAPSET_CURRENT,
    SYNAPSET_MOTOR,
    SYNAPSET_NEUTRAL,
    SYNAPSET_PLAIN,
    SYNAPSET_MODULATORY,
    SYNAPSET_CONDITIONABLE,
    SYNAPSET_UNKNOWN
  } SynapseType;
  
  typedef enum {
    NEURONT_SPIKING=0,
    NEURONT_NON_SPIKING,
    NEURONT_MOTOR,
    NEURONT_POISSON,
    NEURONT_SPIKE_SOURCE, 
    NEURONT_UNKNOWN
  } NeuronType;
  
  // sets up network
  char* param_get_units(const char *aParam, char *aBuff );
  char* param_get_description(const char *aParam, char *aBuff );
  double param_get_value(const char *aParam );
  void node_free();
  BOOL node_initialise(const char *initFile );
  void node_save_params();
  void node_restore_params();
  BOOL node_set_param(const char *aName, double aValue, const char *theUnits );
  BOOL node_add_router(const char *aName );
  BOOL node_add_neuron(NeuronType aType, const char *aName );
  BOOL neuron_set_target(const char *aName, const char *aTarget );
  BOOL node_add_synapse(SynapseType aType, 
                        const char *aName, 
                        SynapseSign aSign,
                        const char *anIn, 
                        const char *anOut, 
                        double aDelay);
  BOOL node_add_peripheral(BOOL inRobot, 
                           BOOL anInput, 
                           PeripheralType aType,
                           const char *aName, 
                           VarArg aVA );
  BOOL node_remove_node(const char *aName );
  char* node_describe(const char *aName, char *aBuff );
  void node_pop_context();
  void node_set_record_level(const INT aLevel );
  int node_init_record(const char *aBaseName );
  double *node_record_state(double *theData );
  char* node_get_names(INT aLevel, char *aBuff );
  double *node_get_state(double *theData,  INT aLevel );
  double node_get_timestep();
  // does one timestep of simulation
  BOOL node_update();
  // cleans up i/o at end of experiment
  void node_cleanup();
// frees node
  void node_free();

  // Set a parameter belonging to a specific node - added for the XML interface
  BOOL node_set_node_param(const char *aNodeName,
                           const char *aName, 
                           double aValue, 
                           const char *theUnits );
  

  
#ifdef __cplusplus
};
#endif

#endif //ndef NODECONVERSION_H

// $Id: ConductanceSynapse.cc,v 1.7 2004/06/29 10:08:11 darrens Exp $
#include <iostream>
#include <map>
#include <string>

#include <cmath>
#pragma implementation
#include "ConductanceSynapse.hh"
#include "Params.hh"
#include "base.h"
#include "Exception.hh"

using namespace Runtime;

#define SYNAPSE_NAME "ConductanceBased"

namespace 
{
  // This function does the initialisation
  void cos_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    try
      {
	aParams.addParam( "Synapse", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    // Shared with all synapses
	    aParams.addParam( "excitatory", "enum" );
	    aParams.addParam( "inhibitory", "enum" );
	    aParams.addParam( "expSynT", "ms" );
	    aParams.addParam( "synDelayT", "ms" );
	    aParams.addParam( "synFacT", "ms" );
	    aParams.addParam( "synDepT", "ms" );
	  }
	catch ( bad_param& bp )
	  {
            _TRACE_("Parameter exception: " << bp.what() 
                    << " Hint: " << bp.hint() 
                    << " Thrower: " << bp.origin());
	    throw;
	  }
      }
    
    already=false;
    
    try
      {
	aParams.addParam( SYNAPSE_NAME, "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    aParams.addParam( "shunting", "enum" );

	    aParams.addParam( "evSynV", "mV" );
	    aParams.addParam( "ivSynV", "mV" );
	    aParams.addParam( "gvSynV", "mV" );
	    
	    // spike input parameters
	    aParams.addParam( "vSynG", "nS" );
	    aParams.addParam( "vSynFacG", "nS" );
	    aParams.addParam( "vSynDepFrac", "/ms" );
	    
	    // analogue input parameters
	    aParams.addParam( "avSynGT", "nS/ms/unit" );
	    aParams.addParam( "avSynFacGT", "nS/ms/ms/unit/unit" );
	    aParams.addParam( "avSynDepFracT", "/ms/unit" );
	  }
	catch ( bad_param& bp )
	  {
            _TRACE_("Parameter exception: " << bp.what() 
                    << " Hint: " << bp.hint() 
                    << " Thrower: " << bp.origin());
	    throw;
	  }
      }
  }
}

/// Initialise parameter names for this node type
/**
 * @exception bad_param thrown if the parameter already exists
 */
void ConductanceSynapse::initParams() throw ( bad_param, bad_file )
{
  cos_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void ConductanceSynapse::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a conductance synapse called " << iName 
       << " which belongs to node " << iParent->getName() << std::endl;
#endif

  Params& pParams = iParent->getParams();
  double v_base = pParams.getParam( "membV" );
  double stepT = iParams.getParam( "stepT" );
  double tc = - stepT / pParams.getParam( "membC" );

  // Calculate the relative reversal potential of the synapse.
  // Throughout NRS, calculations involving membrane potential are
  // offset so that the membrane potential is formally treated as if it
  // resides at zero.  So to conform with that coordinate system, we
  // here transform the user-specified reveral potential to the
  // zero-based coordinates.  Not that the shift is based upon the
  // membrane potential of the synapse's target, hence the use of
  // v_base, which is the paramater object for the parent of this
  // synapse.
  switch (iSign)
    {
    case SYNAPSES_SIGNLESS:
      // reversal potential
      iVBattery = iParams.getParam( "gvSynV" ) - v_base;
      break;

    case SYNAPSES_EXCITATORY:
      // battery potential
      iVBattery = iParams.getParam( "evSynV" ) - v_base;

      // VERIFICATION COMMENTED OUT TO PREVENT SPURIOUS
      // WARNINGS FOR MOTOR SYNAPSES
//       // Verification. For excitatory synapses, the 'evSynV' parameter
//       // should be greater than the resting potential of the target
//       // neuron (i.e, greater than 'v_base'). 
//       if (iVBattery <= 0.0)
//         {
//           _WARN_("Synapse "
//                  << iName
//                  << " has been explicity defined as excitatory,"
//                  << " but the value specified for parameter evSynV"
//                  << " (" << iParams.getParam( "evSynV" ) << " mV)"
//                  << " is less than or equal to the resting potential"
//                  << " for the target neuron" 
//                  << " (" << v_base << " mV), which means this synapse"
//                  << " will not act excitatory.");
//         } 
      break;

    case SYNAPSES_INHIBITORY:
      // battery potential
      iVBattery = iParams.getParam( "ivSynV" ) - v_base;

      // VERIFICATION COMMENTED OUT TO PREVENT SPURIOUS
      // WARNINGS FOR MOTOR SYNAPSES
//       // Verification. For excitatory synapses, the 'evSynV' parameter
//       // should be greater than the resting potential of the target
//       // neuron (i.e, greater than 'v_base'). 
//       if (iVBattery >= 0.0)
//         {
//           _WARN_("Synapse "
//                  << iName
//                  << " has been explicity defined as inhibitory,"
//                  << " but the value specified for parameter ivSynV"
//                  << " (" << iParams.getParam( "ivSynV" ) << " mV)"
//                  << " is greater than or equal to the resting potential"
//                  << " for the target neuron" 
//                  << " (" << v_base << " mV), which means this synapse"
//                  << " will not act inhibitory.");
//         } 
      break;

    case SYNAPSES_SHUNTING:
      // battery potential
      iVBattery = 0.0;
      break;

    }

  // scaled base conductance
  iGBase_tc = tc * iParams.getParam( "vSynG" );;
  iGCBase_tc = tc * iParams.getParam( "avSynGT" ) * stepT;
  // Decay rate of conductance in synapse
  iDecay = exp( log( 0.5 ) * stepT / iParams.getParam( "expSynT" ) );
  // scaled current conductance defaults to 0
  iGActual_tc = 0.0;
  // scaled facilitatory conductance increase
  iGFac_tc = tc * iParams.getParam("vSynFacG");
  iGCFac_tc = tc * iParams.getParam("avSynFacGT") * stepT;
  // and associated decay
  iTFac = exp( log( 0.5 ) * stepT / iParams.getParam( "synFacT" ) );
  // start at 0
  iGFacReal_tc = 0;
  // scaled depressive conductance decrease
  iDepFrac = iParams.getParam( "vSynDepFrac" );
  iCDepFrac_ln = log( iParams.getParam( "avSynDepFracT" ) ) * stepT;
  // and associated decay
  iTDep = exp( log( 0.5 ) * stepT / iParams.getParam( "synDepT" ) );
  // start at 1 (no depression)
  iDepRealFrac = 1.0;

  // No timesteps since last output
  iCount = 0;
  iSumGActual_tc = 0;
  iSumDepRealFrac = 0;
  iSumGFacReal_tc = 0;
}

/// Do one timestep for self only
bool ConductanceSynapse::nodeTimestep()
{
  double theGV;

  // decay conductance
  iGActual_tc *= iDecay;
  iSumGActual_tc += iGActual_tc;
  
  // decay facilitation/depression
  iGFacReal_tc *= iTFac;
  iSumGFacReal_tc += iGFacReal_tc;
  
  // iDepRealFrac = 1.0 - (1.0 - iDepRealFrac) * iTDep;
  iDepRealFrac -= 1.0;
  iDepRealFrac *= iTDep;
  iDepRealFrac += 1.0;
  iSumDepRealFrac += iDepRealFrac;
  
  // Update output neuron (only if conductance > 0 ie tc<0)
  // Actually always true now, so can remove this:
  // if (iGActual_tc < 0.0 )
  theGV = iGActual_tc * iVBattery;
  iParent->doReceive( iMessage.newConductance( iGActual_tc, theGV ) );

  iCount++;
  return true;
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& ConductanceSynapse::externalise( std::ostream& anOStream, 
					       std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();
  
#ifdef DEBUG
  anOStream << "// Configuring a conductance synapse called " << iName
	    << " which belongs to node " << iParent->getName() 
	    << std::endl << indent;
#endif

  // Note may not be connected, so should not mention it...
  if (iRemote.begin() == iRemote.end())
    return anOStream;

  anOStream << "add Synapse " << iName << " " << SYNAPSE_NAME << " "; 
  switch (iSign)
    {
    case SYNAPSES_SIGNLESS:
      /* nothing to output */
      break;
    case SYNAPSES_EXCITATORY:
      anOStream << "excitatory ";
      break;
    case SYNAPSES_INHIBITORY:
      anOStream << "inhibitory ";
      break;
    case SYNAPSES_SHUNTING:
      anOStream << "shunting ";
      break;
    }
  anOStream << "from ";
  // Note illegal if more than one, but we'll print it out anyway...
  for (std::map< std::string, bool >::iterator anIter = iRemote.begin();
       anIter != iRemote.end(); anIter++)
    {
      if (anIter != iRemote.begin())
	anOStream << "/* and illegal ";
      anOStream << anIter->first << " ";
      if (anIter != iRemote.begin())
	anOStream << "*/ ";
    }

  anOStream << "to " << iParent->getName();
  
  if (iParams.getParam( "synDelayT" ) != pParams.getParam( "synDelayT" ))
    {
      anOStream << " delay " << iParams.getParam( "synDelayT" );
    }

  if (iParams.getParam( "expSynT" ) != pParams.getParam( "expSynT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "expSynT" );
    }

  if (iParams.getParam( "synFacT" ) != pParams.getParam( "synFacT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "synFacT" );
    }

  if (iParams.getParam( "synDepT" ) != pParams.getParam( "synDepT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "synDepT" );
    }

  if (iParams.getParam( "evSynV" ) != pParams.getParam( "evSynV" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "evSynV" );
    }

  if (iParams.getParam( "ivSynV" ) != pParams.getParam( "ivSynV" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "ivSynV" );
    }

  if (iParams.getParam( "gvSynV" ) != pParams.getParam( "gvSynV" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "gvSynV" );
    }

  if (iParams.getParam( "vSynG" ) != pParams.getParam( "vSynG" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "vSynG" );
    }

//    if (iParams.getParam( "evSynG" ) != pParams.getParam( "evSynG" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "evSynG" );
//      }

//    if (iParams.getParam( "ivSynG" ) != pParams.getParam( "ivSynG" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "ivSynG" );
//      }

//    if (iParams.getParam( "svSynG" ) != pParams.getParam( "svSynG" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "svSynG" );
//      }

  if (iParams.getParam( "vSynFacG" ) != pParams.getParam( "vSynFacG" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "vSynFacG" );
    }

  if (iParams.getParam( "vSynDepFrac" ) !=
      pParams.getParam( "vSynDepFrac" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "vSynDepFrac" );
    }

  if (iParams.getParam( "avSynGT" ) != pParams.getParam( "avSynGT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "avSynGT" );
    }

//    if (iParams.getParam( "aevSynGT" ) != pParams.getParam( "aevSynGT" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "aevSynGT" );
//      }

//    if (iParams.getParam( "aivSynGT" ) != pParams.getParam( "aivSynGT" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "aivSynGT" );
//      }

//    if (iParams.getParam( "asvSynGT" ) != pParams.getParam( "asvSynGT" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "asvSynGT" );
//      }

  if (iParams.getParam( "avSynFacGT" ) != pParams.getParam( "avSynFacGT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "avSynFacGT" );
    }

  if (iParams.getParam( "avSynDepFracT" ) != 
      pParams.getParam( "avSynDepFracT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "avSynDepFracT" );
    }

  if (extras)
    {
      indent.erase( indent.size()-1 );
      anOStream << indent << "}";
    }
  else
    anOStream << ";";
  
  return anOStream;
}

/// Output variable names for neuron
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& ConductanceSynapse::write( std::ostream& anOStream,
					 OutputType aType )
{
  std::string aName = getName( true );
  std::string myName = aName;
  std::string totalName = getRoot()->getName() + '.' + aName;
  std::string::size_type anST = aName.find( '.' );
  while (anST != aName.npos)
    {
      aName.replace( anST, 1, "_" );
      anST = aName.find( '.' );
    }

  switch ( aType )
    {
    case outTypeText:
      if (gOutputLevel.test( outputSynapse ))
	anOStream << myName << ".iGActual_tc\t";
      if (gOutputLevel.test( outputFacDep ))
	anOStream << myName << ".iGFacReal_tc\t" 
		  << myName << ".iDepRealFrac\t";
      break;
    case outTypeMatlab:
      if ((gOutputLevel.test( outputSynapse )) ||
	  (gOutputLevel.test( outputFacDep )))
	{
	  anOStream << aName << ".conductancesynapse = 1;" << std::endl;
	  double tcMemb = - iParams.getParam( "stepT" ) /
	    iParent->getParams().getParam( "membC" );
	  anOStream << aName << ".tcMemb = " << tcMemb << ";" << std::endl;
	  double synV = iVBattery + iParent->getParams().getParam( "membV" );
	  anOStream << aName << ".synV = " << synV << ";" << std::endl;
	  anOStream << totalName << " = node_convert( "
		    << aName << " );" << std::endl;
	  anOStream << "clear " << aName << std::endl;
	}
      break;
    default:
      std::cerr << "Unknown output type" << std::endl;
      break;
    }
  return anOStream;
}

/// Output current state of node to array
/**
 * This is used by all the other put methods to gather data
 * @param anArray array of doubles to store data in
 * @param average true to get average since last reading, else current
 * @return pointer to end of used part of array
 */
double* ConductanceSynapse::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test( outputSynapse ))
	    {
	      *(thePtrPos++) = iSumGActual_tc/iCount;
	    }
	  if (gOutputLevel.test( outputFacDep ))
	    {
	      *(thePtrPos++) = iSumGFacReal_tc/iCount;
	      *(thePtrPos++) = iSumDepRealFrac/iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputSynapse ))
	    {
	      *(thePtrPos++) = iGActual_tc;
	    }
	  if (gOutputLevel.test( outputFacDep ))
	    {
	      *(thePtrPos++) = iGFacReal_tc;
	      *(thePtrPos++) = iDepRealFrac;
	    }
	}
      
      iCount = 0;
      iSumGFacReal_tc = 0;
      iSumGActual_tc = 0;
      iSumDepRealFrac = 0;
    }
  else
    {
      if (gOutputLevel.test( outputSynapse ))
	{
	  *(thePtrPos++) = iGActual_tc;
	}
      if (gOutputLevel.test( outputFacDep ))
	{
	  *(thePtrPos++) = iGFacReal_tc;
	  *(thePtrPos++) = iDepRealFrac;
	}
    }      
  return thePtrPos;
}


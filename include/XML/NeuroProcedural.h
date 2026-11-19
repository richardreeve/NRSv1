// $Id: NeuroProcedural.h,v 1.4 2003/10/13 12:21:34 darrens Exp $
#ifndef NEUROPROCEDURAL_H
#define NEUROPROCEDURAL_H

/// XML element tags
#define XML_TAB_CREATE_CONTAINER "CreateContainer"
#define XML_TAB_CREATE_LINK "CreateLink"
#define XML_TAB_SET_PARAM "SetParam"

/// XML element attributes
#define XML_ATTR_TYPE "type"
#define XML_ATTR_NAME "name"
#define XML_ATTR_VALUE "value"
#define XML_ATTR_UNITS "units"
#define XML_ATTR_TARGET "target"
#define XML_ATTR_FROM "from"
#define XML_ATTR_TO "to"
#define XML_ATTR_SIGN "sign"   // used for synapse sign
#define XML_ATTR_DELAY "delay" // used for synapse delay

/// Values permitted for XML element attributes
#define XML_ATTRVAL_NETWORK "network"
#define XML_ATTRVAL_SPIKING "Spiking"
#define XML_ATTRVAL_NON_SPIKING "NonSpiking"
#define XML_ATTRVAL_MOTOR "Motor"
#define XML_ATTRVAL_POISSON "Poisson"
#define XML_ATTRVAL_SPIKE_SOURCE "SpikeSource"
#define XML_ATTRVAL_CONDUCTANCE_BASED "ConductanceBased"

#endif

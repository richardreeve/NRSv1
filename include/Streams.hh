// Streams template for << and >>
// $Id: Streams.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __STREAMS_HH
#define __STREAMS_HH

#include <iostream>
#include "Node.hh"

std::ostream& 
operator<<( std::ostream& anOStream, const Runtime::Node& anOutput )
{
  return anOutput.put( anOStream );
}

std::istream& 
operator>>( std::istream& anIStream, const Runtime::Node& anInput )
{
  return anInput.get( anIStream );
}

#endif //ndef __STREAMS_HH

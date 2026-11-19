// $Id: Exception.cc,v 1.2 2003/09/25 13:35:03 darrens Exp $

#pragma implementation
#include "Exception.hh"

namespace NRS
{
  std::ostream& operator<<(std::ostream& stream, NRS::Exception& ex)
  {
    stream << ex.m_sstream.str() << ". Thrower=" << ex.m_thrower;
    
    return stream;
  }  
}

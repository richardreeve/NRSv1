// $Id: Exception.cc,v 1.2 2004/05/14 21:43:31 richardr Exp $

#ifndef WIN32
#pragma implementation
#endif

#include "Exception.hh"

namespace NRS
{
  namespace Base
  {
    std::ostream& operator<<( std::ostream& stream, NRS::Base::Exception& ex )
    {
      stream << ex.m_sstream.str() << "Thrower=" << ex.m_thrower << ". ";
      
      return stream;
    }  
  }
}

// Parameters for simulation
// $Id: Params.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __PARAMS_HH
#define __PARAMS_HH

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include "Exception.hh"

#pragma interface

#define BAD_PARAM(name, hint) bad_param(name, hint, __FILE__, __LINE__)

namespace Runtime
{
  /// Exception class indicating parameter was inappropriate for task chosen
  class bad_param : public runtime_exception
  {
  public:
    bad_param() : runtime_exception() { }
    bad_param( std::string aName ) : runtime_exception( aName ) { }
    bad_param(std::string aName,
              std::string aHint,
              std::string aOrigin_file,
              int aOrigin_line) : 
      runtime_exception(aName, aHint, aOrigin_file, aOrigin_line ) { }
  };

  /// class for storage of internal variables of nodes
  class Params
  {
  public:
    /// Standard constructor
    Params() : iBackedUp( false ) 
    {
      // Is the initialisation file open?
      if (!gInitFileStream.is_open())
	{
	  // Open new file for reading
	  gInitFileStream.open( gInitFilename.c_str() );
	}
    }

    /// Copy constructor
    Params( const Params& aParams ) : iBackedUp( false )
    {
      *this = aParams;
    }

    /// Copy assignment
    Params& operator=( const Params& aParams )
    {
      iParamSet = aParams.iParamSet;
      iBackedUp = false;
      return *this;
    }

    /// Static mutator for parameter initialisation file
    /**
     * This static method sets the file that all addParam calls will look
     * for a default value in.
     * @param aFilename name of file to open
     * @exception runtime_exception thrown if filename not available
     * for reading
     */
    static void setInitialisationFile( std::string aFilename ) throw ( bad_file );

    /// Accessor for the initialisation filename
    /**
     * @return Name of current initialisation file
     */
    static std::string getInitialisationFile( void )
    {
      return gInitFilename;
    }

    /// Accessor for a parameter from the initialisation file
    /**
     * @param aParam parameter name to be found
     * @exception bad_param thrown if parameter is not found
     * @exception bad_file thrown if error reading file
     */
    static double getInitialValue( std::string aParam ) 
      throw ( bad_param, bad_file );

    /// Accessor for the parameter units from the initialisation file
    /**
     * @param aParam parameter name to be found
     * @exception bad_param thrown if parameter is not found
     * @exception bad_file thrown if error reading file
     */
    static std::string getParameterUnits( std::string aParam ) 
      throw ( bad_param, bad_file );

    /// Accessor for a description from the initialisation file
    /**
     * @param aParam parameter name to be found
     * @exception bad_param thrown if parameter is not found
     * @exception bad_file thrown if error reading file (eg non terminated
     * comment)
     */
    static std::string getParameterDescription( std::string aParam )
      throw ( bad_param, bad_file );

    /// Add a new parameter
    /**
     * @param aName name of parameter
     * @param theUnits units of parameter
     * @exception bad_param thrown if parameter already exists
     * or units wrong
     */
    void addParam( std::string aName, std::string theUnits ) 
      throw ( bad_param, bad_file )
    {
      if (iParamSet.find( aName ) != iParamSet.end())
	throw BAD_PARAM(aName, "Already defined");
      else
	{
	  if (getParameterUnits( aName ) == theUnits)
	    iParamSet[aName] = getInitialValue( aName );
	  else
	    throw BAD_PARAM(aName, "Incorrect units");
	}
    }

    /// Add a new parameter
    /**
     * @param aName name of parameter
     * @exception bad_param thrown if parameter already exists
     */
    void addParam( std::string aName ) 
      throw ( bad_param, bad_file )
    {
      if (iParamSet.find( aName ) != iParamSet.end())
	throw BAD_PARAM(aName, "Already defined"); 
      else
	iParamSet[aName] = getInitialValue( aName );
    }

    /// Change value of an existing parameter
    /**
     * @param aName name of parameter
     * @param aVal value for parameter
     * @exception bad_param thrown if parameter does not exists
     */
    void setParam( std::string aName, double aVal ) throw ( bad_param )
    {
      if (iParamSet.find( aName ) == iParamSet.end())
	throw BAD_PARAM(aName, "Parameter not found");
      else
	iParamSet[aName] = aVal;
    }
    
    /// Change value of an existing parameter
    /**
     * @param aName name of parameter
     * @param theUnits name of units for parameter
     * @param aVal value for parameter
     * @exception bad_param thrown if parameter does not exists
     * or units wrong
     * @exception bad_file thrown if there is a problem with 
     * the initialisation file
     */
    void setParam( std::string aName, std::string theUnits, double aVal )
      throw ( bad_param, bad_file )
    {
      if (iParamSet.find( aName ) == iParamSet.end())
	throw BAD_PARAM(aName, "Parameter not found");
      else
	{
	  if (getParameterUnits( aName ) == theUnits)
	    iParamSet[aName] = aVal;
	  else
	    throw BAD_PARAM(aName, "Incorrect units");
	}
    }
    
    /// Get current value of a parameter
    /**
     * @param aName name of parameter
     * @return current value of parameter
     * @exception bad_param thrown if parameter does not exists
     */
     double getParam( std::string aName ) throw ( bad_param )
    {
      if (iParamSet.find( aName ) == iParamSet.end())
	throw BAD_PARAM(aName, "Parameter not found");
      
      return iParamSet[aName];
    }
    
    /// Get current parameter map
    /**
     * @return current parameter map
     */
    std::map< std::string, double >& getParamSet() throw ()
    {
      return iParamSet;
    }
    
    /// Make a backup of current parameter settings for subsequent restore
    /**
     * @exception bad_param thrown if backup already active
     */
    void backup() throw ( bad_param )
    {
      if (iBackedUp)
	{
	  std::cerr << "Warning: Parameters already backed up..." << std::endl;
	}
      else
	{
	  iBackedUp = true;
	}
      iBackup = iParamSet;
    }
    
    /// Restore old parameter settings from backup
    /**
     * @exception bad_param thrown if backup not made
     */
    void restore() throw ( bad_param )
    {
      if (!iBackedUp)
	throw bad_param();
      else
	{
	  iBackedUp = false;
	  iParamSet = iBackup;
	}
    }

    /// Output parameters to stream
    /**
     * @param aStream the output stream
     * @param useUnits do we output the units
     * @param giveDescription do we output the description of the parameter
     * @return the stream after output
     */
    std::ostream& put( std::ostream& aStream )
    {
      for (std::map< std::string, double >::const_iterator seq = iParamSet.begin(); 
	   seq != iParamSet.end(); seq++)
	{
	  put( aStream, seq->first, true, true );
	}
      return aStream << std::endl << "// " << iParamSet.size() << " element(s)";
    }
    
    /// Output a parameter to stream
    /**
     * @param aStream the output stream
     * @param aParam the name of the parameter
     * @param useUnits do we output the units
     * @param giveDescription do we output the description of the parameter
     * @return the stream after output
     */
    std::ostream& put( std::ostream& aStream, std::string aParam,
		  bool useUnits = true, bool giveDescription = false  )
    {
      aStream << aParam << " = " << getParam( aParam ) << " ";
      if (useUnits)
	aStream << getParameterUnits( aParam );
      if ((giveDescription) && (!getParameterDescription( aParam ).empty()))
	aStream << " { " << getParameterDescription( aParam ) << " }" << std::endl;
      else
	aStream << ";" << std::endl;

      return aStream;
    }
    
    /// Input parameters from stream
    /**
     * @param aStream the input stream
     * @return the stream after input
     */
    std::istream& get( std::istream& aStream )
    {
      throw bad_file();
    }
    
  private:
    /// true if a backup is active
    bool iBackedUp;

    /// storage for parameters and their backups
    std::map< std::string, double > iParamSet, iBackup;

    /// Static filename for finding initialisation parameters
    static std::ifstream gInitFileStream;
    static std::string gInitFilename;
  };

}

std::ostream& operator<<( std::ostream& anOStream, const Runtime::Params& anOutput );

std::istream& operator>>( std::istream& anIStream, Runtime::Params& anInput );

#endif //ndef __PARAMS_HH

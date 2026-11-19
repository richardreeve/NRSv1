// $Id: Params.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <string>
#include <iostream>
#include <fstream>

#include <cctype>

#include "base.h"

#pragma implementation
#include "Params.hh"

using namespace Runtime;

std::ifstream Params::gInitFileStream;
std::string Params::gInitFilename( DEFAULT_INIT_FILE );

namespace 
{
  /// Gets rid of C or C++ comments, called after finding first /
  /**
   * @param aStream input stream current looking at a /
   * @return next input byte
   * @exception bad_file if we reach the end of file during a C comment
   */
  int killComment( std::ifstream &aStream ) throw ( bad_file )
  {
    int c = 0;
    std::string junk;

    std::streampos comment = aStream.tellg();
    aStream.ignore();
    if (!aStream.eof())
      {
	c = aStream.peek();
	if (c == '/') // Is a C++ comment
	  {
	    getline( aStream, junk );
	    if (!aStream.eof())
	      {
		c = aStream.peek();
		if (c == '/')
		  c = killComment( aStream );
	      }
	  }
	else if (c == '*') // Is a C comment
	  {
	    while ((!aStream.eof()) && (c != '/'))
	      {
		getline( aStream, junk, '*' );
		if (aStream.eof())
		  throw bad_file();
		c = aStream.peek();
	      }
	    aStream.ignore();
	    if (!aStream.eof())
	      {
		c = aStream.peek();
		if (c == '/')
		  c = killComment( aStream );
	      }
	  }
	else // rewind
	  {
	    aStream.seekg( comment );
	    c = aStream.peek();
	  }
      }
    return c;
  }

  /// Gets next available word
  /**
   * Picks up next word starting with a letter, and with only
   * alphanumeric characters _, / and * in it, and puts in into aWord
   * @param aStream input stream
   * @param aWord storage for word
   * @exception bad_file thrown on unterminated comment, or non alpha first,
   * but not just because there is no word
   */
  void getWord( std::ifstream &aStream, std::string& aWord )
    throw ( bad_file )
  {
    int c = 0;
    
    aWord.erase();
    if (!aStream.eof())
      {
	c = aStream.peek();
	if (c == '/') // May be a C or C++ comment
	  c = killComment( aStream );
  	
	while ((!aStream.eof()) && isspace( c ))
	  {
	    aStream.ignore();
	    if (!aStream.eof())
	      c = aStream.peek();
	    if (c == '/') // May be a C or C++ comment
	      c = killComment( aStream );
	  }
	
	// Word must start with alpha or / (for units)
	if ((!aStream.eof()) && (!isalpha( c )) && (c != '/'))
	  throw bad_file();
	
	// Found start of word
	while ((!aStream.eof()) && (isalnum( c ) || (c == '_') ||
				    (c == '*') || (c == '/')))
	  {
	    aWord.push_back( static_cast< char >( c ) );
	    aStream.ignore();
	    if (!aStream.eof())
	      c = aStream.peek();
	    if (c == '/')
	      {
		c = killComment( aStream );
		// if we've moved on, then comment marks end of word!
		if (c != '/')
		  c = ' ';
	      }
	  }
      }
  }

  /// Gets next available number
  /**
   * Picks up next number, ignoring spaces and equals sign
   * @param aStream input stream
   * @param aNumber storage for number
   * @exception bad_file thrown on unterminated comment, or non number,
   * or nothing
   */
  void getNumber( std::ifstream &aStream, double& aNumber )
    throw ( bad_file )
  {
    int c = 0;
    std::string aString;

    if (aStream.eof())
      throw bad_file();

    c = aStream.peek();
    if (c == '/') // May be a C or C++ comment
      c = killComment( aStream );
      
    while ((!aStream.eof()) && (isspace( c ) || (c == '=')))
      {
	aStream.ignore();
	if (!aStream.eof())
	  c = aStream.peek();
	if (c == '/') // May be a C or C++ comment
	  c = killComment( aStream );
      }
    
    // Did we find anything?
    if (aStream.eof())
      throw bad_file();
    
    // String must start with number or decimal point, or -
    if ((!isdigit( c )) && (c != '.') && (c != '-'))
      throw bad_file();
    else if (c == '-')
      { // allowed - at beginning only
	aString.push_back( static_cast< char >( c ) );
	aStream.ignore();
	if (!aStream.eof())
	  c = aStream.peek();
      }

    // Found start of number, so find rest
    while ((!aStream.eof()) && (isdigit( c ) || (c == '.')))
      {
	aString.push_back( static_cast< char >( c ) );
	aStream.ignore();
	if (!aStream.eof())
	  c = aStream.peek();
      }

    // Get number out
    aNumber = atof( aString.c_str() );
  }

  /// Gets next available description
  /**
   * Descriptions are enclosed in braces {...} after a parameter initialisation
   * No description (returning empty string) if initialisation ends with ;
   * @param aStream input stream
   * @param aDescription storage for description
   * @exception bad_file thrown on unterminated comment or description,
   * or something else appears
   */
  void getDescription( std::ifstream &aStream, std::string& aDescription )
    throw ( bad_file )
  {
    int c = 0;
    bool done_space = false;

    aDescription.erase();
    if (!aStream.eof())
      {
	c = aStream.peek();
	if (c == '/') // May be a C or C++ comment
	  c = killComment( aStream );
      }

    while ((!aStream.eof()) && isspace( c ))
      {
	aStream.ignore();
	if (!aStream.eof())
	  c = aStream.peek();
	if (c == '/') // May be a C or C++ comment
	  c = killComment( aStream );
	
      }
    // Is there a description associated with this 
    if (c == '{')
      {
	aStream.ignore();
	if (!aStream.eof())
	  c = aStream.peek();

	// Get rid of spaces up to first printable character
	while ((!aStream.eof()) && (isspace(c)))
	  {
	    aStream.ignore();
	    if (!aStream.eof())
	      c = aStream.peek();
	    if (c == '/') // May be a C or C++ comment
	      c = killComment( aStream );
	  }
	
	while ((!aStream.eof()) && (c != '}'))
	  {
	    if (isspace(c))
	      {
		// put in first space and all carriage returns
		if ((c == '\n') || (!done_space))
		  {
		    done_space = true;
		    aDescription.push_back( static_cast< char >( c ) );
		  }
	      }
	    else
	      {
		done_space = false;
		aDescription.push_back( static_cast< char >( c ) );
	      }
	    aStream.ignore();
	    if (!aStream.eof())
	      c = aStream.peek();
	    if (c == '/') // May be a C or C++ comment
	      c = killComment( aStream );
	  }
      }
    else if (c != ';')
      { // Not a description
	throw bad_file();
      }

    if (aStream.eof())
      throw bad_file();
    
    aStream.ignore();

    // Get rid of trailing blanks
    while ((!aDescription.empty()) 
	   && (isspace( aDescription[aDescription.size() - 1] )))
    {
      aDescription.erase( aDescription.size() - 1, aDescription.size() );
    }
  }
}

/// Accessor for a parameter from the initialisation file
/**
 * @param aParam parameter name to be found
 * @exception bad_param thrown if parameter is not found
 * @exception bad_file thrown if error reading file
 */
double Params::getInitialValue( std::string aParam ) 
  throw ( bad_param, bad_file )
{
  std::string match, units, aDescription;
  double aValue;
  bool gotParam = false;
  
  if (!gInitFileStream)
    throw bad_file();
  
  gInitFileStream.seekg( 0, std::ios::beg );
  
  while ((match != aParam) && (!gInitFileStream.eof()))
    {
      try
	{
	  getWord( gInitFileStream, match );
	  gotParam = true;
	  // Have we finished?
	  if (!match.empty())
	    { // If not, get rest of parameter
	      getNumber( gInitFileStream, aValue );
	      getWord( gInitFileStream, units );
	      getDescription( gInitFileStream, aDescription );
	    }
	  gotParam = false;
	}
      catch ( bad_file& )
	{
	  if (gotParam)
	    throw bad_file( gInitFilename );
	  else
	    throw bad_param( aParam );
	}
    }

  // is the variable there?
  if (gInitFileStream.eof())
    throw bad_param( aParam );

#ifdef DEBUG
  if (aDescription.empty())
    std::cout << match << " = " << aValue << " " << units << ";"  << std::endl;
  else
    std::cout << match << " = " << aValue << " " << units
	 << " { " << aDescription << " }" << std::endl;
#endif //def DEBUG

  return aValue;
}

/// Accessor for the parameter units from the initialisation file
/**
 * @param aParam parameter name to be found
 * @exception bad_param thrown if parameter is not found
 * @exception bad_file thrown if error reading file
 */
std::string Params::getParameterUnits( std::string aParam ) 
  throw ( bad_param, bad_file )
{
  std::string match, units, aDescription;
  double aValue;
  bool gotParam = false;

  if (!gInitFileStream)
    throw bad_file();
  
  gInitFileStream.seekg( 0, std::ios::beg );
  
  while ((match != aParam) && (!gInitFileStream.eof()))
    {
      try
	{
	  getWord( gInitFileStream, match );
	  gotParam = true;
	  // Have we finished?
	  if (!match.empty())
	    { // If not, get rest of parameter
	      getNumber( gInitFileStream, aValue );
	      getWord( gInitFileStream, units );
	      getDescription( gInitFileStream, aDescription );
	    }
	  gotParam = false;
	}
      catch ( bad_file& )
	{
	  if (gotParam)
	    throw bad_file( gInitFilename );
	  else
	    throw bad_param( aParam );
	}
    }

  // is the variable there?
  if (gInitFileStream.eof())
    throw bad_param( aParam );

  return units;
}

/// Accessor for a description from the initialisation file
/**
 * @param aParam parameter name to be found
 * @exception bad_param thrown if parameter is not found
 * @exception bad_file thrown if error reading file (eg non terminated
 * comment)
 */
std::string Params::getParameterDescription( std::string aParam )
  throw ( bad_param, bad_file )
{
  std::string match, units, aDescription;
  double aValue;
  bool gotParam = false;
  
  if (!gInitFileStream)
    throw bad_file();
  
  gInitFileStream.seekg( 0, std::ios::beg );
  
  while ((match != aParam) && (!gInitFileStream.eof()))
    {
      try
	{
	  getWord( gInitFileStream, match );
	  gotParam = true;
	  // Have we finished?
	  if (!match.empty())
	    { // If not, get rest of parameter
	      getNumber( gInitFileStream, aValue );
	      getWord( gInitFileStream, units );
	      getDescription( gInitFileStream, aDescription );
	    }
	  gotParam = false;
	}
      catch ( bad_file& )
	{
	  if (gotParam)
	    throw bad_file( gInitFilename );
	  else
	    throw bad_param( aParam );
	}
    }
  
  if (gInitFileStream.eof())
    throw bad_param( aParam );

  return aDescription;
}

/// Static mutator for parameter initialisation file
/**
 * This static method sets the file that all addParam calls will look
 * for a default value in.
 * @param aFilename name of file to open
 * @exception runtime_exception thrown if filename not available
 * for reading
 */
void Params::setInitialisationFile( std::string aFilename ) throw ( bad_file )
{
  // Is some file already open?
  if (gInitFileStream.is_open())
    gInitFileStream.close();
  gInitFileStream.clear();
  // Open new file for reading
  gInitFileStream.open( aFilename.c_str() );
  
  // Did that work?
  if (!gInitFileStream)
    {
      std::cout << "oh, failure..." << std::endl;
      if (gInitFileStream.is_open())
	gInitFileStream.close();
      gInitFileStream.clear();
      throw bad_file( aFilename );
    }
//    if (gInitFileStream.is_open())
//      gInitFileStream.close();
  
//    std::string maybeFile( PROJDIR );
//    maybeFile += aFilename;
//    // Open new file for reading
//    gInitFileStream.open( maybeFile.c_str() );
  
//    // Did that work?
//    if (!gInitFileStream)
//      {
//        maybeFile = PROJDIR;
//        maybeFile += "../";
//        maybeFile += aFilename;
//        if (gInitFileStream.is_open())
//  	gInitFileStream.close();
//        gInitFileStream.clear();
//        // Open new file for reading
//        gInitFileStream.open( maybeFile.c_str() );
      
//        // Did that work?
//        if (!gInitFileStream)
//  	{
//  	  if (gInitFileStream.is_open())
//  	    gInitFileStream.close();
//  	  gInitFileStream.clear();
//  	  // Open new file for reading
//  	  gInitFileStream.open( aFilename.c_str() );
	  
//  	  // Did that work?
//  	  if (!gInitFileStream)
//  	    {
//  	      if (gInitFileStream.is_open())
//  		gInitFileStream.close();
//  	      gInitFileStream.clear();
//  	      throw bad_file( aFilename );
//  	    }
//  	}
//      }
  
  gInitFilename = aFilename;
}

std::ostream& 
operator<<( std::ostream& anOStream, Params& anOutput )
{
  return anOutput.put( anOStream );
}

std::istream& 
operator>>( std::istream& anIStream, Params& anInput )
{
  return anInput.get( anIStream );
}
  

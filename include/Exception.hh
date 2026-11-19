// Base for runtime exceptions
// $Id: Exception.hh,v 1.3 2003/09/25 17:40:15 darrens Exp $

#ifndef __EXCEPTION_HH
#define __EXCEPTION_HH

#include <string>
#include <exception>
#include <sstream>
#include <iostream>

#pragma interface

/* Each of these is functionally similar, but offer different trace-roles. */

#define _TRACE_(X) \
std::cout << "TRACE: " << X << " (" << __FILE__ << ":" << __LINE__ << ")\n"

#define _WARN_(X) \
std::cout << "WARNING: " << X << " (" << __FILE__ << ":" << __LINE__ << ")\n"

#define _ERROR_(X) \
std::cout << "ERROR: " << X << " (" << __FILE__ << ":" << __LINE__ << ")\n"

#define _INFO_(X) \
std::cout << "INFO: " << X << " (" << __FILE__ << ":" << __LINE__ << ")\n"

#define _DEBUG_(X) \
std::cout << "DEBUG: " << X << " (" << __FILE__ << ":" << __LINE__ << ")\n"

// Exception throw macro, can be used to aid debugging
#define _THROW_EXCEPTION_ \
if (std::cout << "EXCEPTION: Thrown by " << __FILE__ << ":" << __LINE__) throw

// Useful macro for use in call to Exception constructors
#define _FL_  __FILE__, __LINE__

/// The namespace for newer NRS components
namespace NRS
{
  /// Base class for all exception classes in NRS.
  /** 
   *  This is the base class for all exception classes used in
   *  NRS. Objects of type Exception can also be created (i.e., this
   *  class is not abstract) and used to pass exception information
   *  within an NRS program. 
   *
   * To represent error information the class stores two kinds of data: 
   *
   * - Thrower location. This is a code location (file and line stamp)
   *   where the Exception object was created. This information is
   *   passed in through the constructor call. Once an Exception object
   *   is created, this thrower information cannot be changed.
   *
   * - Cumulative textual error description. An Exception object can be
   *   created with or without an error string. Either way, the error
   *   string it hold can latter be added to (using insertors), so that
   *   an error message can gradually be built up as a single Exception
   *   object is passed through a program.
   *
   * This class replaces Runtime::runtime_exception which is now deprecated.
   *
   * A friend function is provided to pass the error information within an
   * Exception object to a C++ stream.
   */
  class Exception : public std::exception
  {
  public:
    /** Constructor for use when a complicated error string is not going
     * to be provided. This constructor allows you to specify the error
     * string, so subsequent calls are not needeed (although can be
     * added) to construct an error string.
     *
     * \param errStr A textual description of the program error.
     *
     * \param thrower_file The name of the file which is throwing this
     * exception. Typically you will use the __FILE__ macro.
     *
     * \param thrower_line The line number at which this is exception 
     * originated. Typically you will use the __LINE__ macro.
     *
     */
    Exception(std::string errStr,
              std::string thrower_file, int thrower_line)
    {
      // Build the origin string
      std::stringstream outs;
      outs << thrower_file << ":" << thrower_line << std::ends;
      
      // Store in class member
      m_thrower += outs.str();  
      
      m_sstream << errStr;
    }

    /**
     * Creates an Exception object with an initial empty error
     * string. An error string must be later constructed using the
     * insertor operator.
     *
     * \param errStr A textual description of the program error.
     *
     * \param thrower_file The name of the file which is throwing this
     * exception. Typically you will use the __FILE__ macro.
     *
     * \param thrower_line The line number at which this is exception 
     * originated. Typically you will use the __LINE__ macro.
     *
     */
    Exception(std::string thrower_file, int thrower_line)
    {
      // Build the origin string
      std::stringstream outs;
      outs << thrower_file << ":" << thrower_line << std::ends;
      
      // Store in class member
      m_thrower += outs.str();    
    }
    
    /** The copy constructor.
     */
    Exception(const Exception& src)
      : std::exception(src), m_thrower(src.m_thrower)
    {
      m_sstream << src.m_sstream.str();
    }

    /** Destructor
     */
    virtual ~Exception() throw () {}
    
    /** Inserter. Use this operator to build up the error string held by
     * an Exception object. It's templatised so should work will all
     * types of operand.
     *
     * Example:
     * \code
     * 
     * // Create the object, using C++ macros. Alternatively use the
     * // macro _FL_ defined in the header file which just combines the two
     * // macros below
     * Exception myError(__FILE__, __LINE__);
     *
     * // Build the error string
     * myError << "Error number " << errNum << " for object " << some_obj.name;
     *
     * // And throw
     * throw myError;
     *
     * \endcode
     *
     */
    template <typename T> Exception&  operator<<(T t)
    {
      m_sstream << t;
      
      return *this;
    }

    /**
     * Assignment operator.
     *
     * \return A reference to self.
     */
    virtual Exception& Exception::operator=(Exception& rhs)
    {
      // Ignore assignment to self
      if (this == &rhs) return *this;

      // Do base class assignment
      std::exception::operator=(rhs);

      this->m_sstream.str(rhs.m_sstream.str());
      this->m_thrower = rhs.m_thrower;

      return *this;
    }

    /** 
     * Return the current contents of the error string.
     *
     * \return The contexts of the error string.
     */
    virtual const char* error() const { return m_sstream.str().c_str(); } 

    /** 
     * Return the current contents of the thrower string. The thrower
     * string stores the file and line where the Exception object was
     * made.
     *
     * \return A string representation of the Exception thrower location.
     */
    virtual const char* thrower() const { return m_thrower.c_str(); }  
    
    /**
     * Friend function to stream out the error information stored within
     * an Exception object. When streaming out the object information in
     * this way, the thrower string is automatically appended after the
     * error string.
     *
     * Example:
     * \code
     * 
     * try
     * {
     *     start_program();
     * }
     * catch (Exception& ex)
     * {
     *     // caught an error, report to std out using the exception's 
     *     // friend function
     *     std::cout << ex;
     *
     *     // rethrow
     *     throw;
     * }
     *
     * \endcode
     *
     * Other macros exist to display general purpose error
     * messages. Each macro can be passed a list of operands, separated
     * by '<<'. 
     *
     * Example:
     * \code
     * 
     * Exception ex("test", _FL_);
     *     
     * _INFO_(ex);
     * _TRACE_(ex);
     * _DEBUG_(ex);
     * _ERROR_(ex);
     * _WARN_(ex);
     *
     * _INFO_("Caught an exception, which holds information:" << ex);
     *
     * \endcode
     *
     * \see error() 
     * \see thrower()
     */
    friend std::ostream& operator<<(std::ostream&, Exception&);
  
  protected: 
    
    // Hint about why exception was thrown
    std::stringstream m_sstream;
    
    // File and Line stamp where exception was thrown
    std::string m_thrower;                       
  };
}

namespace Runtime
{

  /// The base class for all Runtime exceptions
  class runtime_exception : public std::exception
  {
  public:
    /// standard constructor for runtime_exception
    /**
     * @param aName name of element which caused exception to be thrown
     */
    runtime_exception( std::string aName ) : iName( aName ) { }
    /// non element-specific exception constructor
    runtime_exception( void ) : iName( "" ) { }

    /// advanced constructor for runtime_exception
    /**
     * @param aName name of element which caused exception to be thrown
     * @param aHint explanation of why exception was thrown
     * @param aOrigin_file file which throws exception (use __FILE__)
     * @param aOrigin_line line which throws exception (use __LINE__)
     */
    runtime_exception(std::string aName,
                      std::string aHint,
                      std::string aOrigin_file,
                      int aOrigin_line) :
      iName(aName),
      iHint(aHint)
    {
      // Build the origin string
      std::stringstream outs;
      outs << aOrigin_file << ":" << aOrigin_line << std::ends;

      // Store in class member
      iOrigin += outs.str();
    }



    /// returns element name
    virtual const char* what() const throw () { return iName.c_str(); }
    /// standard destructor
    virtual ~runtime_exception() throw () {}
    /// returns hint
    virtual const char* hint() const throw () { return iHint.c_str(); }    
    /// returns originator
    virtual const char* origin() const throw () { return iOrigin.c_str(); }

  protected:
    /// Name of element which caused exception to be thrown
    std::string iName;
    
    // Hint about why exception was thrown
    std::string iHint;

    // File and Line stamp where exception was thrown
    std::string iOrigin;
  };

  /// Exception class indicating file was inappropriate for task chosen
  class bad_file : public runtime_exception
  {
  public:
    bad_file() : runtime_exception() { }
    bad_file( std::string aName ) : runtime_exception( aName ) { }
  };

}

#endif //ndef __EXCEPTION_HH

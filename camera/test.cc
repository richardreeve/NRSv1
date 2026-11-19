// $Id: test.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <functional>
#include <valarray>
#include <iostream>
#include <cstring>

struct average : public std::unary_function< unsigned char, unsigned char >
{
  average() : sum(0), items(0) {}

  long sum, items;

  unsigned char operator()( const unsigned char &x )
  {
    sum += static_cast<int>( x );
    items++;
    return x;
  }

  double get_av()
  {
    return (static_cast<double>( sum )/static_cast<double>( items ));
  }
};

int main()
{
  //  unsigned char (*myop)( const unsigned char & )=average();
  unsigned char val[200];
  memset( val, 4, 200 );
  std::valarray<int> vai( 200 );
  for (int i = 0; i < 200; i++ )
    vai[i]=val[i];
  std::cout << vai.sum() << " in " << vai.size() 
	    << " gives " << (vai.sum()+200)/vai.size() << std::endl;
}

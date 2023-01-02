#include <array>
#include <cstdlib>
#include <iostream>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <string>

#include "exception.hh"
#include "parser.hh"
#include "socket.hh"

using namespace std;

void program_body( const string& target_host, const string& target_port )
{
  // construct UDP socket
  UDPSocket sock;

  // connect socket to destination
  sock.connect( { target_host, target_port } );

  // allocate a buffer for the outgoing packets
  array<char, 1400> buffer;
  fill( buffer.begin(), buffer.end(), 0 );

  for ( unsigned int i = 0; i < 64; ++i ) {
    buffer.at( 0 ) = 'A' + i;
    sock.send( { buffer.data(), buffer.size() } );
  }
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc < 0 ) {
      abort();
    }

    if ( argc != 3 ) {
      throw runtime_error( "Usage: "s + argv[0] + " host port" );
    }

    program_body( argv[1], argv[2] );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

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

void program_body( const string& listen_port )
{
  // construct UDP socket
  UDPSocket sock;

  // bind to port
  sock.bind( { "0", listen_port } );

  // allocate a buffer for the incoming packets
  array<char, 1400> buffer;
  string_view buffer_view { buffer.begin(), buffer.size() };

  fill( buffer.begin(), buffer.end(), 0 );

  Address src { "0", "0" };

  for ( unsigned int i = 0; i < 64; ++i ) {
    string_span payload = string_span::from_view( buffer_view );
    sock.recv( src, payload, 1400 );
    if ( payload.size() != 1400 ) {
      throw runtime_error( "unexpected packet size on packet " + to_string( i ) );
    }
    if ( payload.at( 0 ) != char( 'A' + i ) ) {
      throw runtime_error( "unexpected packet contents on packet " + to_string( i ) );
    }
    if ( not all_of( payload.begin() + 1, payload.end(), []( auto x ) { return x == 0; } ) ) {
      throw runtime_error( "unexpected payload contents on packet " + to_string( i ) );
    }
  }

  cout << "All packets received in order with expected contents!\n";
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc < 0 ) {
      abort();
    }

    if ( argc != 2 ) {
      throw runtime_error( "Usage: "s + argv[0] + " listen_port" );
    }

    program_body( argv[1] );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

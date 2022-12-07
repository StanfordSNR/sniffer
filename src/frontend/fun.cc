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

int device_name_to_index( const string& device_name );
void debug_print_packet( const Address& source, string_view contents );

void program_body( const string& device_name )
{
  // construct raw socket
  PacketSocket sock { SOCK_RAW, htobe16( ETH_P_ALL ) };

  // bind to given device (identified by name, but bind by index);
  sockaddr_ll addr_ll {};
  addr_ll.sll_family = AF_PACKET;
  addr_ll.sll_ifindex = device_name_to_index( device_name );
  Address addr { reinterpret_cast<sockaddr*>( &addr_ll ), sizeof( addr_ll ) };
  sock.bind( addr );

  // set interface to promiscuous capture
  sock.set_promiscuous();

  // receive each packet into a buffer
  array<char, 65536> buffer;

  while ( not sock.eof() ) {
    string_span buffer_span { buffer.data(), buffer.size() };
    sock.recv( addr, buffer_span, 65536 );
    // buffer_span has now been resized to the length of the received packet
    debug_print_packet( addr, buffer_span );
  }
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc < 0 ) {
      abort();
    }

    if ( argc != 2 ) {
      throw runtime_error( "Usage: "s + argv[0] + " device_name" );
    }

    program_body( argv[1] );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

string_view packet_type_to_string( const int packet_type )
{
  switch ( packet_type ) {
    case PACKET_HOST:
      return "incoming";
    case PACKET_OTHERHOST:
      return "incoming (to somebody else)";
    case PACKET_OUTGOING:
      return "outgoing";
    default:
      return "other";
  }
}

string_view ip_proto_to_string( const int ip_proto )
{
  switch ( ip_proto ) {
    case IPPROTO_UDP:
      return "UDP";
    case IPPROTO_TCP:
      return "TCP";
    case IPPROTO_ICMP:
      return "ICMP";
    default:
      return "other";
  }
}

constexpr unsigned int ETHERNET_HEADER_LEN = 14;
constexpr unsigned int IP_HEADER_LEN = 20;

void debug_print_packet( const Address& source, string_view contents )
{
  const sockaddr_ll* info = source.cast<sockaddr_ll>( AF_PACKET );

  cerr << "Got packet:\n";
  cerr << "   direction=" << packet_type_to_string( info->sll_pkttype ) << "\n";

  // ignore unless it's IP
  if ( info->sll_protocol != htons( ETH_P_IP ) ) {
    cerr << "   not IP => ignoring\n\n";
    return;
  }

  // ignore if too short
  if ( contents.size() < ETHERNET_HEADER_LEN + IP_HEADER_LEN ) {
    cerr << "   packet length too short => ignoring\n\n";
    return;
  }

  // prepare to parse the IPv4 datagram
  Parser parser { contents };

  // we know it's IP, so we can remove the Ethernet header
  parser.skip( ETHERNET_HEADER_LEN );

  // check the IP version and header length
  uint8_t ip_version_header_length;
  parser.integer( ip_version_header_length );

  if ( ip_version_header_length != 0x45 ) {
    cerr << "   not IPv4 with 20-byte header => ignoring\n\n";
    return;
  }

  // parse the IP protocol (TCP, UDP, ICMP, etc.)
  parser.skip( 8 ); // skip to protocol
  uint8_t ip_proto;
  parser.integer( ip_proto );
  cerr << "   IP protocol = " << ip_proto_to_string( ip_proto ) << "\n";

  // parse the IP source and destination addresses
  parser.skip( 2 ); // skip to addresses
  uint32_t ip_src, ip_dest;
  parser.integer( ip_src );
  parser.integer( ip_dest );

  cerr << "   IPv4 source = " << Address::from_ipv4_numeric( be32toh( ip_src ) ).ip()
       << ", dest = " << Address::from_ipv4_numeric( be32toh( ip_dest ) ).ip() << "\n";

  // parse the UDP port numbers
  if ( ip_proto != IPPROTO_UDP ) {
    cerr << "   not UDP => ignoring\n\n";
    return;
  }

  uint16_t udp_src_port, udp_dest_port;
  parser.integer( udp_src_port );
  parser.integer( udp_dest_port );

  cerr << "   UDP source port = " << be16toh( udp_src_port ) << ", dest port = " << be16toh( udp_dest_port )
       << "\n";

  cerr << "\n";
}

int device_name_to_index( const string& device_name )
{
  unsigned int ret = if_nametoindex( device_name.c_str() );
  if ( ret == 0 ) {
    throw unix_error( "if_nametoindex" );
  }
  return ret;
}

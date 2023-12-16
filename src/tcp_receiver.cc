#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  // (void)message;
  // (void)reassembler;
  // (void)inbound_stream;

  if ( message.SYN ) {
    initialized = true;
    zero_point = message.seqno;
  }

  uint64_t check_point = inbound_stream.bytes_pushed() + inbound_stream.bytes_pushed();
  uint64_t first_index = message.seqno.unwrap( zero_point, check_point );

  reassembler.insert( first_index, message.payload, message.FIN, inbound_stream );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  // (void)inbound_stream;

  uint16_t window_size = inbound_stream.available_capacity();

  if ( !initialized ) {
    return TCPReceiverMessage( {}, window_size );
  }

  Wrap32 ackno = zero_point + inbound_stream.bytes_pushed() + 1;
  return TCPReceiverMessage( ackno, window_size );
}

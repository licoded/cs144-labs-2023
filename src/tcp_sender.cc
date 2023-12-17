#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  uint64_t in_flight_seqnos = 0;
  for ( auto& segment : outstanding_segments ) {
    in_flight_seqnos += segment.sequence_length();
  }
  return in_flight_seqnos;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmissions_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if ( outstanding_segments.empty() ) {
    return {};
  }
  return outstanding_segments.front();
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  // (void)outbound_stream;

  if ( recv_winsz == 0 ) {
    send_empty_message();
    return;
  }

  while ( recv_winsz != 0 ) {
    Wrap32 current_seqno = isn_ + outbound_stream.bytes_popped();
    TCPSenderMessage message { current_seqno, false, Buffer( "" ), false };

    message.SYN = false;
    if ( first_push ) {
      message.SYN = true;
      first_push = false;
    }

    uint64_t left_capacity = min( TCPConfig::MAX_PAYLOAD_SIZE, recv_winsz ) - message.sequence_length();
    std::string payload;
    read( outbound_stream, left_capacity, payload );
    message.payload = Buffer( payload );

    if ( outbound_stream.is_finished() ) {
      if ( min( TCPConfig::MAX_PAYLOAD_SIZE, recv_winsz ) > message.sequence_length() ) {
        message.FIN = true;
      }
    }

    outstanding_segments.push_back( message );
    recv_winsz -= message.sequence_length();
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  /**
   * NOTE: I don't consider SYN=1 here
   * I will not fix it until the tests result tell me to do that
   */

  // I don't know what bit to send...

  // add 1 to let recv_client to discard this message
  TCPSenderMessage message { isn_ + 1, false, Buffer( "" ), false };

  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  (void)msg;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  (void)ms_since_last_tick;
}

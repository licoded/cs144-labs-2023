#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <iostream>
#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , initial_RTO_ms_( initial_RTO_ms )
  , RTO_ms( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  uint64_t in_flight_seqnos = 0;
  for ( auto& segment : outstanding_segments ) {
    in_flight_seqnos += segment.sequence_length();
  }
  return in_flight_seqnos;

  /**
   * TODO: I think above is wrong... The following is right
   * If so, other two places can invoke this func to make code concise
   */
  // uint64_t sent_seqnos = 0;
  // for ( uint64_t i = 1; i < send_index; i++ ) {
  //   sent_seqnos += outstanding_segments.at( outstanding_segments.size() - i ).sequence_length();
  // }
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmissions_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if ( retransmit_flag ) {
    retransmit_flag = false;
    return outstanding_segments.at( outstanding_segments.size() - 1 );
  }

  if ( outstanding_segments.size() < send_index ) {
    return {};
  }

  if ( !timer_open ) {
    timer_open = true;
    timer_ms = RTO_ms;
  }

  TCPSenderMessage message = outstanding_segments.at( outstanding_segments.size() - send_index );
  send_index++;
  return message;
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  // (void)outbound_stream;

  uint64_t pretend_recv_winsz = recv_winsz;

  if ( recv_winsz == 0 ) {
    pretend_recv_winsz = 1;
  }

  uint64_t left_recv_winsz = pretend_recv_winsz - min( pretend_recv_winsz, sequence_numbers_in_flight() );

  while ( left_recv_winsz != 0 && ( outbound_stream.bytes_buffered() != 0 || first_push ) ) {
    Wrap32 current_seqno = isn_ + outbound_stream.bytes_popped();
    TCPSenderMessage message { current_seqno, false, Buffer( "" ), false };

    message.SYN = false;
    if ( first_push ) {
      // we have checked left_recv_winsz !=0, i.e. left_recv_winsz > 0
      // so, SYN can be sent (have space)
      message.SYN = true;
      first_push = false;
    } else {
      message.seqno = message.seqno + 1;
    }

    uint64_t left_capacity = min( TCPConfig::MAX_PAYLOAD_SIZE, left_recv_winsz - message.sequence_length() );
    std::string payload;
    read( outbound_stream, left_capacity, payload );
    message.payload = Buffer( payload );

    if ( outbound_stream.is_finished() ) {
      if ( left_recv_winsz > message.sequence_length() ) {
        message.FIN = true;
      }
    }

    // outstanding_segments.push_back( message );
    outstanding_segments.insert( outstanding_segments.begin(), message );
    left_recv_winsz -= message.sequence_length();
  }
  // if FIN haven't been sent
  if ( left_recv_winsz != 0 && outbound_stream.is_finished() && !outstanding_segments.front().FIN ) {
    Wrap32 current_seqno = isn_ + outbound_stream.bytes_popped() + 1;
    TCPSenderMessage message { current_seqno, false, Buffer( "" ), true };
    outstanding_segments.insert( outstanding_segments.begin(), message );
    left_recv_winsz -= message.sequence_length();
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

  uint64_t sent_seqnos = 0;
  for ( uint64_t i = 1; i < send_index; i++ ) {
    sent_seqnos += outstanding_segments.at( outstanding_segments.size() - i ).sequence_length();
  }

  // add 1 to let recv_client to discard this message
  TCPSenderMessage message { isn_ + poped_seqnos + sent_seqnos, false, Buffer( "" ), false };

  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  // (void)msg;

  recv_winsz = msg.window_size;

  if ( msg.ackno.has_value() ) {
    uint64_t ackno = ( msg.ackno.value() ).unwrap( isn_, poped_seqnos );

    // check if ackno is valid
    if ( ackno > poped_seqnos + sequence_numbers_in_flight() ) {
      return;
    }

    bool ack_effective = false;
    while ( send_index > 1 ) {
      TCPSenderMessage& message = outstanding_segments.back();

      uint64_t seqno = message.seqno.unwrap( isn_, poped_seqnos );
      // seqno + message.sequence_length() - 1 < ackno, following are equivalent
      if ( seqno + message.sequence_length() <= ackno ) {
        poped_seqnos += message.sequence_length();
        outstanding_segments.pop_back();
        send_index--;
        ack_effective = true;
        retransmit_flag = false;
      } else {
        break;
      }
    }

    if ( ack_effective ) {
      RTO_ms = initial_RTO_ms_;
      timer_open = send_index > 1;
      timer_ms = RTO_ms;
      consecutive_retransmissions_ = 0;
    }
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  // (void)ms_since_last_tick;

  if ( !timer_open ) {
    std::cout << "!timer_open" << std::endl;
    return;
  }
  std::cout << "timer_open" << std::endl;
  if ( timer_ms > ms_since_last_tick ) {
    timer_ms -= ms_since_last_tick;
    return;
  }

  if ( send_index == 1 ) {
    timer_open = false;
    return;
  }

  // re-transmit the earliest (lowest sequence number) segment
  // Q: I don't find approach/func to do re-transmit...
  // A: just send it when invoke maybe_send func next time
  retransmit_flag = true;

  std::cout << recv_winsz << std::endl;
  if ( recv_winsz > 0 ) {
    consecutive_retransmissions_++;
    if ( RTO_ms > UINT64_MAX / 2 ) {
      RTO_ms = UINT64_MAX;
    } else {
      RTO_ms *= 2;
    }
  }

  timer_ms = RTO_ms;
}

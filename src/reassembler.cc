#include "reassembler.hh"
#include <tuple>

using namespace std;

void Reassembler::push_to_writer( Writer& output, const Datagram& datagram )
{
  auto [first_index, data, is_last_substring] = datagram;
  output.push( data );
  next_index += data.size();

  if ( is_last_substring )
    output.close();
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  if ( first_index < next_index ) {
    if ( first_index + data.size() < next_index ) { // 10, 0-9, 10
      // IF case: the whole datagram is already received [repetition]
      return; // just discard this datagram if it is already received
    } else {  // 10, 10-19, 15
      // ELSE case: the datagram is partially received [overlap with previous already written datagram]
      data = data.substr( next_index - first_index );
      first_index = next_index;
      // continue to encounter `first_index == next_index` if condition
    }
  }

  bool out_of_range = first_index >= next_index + output.available_capacity(); // 10, 0-9, 10 is out of range
  if ( out_of_range )
    return; // just discard this datagram

  // 10, 10-19, 15, 19-15+1=19+1-15=10+10-15
  uint64_t in_range_size = min( data.size(), next_index + output.available_capacity() - first_index );
  data = data.substr( 0, in_range_size );
  if ( in_range_size < data.size() )
    is_last_substring = false;

  vector<Datagram> datagram_vec;
  while ( !datagram_queue.empty() ) {
    datagram_vec.push_back( datagram_queue.top() );
    datagram_queue.pop();
  }
  temporary_bytes = 0;

  uint64_t last_index = first_index + data.size() - 1;
  /**
   * ATTENTION: can overlap with multiple datagrams!!!
   */
  for ( u_int64_t i = 0; i < datagram_vec.size(); i++ ) {
    auto [first_index_, data_, is_last_substring_] = datagram_vec[i];
    uint64_t last_index_ = first_index_ + data_.size() - 1;
    bool is_overlap = ( first_index <= last_index_ ) && ( first_index_ <= last_index );
    if ( !is_overlap ) {
      datagram_queue.push( datagram_vec[i] );
      temporary_bytes += data_.size();
      continue;
    }
    is_last_substring = is_last_substring || is_last_substring_;
    // deal with the pre-part
    if ( first_index_ < first_index ) // extend data to the left
    {
      data = data_.substr( 0, first_index - first_index_ ) + data; // 10, 10-19, 15
      first_index = first_index_;
    }
    // deal with the post-part
    if ( last_index_ > last_index ) // extend data to the right
    {
      data = data + data_.substr( last_index + 1 - first_index_, last_index_ - last_index ); // 10, 10-19, 15
      last_index = last_index_;
    }
  }

  Datagram datagram = make_tuple( first_index, data, is_last_substring );

  if ( first_index == next_index ) {
    push_to_writer( output, datagram );

    while ( !datagram_queue.empty() ) {
      auto [first_index_, data_, is_last_substring_] = datagram_queue.top();

      if ( first_index_ > next_index )
        break;

      push_to_writer( output, datagram_queue.top() );
      datagram_queue.pop();
      temporary_bytes -= data_.size();
    }

    return;
  }

  datagram_queue.push( datagram );
  temporary_bytes += data.size();
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return temporary_bytes;
}

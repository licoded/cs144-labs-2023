#include "reassembler.hh"
#include <tuple>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  /**
   * ATTENTION: TODO: deal with the overlap case
   */

  if ( first_index < next_index ) {
    if ( first_index + data.size() < next_index ) { // 10, 0-9, 10
      return;                                       // just discard this datagram if it is already received
    } else {                                        // 10, 10-19, 15
      data = data.substr( next_index - first_index );
      first_index = next_index;
      // continue to encounter `first_index == next_index` if condition
    }
  }

  if ( first_index == next_index ) {
    /**
     * TODO: write to `Writer& output`
     *
     * ATTENTION: the maximum write size is limited by `Writer::available_capacity()`
     * ATTENTION: after writing this incoming datagram, we should check and pop some datagrams in datagram_queue
     */
    // write to `Writer& output`
    uint64_t true_write_size = min( data.size(), output.available_capacity() );
    output.push( data.substr( 0, true_write_size ) );
    next_index += true_write_size;

    if ( is_last_substring && true_write_size == data.size() ) {
      output.close();
    }

    // check and pop some datagrams in datagram_queue
    while ( output.available_capacity() > 0 && !datagram_queue.empty() ) {
      auto [index, datagram, is_end] = datagram_queue.top();
      datagram_queue.pop();

      if ( index < next_index ) {
        if ( index + datagram.size() <= next_index ) { // 10, 10-19, 20
          continue;                                    // just discard this datagram if it is already received
        } else {
          datagram = datagram.substr( next_index - index );
          output.push( datagram );
          next_index += datagram.size();
        }
        temporary_bytes -= datagram.size();
        continue;
      } else if ( index == next_index ) {
        /**
         * NOTE: all datagrams in `datagram_queue` are in capacity
         */
        output.push( datagram );
        if ( is_end ) {
          output.close();
        }
        next_index += datagram.size();
        temporary_bytes -= datagram.size();
      } else // index > next_index
      {
        datagram_queue.push( tuple( index, datagram, is_end ) );
        break;
      } // END else
    }   // END while

    return;
  }

  /**
   * judge if the `first_index` is in the range of `next_index` and `next_index + Writer::remaining_capacity()`
   * - If yes, store the data in `datagram_queue`
   * - Otherwise, discard the data
   */
  bool out_of_range = first_index >= next_index + output.available_capacity(); // 10, 0-9, 10 is out of range
  if ( out_of_range )
    return; // just discard this datagram

  // 10, 10-19, 15, 19-15+1=19+1-15=10+10-15
  uint64_t in_range_size = min( data.size(), next_index + output.available_capacity() - first_index );
  data = data.substr( 0, in_range_size );

  /**
   * Store the data in `datagram_queue`
   */
  // TODO: check capacity is enough to store the whole datagram
  // pop all elems in datagram_queue, and push them into a vector
  vector<Datagram> datagram_vec;
  // vector<Datagram> datagram_start_vec, datagram_end_vec;
  while ( !datagram_queue.empty() ) {
    datagram_vec.push_back( datagram_queue.top() );

    // auto [index, datagram, is_end] = datagram_queue.top();
    // datagram_start_vec.push_back( make_tuple( index, datagram, is_end ) );
    // datagram_end_vec.push_back( make_tuple( index + datagram.size() - 1, datagram, is_end ) );

    datagram_queue.pop();
    // temporary_bytes -= datagram.size();
  }
  temporary_bytes = 0;

  uint64_t last_index = first_index + data.size() - 1;
  // bool has_overlap = false; // SEEMs useless, as we always push the incoming datagram into datagram_queue
  //                           // in the last (after the following for loop)
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
    // has_overlap = true;
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

  datagram_queue.push( make_tuple( first_index, data, is_last_substring ) );
  temporary_bytes += data.size();
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return temporary_bytes;
}

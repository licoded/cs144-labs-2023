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
    return; // just discard this datagram if it is already received
            // In fact, this datagram may could be partially accepted
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

    if ( is_last_substring && true_write_size == data.size() ) {
      output.close();
    }

    // check and pop some datagrams in datagram_queue
    while ( output.available_capacity() > 0 && !datagram_queue.empty() ) {
      auto [index, datagram, is_end] = datagram_queue.top();
      datagram_queue.pop();

      if ( index > next_index ) {
        temporary_bytes -= datagram.size();
        continue; // just discard this datagram if it is already received
                  // In fact, this datagram may could be partially accepted
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
      } else // index < next_index
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

  bool out_of_capacity
    = first_index + data.size() > next_index + output.available_capacity(); // 10, 10-19, 10, 10-19 is in capacity
  if ( out_of_capacity )
    return; // also discard only if we cannot accept the whole datagram, even if can partially accept

  /**
   * Store the data in `datagram_queue`
   */
  // TODO: check capacity is enough to store the whole datagram
  datagram_queue.push( make_tuple( first_index, data, is_last_substring ) );
  temporary_bytes += data.size();
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return temporary_bytes;
}

#include "reassembler.hh"

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
  datagram_queue.push( make_pair( first_index, data ) );
  temporary_bytes += data.size();
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return temporary_bytes;
}

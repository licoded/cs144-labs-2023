#pragma once

#include "byte_stream.hh"

#include <string>

class Reassembler
{
  using Datagram = std::tuple<uint64_t, std::string, bool>;

  std::priority_queue<Datagram, std::vector<Datagram>, std::greater<>> datagram_queue {};
  uint64_t next_index = 0;
  uint64_t temporary_bytes = 0;

public:
  /**
   * Write datagram to output. (should be checked in `insert` func)
   * Deal with `next_index` variable, and close the stream if reach end.
   */
  void push_to_writer( Writer& output, const Datagram& datagram );

  /**
   * Merge current datagram with existing datagrams, which is overlapped with current datagram, in queue.
   * ATTENTION: can overlap with multiple datagrams!!!
   */
  void extend( Datagram& datagram );

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;
};

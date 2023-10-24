#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

/**
 * An in-memory reliable byte stream
 *
 * REQUIREMENTS:
 *  - in the same sequence, from "input" side to "output" side
 *  - the writer can end the input, and then no more bytes can be written
 *  - once the "output" side reach "EOF", it cannot be read
 *
 *  - should be flow-controlled to limit its memory consumption
 *    - The object is initialized with a particular “capacity”: the maximum number of bytes it’s willing to store
 *    - The byte stream will limit the writer in how much it can write at any given moment
 *    - As the reader reads bytes and drains them from the stream, the writer is allowed to write more.
 *    - Your byte stream is for use in a single thread, and so have no concurrency concerns. (TODO)
 *
 * ATTENTION: (To be clear)
 *  Your implementation must be able to handle streams that are much longer than the capacity.
 *  The capacity limits the number of bytes that are held in memory (written but not yet read) at a given point,
 *  but does not limit the length of the stream.
 *
 * FUTUREWORK:
 *  - multiple threads, should consider concurrency
 *  - speed/effeciency optimization
 *
 * WORKFLOW:
 *  1. just run `make`, and you need to fix all errors
 *  2. manual test: a little difficult
 *  3. automated test: `cmake --build build --target check0`
 */

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), remain_capacity_( capacity ) {}

void Writer::push( const string& data )
{
  // Your code here.
  if ( closed_ ) {
    return;
  }
  const uint64_t true_write_length = min( remain_capacity_, data.size() );
  buffer_ += data.substr( 0, true_write_length );
  total_written_ += true_write_length;
  remain_capacity_ -= true_write_length;
}

void Writer::close()
{
  // Your code here.
  if ( closed_ ) {
    return;
  }
  closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  is_error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return remain_capacity_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return total_written_;
}

string_view Reader::peek() const
{
  // Your code here.
  return buffer_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ && buffer_.empty();
}

bool Reader::has_error() const
{
  // Your code here.
  return is_error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  const uint64_t true_pop_length = min( len, buffer_.size() );
  buffer_.erase( 0, true_pop_length );
  total_read_ += true_pop_length;
  remain_capacity_ += true_pop_length;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return capacity_ - remain_capacity_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return total_read_;
}

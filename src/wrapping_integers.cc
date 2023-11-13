#include "wrapping_integers.hh"
#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // get k to make |2^32*k+zero_point - checkpoint| minimal, and checkpoint = 2^32*q+r
  // that is to say, get k to make |2^32*(k-q)+zero_point-r| minimal

  uint64_t mod = 1UL << 32;
  uint64_t seqno = ( raw_value_ - zero_point.raw_value_ + mod ) % mod;

  uint64_t q = checkpoint >> 32;
  // uint64_t r = checkpoint-(q<<32);
  uint64_t r = checkpoint % mod;
  // uint64_t d = ( mod + seqno - r ) % mod;

  // uint64_t k = q + ( d > mod - d );

  // std::cout << r << "\t" << d << "\t" << k << std::endl;

  uint64_t d;
  uint64_t k = q;

  if ( r > seqno ) {
    d = ( mod + r - seqno ) % mod;
    if ( k != ( 1UL << 32 ) - 1 && d > mod - d )
      k++;
  } else if ( r < seqno ) {
    d = ( mod + seqno - r ) % mod;
    if ( k != 0 && d > mod - d )
      k--;
  } else if ( r == seqno ) {
    k = q;
  }

  return k * mod + seqno;
}

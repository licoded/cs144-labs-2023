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

  const uint64_t mod = 1UL << 32;
  const uint64_t seqno = ( raw_value_ - zero_point.raw_value_ + mod ) % mod;

  const uint64_t r = checkpoint % mod;

  uint64_t k = checkpoint >> 32;

  const uint64_t d = ( max( r, seqno ) - min( r, seqno ) ) % mod;

  // if (d > mod - d)
  if ( d > ( mod / 2 ) ) {
    if ( r > seqno ) {
      if ( k != ( 1UL << 32 ) - 1 ) {
        k++;
      }
    } else if ( r < seqno ) {
      if ( k != 0 ) {
        k--;
      }
    }
  }

  return k * mod + seqno;
}

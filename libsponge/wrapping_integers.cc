#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

static constexpr uint64_t kI32 = (1ul << 32);

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number

//! \brief abs_seqno -> abs_seqnoseqno
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    DUMMY_CODE(n, isn);
    return WrappingInt32(n + isn.raw_value());
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.

//! TODO: too complex, maybe can use bit operation
//! \brief seqno -> abs_seqno
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    DUMMY_CODE(n, isn, checkpoint);
    uint64_t seqno = n.raw_value();
    uint64_t offset = (seqno - isn.raw_value() + kI32) % kI32;
    uint64_t x = checkpoint / kI32;
    if (x)
        x--;
    uint64_t resp = 0, diff = kI32;
    for (int i = 0; i < 3; ++i) {
        uint64_t t = offset + x * kI32;
        t = t > checkpoint ? t - checkpoint : checkpoint - t;
        if (t <= diff) {
            diff = t;
            resp = offset + x * kI32;
        }
        ++x;
    }
    return resp;
}

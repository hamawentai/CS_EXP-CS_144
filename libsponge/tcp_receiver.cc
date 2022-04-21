#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    DUMMY_CODE(seg);
    TCPHeader header = seg.header();
    if (!_has_syn) {
        if (!header.syn) {
            return;
        }
        _has_syn = true;
        _isn = header.seqno;
    }
    size_t ackno = _reassembler.get_exp_index();
    uint64_t abs_seqno = unwrap(header.seqno, _isn, ackno);
    // syn占了seqno的一个位置，其他的seqno要往前移一个位置
    uint64_t seq_index = abs_seqno - 1 + (header.syn);
    _reassembler.push_substring(seg.payload().copy(), seq_index, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_has_syn)
        return nullopt;
    // syn
    uint64_t abs_ack_no = _reassembler.stream_out().bytes_written() + 1;
    // fin
    if (_reassembler.stream_out().input_ended())
        ++abs_ack_no;
    // 超过32位相当于取余了
    return WrappingInt32(_isn) + abs_ack_no;
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }

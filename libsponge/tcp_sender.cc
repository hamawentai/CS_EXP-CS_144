#include "tcp_sender.hh"

#include <random>

#include "tcp_config.hh"

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs&&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()})), _initial_retransmission_timeout{retx_timeout}, _stream(capacity), _timer(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _unconfirmed_size; }

void TCPSender::fill_window() {
    size_t win_size = max(_win_size, 1ul);
    while (win_size > _unconfirmed_size) {
        TCPSegment tcp_segment;
        // 没有发送过syn
        if (!_send_syn) {
            _send_syn = tcp_segment.header().syn = true;
        }
        // win_size - 1 >= _unconfirmed_size
        size_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, win_size - _unconfirmed_size - tcp_segment.header().syn);
        // cout << "win_size-> " << win_size << " pay_size-> " << payload_size << endl;
        tcp_segment.payload() = Buffer(_stream.read(payload_size));
        tcp_segment.header().seqno = next_seqno();
        // 设置fin
        if (!_send_fin && _stream.eof() && win_size - tcp_segment.length_in_sequence_space() - _unconfirmed_size > 0) {
            _send_fin = tcp_segment.header().fin = true;
        }
        size_t seg_len = tcp_segment.length_in_sequence_space();
        // 无数据
        if (seg_len == 0) {
            return;
        }
        if (_unconfirmed_queue.empty()) {
            _timer.reset(_initial_retransmission_timeout);
            _timer.start();
        }
        _next_seqno += tcp_segment.length_in_sequence_space();
        _segments_out.push(tcp_segment);
        _unconfirmed_queue.push(tcp_segment);
        _unconfirmed_size += seg_len;
        if (_send_fin) {
            return;
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    DUMMY_CODE(ackno, window_size);
    uint64_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    if (abs_ackno > _next_seqno) {
        return;
    }
    while (!_unconfirmed_queue.empty()) {
        const TCPSegment& tcp_segment = _unconfirmed_queue.front();
        if (unwrap(tcp_segment.header().seqno + tcp_segment.length_in_sequence_space(), _isn, _next_seqno) <= abs_ackno) {
            _unconfirmed_size -= tcp_segment.length_in_sequence_space();
            _unconfirmed_queue.pop();
            _timer.reset(_initial_retransmission_timeout);
            _timer.start();
            _consecutive_retransmissions_cnt = 0;
        } else {
            break;
        }
    }
    _win_size = window_size;
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    DUMMY_CODE(ms_since_last_tick);
    if (_unconfirmed_queue.empty()) {
        return;
    }
    _timer.run(ms_since_last_tick);
    // 超时->计数器当前时间重置
    if (_timer.is_expired()) {
        // 窗口仍有剩余但发生了拥塞->指数避让
        if (_win_size != 0) {
            _timer.exponential_backoff();
        }
        _timer.start();
        // 重传
        _segments_out.push(_unconfirmed_queue.front());
        ++_consecutive_retransmissions_cnt;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const {
    return _consecutive_retransmissions_cnt;
}

void TCPSender::send_empty_segment() {
    TCPSegment tcp_segment;
    tcp_segment.header().seqno = next_seqno();
    _segments_out.push(tcp_segment);
}

#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t cap) {
    size_ = cap + 1;
    cap_ = cap;
    tail_ = head_ = 0;
    buffer_.assign(size_, ' ');
    cnt_read_ = cnt_write_ = 0;
}

void ByteStream::push(char c) {
    buffer_[tail_] = c;
    tail_ = (tail_ + 1) % size_;
    cnt_write_++;
}

char ByteStream::pop_front() {
    char c = buffer_[head_];
    head_ = (head_ + 1) % size_;
    cnt_read_++;
    return c;
}

size_t ByteStream::write(const string &data) {
    if (end_input_)
        return 0;
    size_t t = min(remaining_capacity(), data.size());
    for (size_t i = 0; i < t; ++i) {
        push(data[i]);
    }
    return t;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t t = min(len, buffer_size()), head = head_;
    string str;
    for (size_t i = 0; i < t; ++i) {
        str = str + buffer_[head];
        head = (head + 1) % size_;
    }
    return str;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t t = min(len, buffer_size());
    for (size_t i = 0; i < t; ++i) {
        pop_front();
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t t = min(len, buffer_size());
    string str;
    for (size_t i = 0; i < t; ++t) {
        str = str + pop_front();
    }
    return str;
}

void ByteStream::end_input() { end_input_ = true; }

bool ByteStream::input_ended() const { return end_input_; }

size_t ByteStream::buffer_size() const { return (tail_ - head_ + size_) % size_; }

bool ByteStream::buffer_empty() const { return tail_ == head_; }

bool ByteStream::eof() const { return end_input_ && buffer_empty(); }

size_t ByteStream::bytes_written() const { return cnt_write_; }

size_t ByteStream::bytes_read() const { return cnt_read_; }

size_t ByteStream::remaining_capacity() const { return cap_ - buffer_size(); }
#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    DUMMY_CODE(data, index, eof);
    if (eof) {
        _is_eof = true;
        _eof_index = index + data.size();
    }
    insert_map(data, index);
    // key
    if (index <= _exp_index && index + data.size() >= _exp_index) {
        merge_interval(_exp_index);
    }
}

void StreamReassembler::insert_map(const string &data, const size_t index) {
    auto it = _un_reassemb_bf.find(index);
    if (it == _un_reassemb_bf.end() || it->second.size() < data.size()) {
        _un_reassemb_bf[index] = data;
    }
}
void StreamReassembler::merge_interval(const size_t index) {
    size_t idx = index, real_len, t_index;
    string str, t_str;
    bool flag = false;
    auto it = _un_reassemb_bf.begin();
    for (; it != _un_reassemb_bf.end(); ++it) {
        if (it->first <= idx) {
            size_t str_size = it->second.size();
            if (idx < it->first + str_size) {
                idx = it->first + str_size;
                real_len = str_size + it->first - _exp_index;
                str = it->second.substr(_exp_index - it->first, real_len);
                size_t acc_size = _output.write(str);
                _exp_index += acc_size;
                if (acc_size < real_len) {
                    t_str = it->second.substr(acc_size);
                    t_index = it->first + acc_size;
                    flag = true;
                    ++it;
                    break;
                }
            }
        } else {
            break;
        }
    }
    _un_reassemb_bf.erase(_un_reassemb_bf.begin(), it);
    if (flag) {
        insert_map(t_str, t_index);
    }
    if (_is_eof && _eof_index <= _exp_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t exp = 0, cnt = 0;
    for (auto x : _un_reassemb_bf) {
        size_t new_exp = x.first + x.second.size();
        if (x.first >= exp) {
            cnt += x.second.size();
        } else if (new_exp > exp) {
            cnt += (new_exp - exp);
        }
        exp = new_exp;
    }
    return cnt;
}

bool StreamReassembler::empty() const { return _is_eof && _un_reassemb_bf.empty(); }

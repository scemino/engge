#include <cstring>
#include "Parsers/GGPackBufferStream.hpp"

GGPackBufferStream::GGPackBufferStream(std::vector<char> input) : _input(std::move(input)) {}

void GGPackBufferStream::setBuffer(const std::vector<char> &input) {
  _input = input;
  _offset = 0;
}

void GGPackBufferStream::read(char *data, size_t size) {
  if ((static_cast<int>(_offset + size)) > getLength())
    return;
  memcpy(data, _input.data() + _offset, size);
  _offset += size;
}

void GGPackBufferStream::seek(int pos) {
  _offset = pos;
}

int GGPackBufferStream::getLength() const {
  return _input.size();
}

int GGPackBufferStream::tell() {
  return _offset;
}

bool GGPackBufferStream::eof() const {
  return _offset == static_cast<int>(_input.size());
}

char GGPackBufferStream::peek() const {
  return _offset >= _input.size() ? EOF : _input.at(_offset);
}

GGPackBufferStream &GGPackBufferStream::ignore(std::streamsize n, int delim) {
  for (int i = 0; i < n && _offset < static_cast<int>(_input.size()); i++) {
    if (_input[_offset++] == delim)
      return *this;
  }
  return *this;
}
#include <cstring>
#include "engge/Parsers/GGPackBufferStream.hpp"

GGPackBufferStream::GGPackBufferStream(std::vector<char> input) : m_input(std::move(input)) {}

void GGPackBufferStream::setBuffer(const std::vector<char> &input) {
  m_input = input;
  m_offset = 0;
}

void GGPackBufferStream::read(char *data, size_t size) {
  if ((static_cast<int>(m_offset + size)) > getLength())
    return;
  memcpy(data, m_input.data() + m_offset, size);
  m_offset += size;
}

void GGPackBufferStream::seek(int pos) {
  m_offset = pos;
}

int GGPackBufferStream::getLength() const {
  return m_input.size();
}

int GGPackBufferStream::tell() {
  return m_offset;
}

bool GGPackBufferStream::eof() const {
  return m_offset == static_cast<int>(m_input.size());
}

char GGPackBufferStream::peek() const {
  return m_offset >= m_input.size() ? EOF : m_input.at(m_offset);
}

GGPackBufferStream &GGPackBufferStream::ignore(std::streamsize n, int delim) {
  for (int i = 0; i < n && m_offset < static_cast<int>(m_input.size()); i++) {
    if (m_input[m_offset++] == delim)
      return *this;
  }
  return *this;
}
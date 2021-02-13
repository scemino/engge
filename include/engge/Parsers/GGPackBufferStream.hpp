#pragma once
#include <ios>
#include <vector>
#include "GGPackStream.hpp"

class GGPackBufferStream : public GGPackStream {
public:
  GGPackBufferStream() = default;
  explicit GGPackBufferStream(std::vector<char> input);

  void setBuffer(const std::vector<char> &input);
  void read(char *data, size_t size) override;
  void seek(int pos) override;
  [[nodiscard]] int getLength() const override;
  int tell() override;
  [[nodiscard]] bool eof() const override;
  [[nodiscard]] char peek() const override;
  GGPackBufferStream &ignore(std::streamsize n = 1, int delim = EOF);

private:
  std::vector<char> m_input;
  int m_offset{0};
};

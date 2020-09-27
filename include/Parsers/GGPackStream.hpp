#pragma once
#include <cstdio>

class GGPackStream {
public:
  virtual void read(char *data, size_t size) = 0;
  virtual void seek(int pos) = 0;
  virtual int tell() = 0;
  [[nodiscard]] virtual int getLength() const = 0;
  [[nodiscard]] virtual bool eof() const = 0;
  [[nodiscard]] virtual char peek() const = 0;
};

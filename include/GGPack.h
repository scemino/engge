#include <utility>

#pragma once
#include <cstring>
#include <codecvt>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

namespace ng
{
struct GGPackEntry
{
  int offset;
  int size;
};

struct GGPackValue
{
  char type;
  std::string string_value;
  int int_value{0};
  double double_value{0};
  std::map<std::string, GGPackValue> hash_value;
  std::vector<GGPackValue> array_value;
  static GGPackValue nullValue;

public:
  GGPackValue();
  GGPackValue(const GGPackValue &value);

  bool isNull() const;
  bool isHash() const;
  bool isArray() const;
  bool isString() const;
  bool isInteger() const;
  bool isDouble() const;

  GGPackValue &operator[](std::size_t index);
  GGPackValue &operator[](const std::string &key);
  GGPackValue &operator=(const GGPackValue &other);
  virtual ~GGPackValue();
};
class GGPackStream
{
public:
  virtual void read(char *data, size_t size) = 0;
  virtual void seek(int pos) = 0;
  virtual int tell() = 0;
  virtual int getLength() const = 0;
  virtual bool eof() const = 0;
  virtual char peek() const = 0;
};

class GGPackBufferStream : public GGPackStream
{
public:
  GGPackBufferStream() = default;
  explicit GGPackBufferStream(std::vector<char> input) : _input(std::move(input)) {}

  void setBuffer(const std::vector<char> &input)
  {
    _input = input;
    _offset = 0;
  }
  void read(char *data, size_t size) override
  {
    if ((_offset + size) > getLength())
      return;
    memcpy(data, _input.data() + _offset, size);
    _offset += size;
  }
  void seek(int pos) override
  {
    _offset = pos;
  }
  int getLength() const override
  {
    return _input.size();
  }
  int tell() override
  {
    return _offset;
  }
  bool eof() const override
  {
    return _offset == _input.size();
  }
  char peek() const override
  {
    return _input[_offset];
  }
  GGPackBufferStream &ignore(std::streamsize n = 1, int delim = EOF)
  {
    for (int i = 0; i < n; i++)
    {
      if (_input[_offset++] == delim)
        return *this;
    }
    return *this;
  }

private:
  std::vector<char> _input;
  int _offset{0};
};

class GGPack
{
public:
  GGPack();

  void open(const std::string &path);
  bool hasEntry(const std::string &name);
  void readEntry(const std::string &name, std::vector<char> &data);
  void readHashEntry(const std::string &name, GGPackValue &value);

private:
  void readPack();
  void readHash(std::vector<char> &buffer, GGPackValue &value);
  void readString(int offset, std::string &key);
  void readHash(GGPackValue &value);
  void readValue(GGPackValue &value);
  char *decodeUnbreakableXor(char *buffer, int length);
  void getOffsets();

private:
  struct CaseInsensitiveCompare
  {
    bool operator()(const std::string &a, const std::string &b) const noexcept
    {
      return ::strcasecmp(a.c_str(), b.c_str()) < 0;
    }
  };

private:
  std::ifstream _input;
  GGPackBufferStream _bufferStream;
  std::vector<int> _offsets;
  std::map<std::string, GGPackEntry, CaseInsensitiveCompare> _entries;
  int _method;
};
} // namespace ng

#pragma once
#include <fstream>
#include <map>
#include <vector>
#include "GGPackStream.hpp"
#include "GGPackBufferStream.hpp"
#include "GGPackValue.hpp"

namespace ng {
struct GGPackEntry {
  int offset;
  int size;
};

class GGPack {
public:
  GGPack();

  void open(const std::string &path);
  void getEntries(std::vector<std::string> &entries);
  bool hasEntry(const std::string &name);
  void readEntry(const std::string &name, std::vector<char> &data);
  void readHashEntry(const std::string &name, GGPackValue &value);

private:
  void readPack();
  char *decodeUnbreakableXor(char *buffer, int length);

private:
  struct CaseInsensitiveCompare {
    bool operator()(const std::string &a, const std::string &b) const noexcept {
#ifdef WIN32
      return _stricmp(a.c_str(), b.c_str()) < 0;
#else
      return ::strcasecmp(a.c_str(), b.c_str()) < 0;
#endif
    }
  };

private:
  std::ifstream _input;
  std::map<std::string, GGPackEntry, CaseInsensitiveCompare> _entries;
  int _method{0};
};
} // namespace ng
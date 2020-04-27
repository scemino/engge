#include "Parsers/GGPackValue.hpp"

namespace ng {
GGPackValue GGPackValue::nullValue;

GGPackValue::GGPackValue() { type = 1; }
GGPackValue::GGPackValue(const GGPackValue &value)
    : type(value.type) {
  switch (type) {
  case 1:break;
  case 2:
    // hash
    hash_value = value.hash_value;
    break;
  case 3:
    // array
    array_value = value.array_value;
    break;
  case 4:
    // string
    string_value = value.string_value;
    break;
  case 5:int_value = value.int_value;
    break;
  case 6:
    // double
    double_value = value.double_value;
    break;
  }
}

bool GGPackValue::isNull() const { return type == 1; }
bool GGPackValue::isHash() const { return type == 2; }
bool GGPackValue::isArray() const { return type == 3; }
bool GGPackValue::isString() const { return type == 4; }
bool GGPackValue::isInteger() const { return type == 5; }
bool GGPackValue::isDouble() const { return type == 6; }

GGPackValue &GGPackValue::operator[](std::size_t index) {
  if (type == 3)
    return array_value[index];
  throw std::logic_error("This is not an array");
}

const GGPackValue &GGPackValue::operator[](std::size_t index) const {
  if (type == 3)
    return array_value[index];
  throw std::logic_error("This is not an array");
}

GGPackValue &GGPackValue::operator[](const std::string &key) {
  if (type == 2) {
    if (hash_value.find(key) == hash_value.end())
      return nullValue;
    return hash_value[key];
  }
  throw std::logic_error("This is not an hashtable");
}

const GGPackValue &GGPackValue::operator[](const std::string &key) const {
  if (type == 2) {
    if (hash_value.find(key) == hash_value.end())
      return nullValue;
    return hash_value.at(key);
  }
  throw std::logic_error("This is not an hashtable");
}

GGPackValue &GGPackValue::operator=(const GGPackValue &other) {
  if (this != &other) { // protect against invalid self-assignment
    type = other.type;
    switch (type) {
    case 1:break;
    case 2:
      // hash
      hash_value = other.hash_value;
      break;
    case 3:
      // array
      array_value = other.array_value;
      break;
    case 4:
      // string
      string_value = other.string_value;
      break;
    case 5:int_value = other.int_value;
      break;
    case 6: {
      // double
      double_value = other.double_value;
      break;
    }
    }
  }
  // by convention, always return *this
  return *this;
}

int GGPackValue::getInt() const {
  if (isInteger())
    return int_value;
  if (isDouble())
    return static_cast<int>(double_value);
  return 0;
}

double GGPackValue::getDouble() const {
  if (isDouble())
    return double_value;
  if (isInteger())
    return static_cast<double>(int_value);
  return 0;
}

std::string GGPackValue::getString() const {
  if (isString())
    return string_value;
  return "";
}

template<>
GGPackValue GGPackValue::toGGPackValue<int>(int value) {
  GGPackValue packValue;
  packValue.type = 5;
  packValue.int_value = value;
  return packValue;
}

template<>
GGPackValue GGPackValue::toGGPackValue<bool>(bool value) {
  GGPackValue packValue;
  packValue.type = 5;
  packValue.int_value = value ? 1 : 0;
  return packValue;
}

template<>
GGPackValue GGPackValue::toGGPackValue<float>(float value) {
  GGPackValue packValue;
  packValue.type = 6;
  packValue.double_value = value;
  return packValue;
}

template<>
GGPackValue GGPackValue::toGGPackValue<std::string>(std::string value) {
  GGPackValue packValue;
  packValue.type = 4;
  packValue.string_value = std::move(value);
  return packValue;
}

template<>
GGPackValue GGPackValue::toGGPackValue<std::nullptr_t>(std::nullptr_t) {
  GGPackValue packValue;
  packValue.type = 1;
  return packValue;
}

static std::ostream &_dumpValue(std::ostream &os, const GGPackValue &value, int indent);

static std::ostream &_dumpHash(std::ostream &os, const GGPackValue &value, int indent) {
  indent++;
  std::string padding(indent * 2, ' ');
  os << "{";
  for (auto iterator = value.hash_value.begin(); iterator != value.hash_value.end();) {
    os << std::endl << padding << "\"" << iterator->first << "\": ";
    _dumpValue(os, iterator->second, indent);
    if (++iterator != value.hash_value.end()) {
      os << ",";
    }
  }
  indent--;
  padding = std::string(indent * 2, ' ');
  os << std::endl << padding << "}";
  return os;
}

static std::ostream &_dumpArray(std::ostream &os, const GGPackValue &value, int indent) {
  indent++;
  std::string padding(indent * 2, ' ');
  os << "[";
  for (auto iterator = value.array_value.begin(); iterator != value.array_value.end();) {
    os << std::endl << padding;
    _dumpValue(os, *iterator, indent);
    if (++iterator != value.array_value.end()) {
      os << ",";
    }
  }
  indent--;
  padding = std::string(indent * 2, ' ');
  os << std::endl << padding << "]";
  return os;
}

static std::ostream &_dumpValue(std::ostream &os, const GGPackValue &value, int indent) {
  if (value.isHash()) {
    _dumpHash(os, value, indent);
    return os;
  }
  if (value.isArray()) {
    _dumpArray(os, value, indent);
    return os;
  }
  if (value.isDouble()) {
    os << value.double_value;
    return os;
  }
  if (value.isInteger()) {
    os << value.int_value;
    return os;
  }
  if (value.isNull()) {
    os << "null";
    return os;
  }
  if (value.isString()) {
    os << "\"" << value.string_value << "\"";
    return os;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const GGPackValue &value) {
  return _dumpValue(os, value, 0);
}

GGPackValue::~GGPackValue() = default;
}

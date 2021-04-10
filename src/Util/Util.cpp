#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"
#include "../../extlibs/squirrel/squirrel/sqarray.h"
#include "../../extlibs/squirrel/squirrel/sqfuncproto.h"
#include "../../extlibs/squirrel/squirrel/sqclosure.h"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include <codecvt>
#include <ngf/IO/GGPackValue.h>
#include "Util.hpp"
#include "engge/System/Locator.hpp"
#include "engge/Engine/Preferences.hpp"

namespace ng {

namespace {
constexpr const char *objectKey = "_objectKey";
constexpr const char *roomKey = "_roomKey";
constexpr const char *actorKey = "_actorKey";
constexpr const char *idKey = "_id";

ngf::GGPackValue toGGPackValue(SQObject obj, bool checkId, const std::string &tableKey = "");

bool canSave(HSQOBJECT obj) {
  switch (sq_type(obj)) {
  case OT_STRING: return true;
  case OT_INTEGER: return true;
  case OT_BOOL:return true;
  case OT_FLOAT:return true;
  case OT_NULL:return true;
  case OT_TABLE:return true;
  case OT_ARRAY:return true;
  default:return false;
  }
}

ngf::GGPackValue toArray(HSQOBJECT obj) {
  ngf::GGPackValue array;
  SQObjectPtr refpos;
  SQObjectPtr outkey, outvar;
  SQInteger res;
  while ((res = obj._unVal.pArray->Next(refpos, outkey, outvar)) != -1) {
    if (canSave(outvar)) {
      array.push_back(toGGPackValue(outvar, true));
    }
    refpos._type = OT_INTEGER;
    refpos._unVal.nInteger = res;
  }
  return array;
}

ngf::GGPackValue toTable(HSQOBJECT table, bool checkId, const std::string &tableKey = "") {
  ngf::GGPackValue hash;
  int id;
  if (checkId && ng::ScriptEngine::get(table, idKey, id)) {
    if (ng::EntityManager::isActor(id)) {
      auto pActor = ng::EntityManager::getActorFromId(id);
      if (pActor && pActor->getKey() != tableKey) {
        hash[actorKey] = pActor->getKey();
        return hash;
      }
      return nullptr;
    }
    if (ng::EntityManager::isObject(id)) {
      auto pObj = ng::EntityManager::getObjectFromId(id);
      if (pObj && pObj->getKey() != tableKey) {
        auto pRoom = pObj->getRoom();
        if (pRoom && pRoom->isPseudoRoom()) {
          hash[roomKey] = pRoom->getName();
        }
        hash[objectKey] = pObj->getKey();
        return hash;
      }
      return nullptr;
    }
    if (ng::EntityManager::isRoom(id)) {
      auto pRoom = ng::EntityManager::getRoomFromId(id);
      if (pRoom && pRoom->getName() != tableKey) {
        hash[roomKey] = pRoom->getName();
        return hash;
      }
      return nullptr;
    }
    return nullptr;
  }

  SQObjectPtr refpos;
  SQObjectPtr outkey, outvar;
  SQInteger res;
  while ((res = table._unVal.pTable->Next(false, refpos, outkey, outvar)) != -1) {
    std::string key = _stringval(outkey);
    if (!key.empty() && key[0] != '_' && canSave(outvar)) {
      auto value = toGGPackValue(outvar, true, key);
      if (!value.isNull()) {
        hash[key] = value;
      }
    }
    refpos._type = OT_INTEGER;
    refpos._unVal.nInteger = res;
  }
  return hash;
}

ngf::GGPackValue toGGPackValue(SQObject obj, bool checkId, const std::string &tableKey) {
  switch (sq_type(obj)) {
  case OT_STRING:return _stringval(obj);
  case OT_INTEGER:
  case OT_BOOL:return static_cast<int>(_integer(obj));
  case OT_FLOAT:return static_cast<float >(_float(obj));
  case OT_NULL:return nullptr;
  case OT_TABLE: return toTable(obj, checkId, tableKey);
  case OT_ARRAY: {
    return toArray(obj);
  }
  default:assert(false);
  }
}
}

std::string str_toupper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return ::toupper(c); } // correct
  );
  return s;
}

void replaceAll(std::string &text, const std::string &search, const std::string &replace) {
  auto pos = text.find(search);
  while (pos != std::string::npos) {
    text.replace(pos, search.size(), replace);
    pos = text.find(search, pos + replace.size());
  }
}

void replaceAll(std::wstring &text, const std::wstring &search, const std::wstring &replace) {
  auto pos = text.find(search);
  while (pos != std::wstring::npos) {
    text.replace(pos, search.size(), replace);
    pos = text.find(search, pos + replace.size());
  }
}

void removeFirstParenthesis(std::wstring &text) {
  if (text.size() < 2)
    return;
  if (text.find(L'(') != 0)
    return;
  auto pos = text.find(L')');
  if (pos == std::wstring::npos)
    return;
  text = text.substr(pos + 1);
}

bool startsWith(const std::string &str, const std::string &prefix) {
  return str.length() >= prefix.length() && 0 == str.compare(0, prefix.length(), prefix);
}

bool endsWith(const std::string &str, const std::string &suffix) {
  return str.length() >= suffix.length() && 0 == str.compare(str.length() - suffix.length(), suffix.length(), suffix);
}

void checkLanguage(std::string &str) {
  if (endsWith(str, "_en")) {
    const auto &lang = Locator<Preferences>::get().getUserPreference<std::string>(PreferenceNames::Language,
                                                                                  PreferenceDefaultValues::Language);
    str = str.substr(0, str.length() - 3).append("_").append(lang);
    return;
  }

  if (str.length() > 7 && str[str.length() - 4] == '.' && str.substr(str.length() - 7, 3) == "_en") {
    const auto &lang = Locator<Preferences>::get().getUserPreference<std::string>(PreferenceNames::Language,
                                                                                  PreferenceDefaultValues::Language);
    str = str.substr(0, str.length() - 7).append("_").append(lang).append(str.substr(str.length() - 4, 4));
  }
}

bool getLine(GGPackBufferStream &input, std::string &line) {
  char c;
  line.clear();
  do {
    input.read(&c, 1);
    if (c == 0 || c == '\n') {
      return input.tell() < input.getLength();
    }
    line.append(&c, 1);
  } while (true);
}

std::wstring towstring(const std::string &text) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
  return converter.from_bytes(text.data(), text.data() + text.size());
}

std::string tostring(const std::wstring &text) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
  return converter.to_bytes(text);
}

std::string toUtf8(const std::wstring &text) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(text);
}

bool getLine(GGPackBufferStream &input, std::wstring &wline) {
  std::string line;
  char c;
  do {
    input.read(&c, 1);
    if (c == 0 || c == '\n') {
      std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
      wline = converter.from_bytes(line.data(), line.data() + line.size());

      return input.tell() < input.getLength();
    }
    line.append(&c, 1);
  } while (true);
}

float distanceSquared(const glm::vec2 &vector1, const glm::vec2 &vector2) {
  float dx = vector1.x - vector2.x;
  float dy = vector1.y - vector2.y;

  return dx * dx + dy * dy;
}

float distance(const glm::vec2 &v1, const glm::vec2 &v2) {
  return std::sqrt(distanceSquared(v1, v2));
}

float length(const glm::vec2 &v) {
  return sqrtf(v.x * v.x + v.y * v.y);
}

const double EPS = 1E-9;

double det(float a, float b, float c, float d) {
  return a * d - b * c;
}

inline bool betw(float l, float r, float x) {
  return fmin(l, r) <= x + EPS && x <= fmax(l, r) + EPS;
}

template<typename T>
void swap(T &a, T &b) {
  T c = a;
  a = b;
  b = c;
}

inline bool intersect_1d(float a, float b, float c, float d) {
  if (a > b)
    swap(a, b);
  if (c > d)
    swap(c, d);
  return fmax(a, c) <= fmin(b, d) + EPS;
}

bool less(const glm::vec2 &p1, const glm::vec2 &p2) {
  return p1.x < p2.x - EPS || (fabs(p1.x - p2.x) < EPS && p1.y < p2.y - EPS);
}

Facing toFacing(std::optional<UseDirection> direction) {
  auto dir = direction.value_or(UseDirection::Front);
  switch (dir) {
  case UseDirection::Front:return Facing::FACE_FRONT;
  case UseDirection::Back:return Facing::FACE_BACK;
  case UseDirection::Left:return Facing::FACE_LEFT;
  case UseDirection::Right:return Facing::FACE_RIGHT;
  }
}

Facing getOppositeFacing(Facing facing) {
  switch (facing) {
  case Facing::FACE_FRONT:return Facing::FACE_BACK;
  case Facing::FACE_BACK:return Facing::FACE_FRONT;
  case Facing::FACE_LEFT:return Facing::FACE_RIGHT;
  case Facing::FACE_RIGHT:return Facing::FACE_LEFT;
  }
}

ngf::irect toRect(const ngf::GGPackValue &json) {
  auto x = json["x"].getInt();
  auto y = json["y"].getInt();
  auto w = json["w"].getInt();
  auto h = json["h"].getInt();
  return ngf::irect::fromPositionSize({x, y}, {w, h});
}

glm::ivec2 toSize(const ngf::GGPackValue &json) {
  glm::ivec2 v;
  v.x = json["w"].getInt();
  v.y = json["h"].getInt();
  return v;
}

UseDirection toDirection(const std::string &text) {
  if (strcmp(text.c_str(), "DIR_FRONT") == 0) {
    return UseDirection::Front;
  }
  if (strcmp(text.c_str(), "DIR_LEFT") == 0) {
    return UseDirection::Left;
  }
  if (strcmp(text.c_str(), "DIR_BACK") == 0) {
    return UseDirection::Back;
  }
  if (strcmp(text.c_str(), "DIR_RIGHT") == 0) {
    return UseDirection::Right;
  }
  return UseDirection::Front;
}

glm::vec2 parsePos(const std::string &text) {
  auto commaPos = text.find_first_of(',');
  auto x = std::strtof(text.substr(1, commaPos - 1).c_str(), nullptr);
  auto y = std::strtof(text.substr(commaPos + 1, text.length() - 1).c_str(), nullptr);
  return glm::vec2(x, y);
}

ngf::irect parseRect(const std::string &text) {
  auto re = std::regex(R"(\{\{(\-?\d+),(\-?\d+)\},\{(\-?\d+),(\-?\d+)\}\})");
  std::smatch matches;
  std::regex_search(text, matches, re);
  auto left = std::strtol(matches[1].str().c_str(), nullptr, 10);
  auto top = std::strtol(matches[2].str().c_str(), nullptr, 10);
  auto right = std::strtol(matches[3].str().c_str(), nullptr, 10);
  auto bottom = std::strtol(matches[4].str().c_str(), nullptr, 10);
  return ngf::irect::fromPositionSize({left, top}, {right - left, bottom - top});
}

void parsePolygon(const std::string &text, std::vector<glm::ivec2> &vertices) {
  int i = 1;
  int endPos;
  do {
    auto commaPos = text.find_first_of(',', i);
    auto x = std::strtol(text.substr(i, commaPos - i).c_str(), nullptr, 10);
    endPos = text.find_first_of('}', commaPos + 1);
    auto y = std::strtol(text.substr(commaPos + 1, endPos - commaPos - 1).c_str(), nullptr, 10);
    i = endPos + 3;
    vertices.emplace_back(x, y);
  } while (static_cast<int>(text.length() - 1) != endPos);
}

ngf::Color parseColor(const std::string &color) {
  auto c = std::strtol(color.c_str(), nullptr, 16);
  return fromRgb(c);
}

ngf::Color fromRgba(SQInteger color) {
  auto col = static_cast<uint32_t>(color);
  ngf::Color c(
      static_cast<int>((col >> 16) & 255),
      static_cast<int>((col >> 8) & 255),
      static_cast<int>(col & 255),
      static_cast<int>((col >> 24) & 255));
  return c;
}

int toInteger(const ngf::Color &c) {
  auto r = static_cast<uint32_t>(c.r * 255u);
  auto g = static_cast<uint32_t>(c.g * 255u);
  auto b = static_cast<uint32_t>(c.b * 255u);
  auto a = static_cast<uint32_t>(c.a * 255u);
  return static_cast<int>((r << 16) | (g << 8) | b | (a << 24));
}

ngf::Color fromRgb(SQInteger color) {
  auto col = static_cast<int>(color);
  ngf::Color c((col >> 16) & 255, (col >> 8) & 255, col & 255);
  return c;
}

glm::vec2 toDefaultView(glm::ivec2 pos, glm::ivec2 fromSize) {
  return glm::vec2((Screen::Width * pos.x) / fromSize.x, (Screen::Height * pos.y) / fromSize.y);
}

InterpolationMethod toInterpolationMethod(SQInteger interpolation) {
  auto method = static_cast<InterpolationMethod>((interpolation & 7) + 1);
  if (interpolation & 0x100) {
    method |= InterpolationMethod::Looping;
  }
  if (interpolation & 0x200) {
    method |= InterpolationMethod::Swing;
  }
  return method;
}

ngf::frect getGlobalBounds(const ngf::Text &text) {
  return ngf::transform(text.getTransform().getTransform(), text.getLocalBounds());
}

ngf::frect getGlobalBounds(const ngf::Sprite &sprite) {
  return ngf::transform(sprite.getTransform().getTransform(), sprite.getLocalBounds());
}

ngf::GGPackValue toGGPackValue(HSQOBJECT obj) {
  return toGGPackValue((SQObject) obj, false);
}

} // namespace ng

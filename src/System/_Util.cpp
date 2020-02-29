#include "_Util.hpp"
#include "../Math/Segment.hpp"

namespace ng
{
std::string str_toupper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return ::toupper(c); } // correct
    );
    return s;
}

SQInteger int_rand(SQInteger min, SQInteger max)
{
    max++;
    auto value = rand() % (max - min) + min;
    return value;
}

float float_rand(float min, float max)
{
    float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
    return min + scale * (max - min);       /* [min, max] */
}

void replaceAll(std::string &text, const std::string &search, const std::string &replace)
{
    auto pos = text.find(search);
    while (pos != std::string::npos)
    {
        text.replace(pos, search.size(), replace);
        pos = text.find(search, pos + replace.size());
    }
}

void replaceAll(std::wstring &text, const std::wstring &search, const std::wstring &replace)
{
    auto pos = text.find(search);
    while (pos != std::wstring::npos)
    {
        text.replace(pos, search.size(), replace);
        pos = text.find(search, pos + replace.size());
    }
}

void removeFirstParenthesis(std::wstring &text)
{
    if (text.size() < 2)
        return;
    if (text.find(L'(') != 0)
        return;
    auto pos = text.find(L')');
    if (pos == std::wstring::npos)
        return;
    text = text.substr(pos + 1);
}

bool startsWith(const std::string &str, const std::string &prefix)
{
    return str.length() >= prefix.length() && 0 == str.compare(0, prefix.length(), prefix);
}

bool getLine(GGPackBufferStream &input, std::string &line)
{
    char c;
    line.clear();
    do
    {
        input.read(&c, 1);
        if (c == 0 || c == '\n')
        {
            return input.tell() < input.getLength();
        }
        line.append(&c, 1);
    } while (true);
    return false;
}

std::wstring towstring(const std::string &text)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(text.data(), text.data() + text.size());
}

std::string tostring(const std::wstring &text)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(text);
}

std::string toUtf8(const std::wstring& text)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(text);
}

bool getLine(GGPackBufferStream &input, std::wstring &wline)
{
    std::string line;
    char c;
    do
    {
        input.read(&c, 1);
        if (c == 0 || c == '\n')
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            wline = converter.from_bytes(line.data(), line.data() + line.size());

            return input.tell() < input.getLength();
        }
        line.append(&c, 1);
    } while (true);

    return false;
}

float distanceSquared(const sf::Vector2f &vector1, const sf::Vector2f &vector2)
{
    float dx = vector1.x - vector2.x;
    float dy = vector1.y - vector2.y;

    return dx * dx + dy * dy;
}

float distance(const sf::Vector2f &v1, const sf::Vector2f &v2)
{
    return std::sqrt(distanceSquared(v1, v2));
}

float length(const sf::Vector2f &v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

const double EPS = 1E-9;

double det(float a, float b, float c, float d)
{
    return a * d - b * c;
}

inline bool betw(float l, float r, float x)
{
    return fmin(l, r) <= x + EPS && x <= fmax(l, r) + EPS;
}

template <typename T>
void swap(T& a, T& b)
{
    T c = a;
    a=b;
    b=c;
}

inline bool intersect_1d(float a, float b, float c, float d)
{
    if (a > b)
        swap(a, b);
    if (c > d)
        swap(c, d);
    return fmax(a, c) <= fmin(b, d) + EPS;
}

bool less(const sf::Vector2f& p1, const sf::Vector2f& p2)
{
    return p1.x < p2.x - EPS || (fabs(p1.x - p2.x) < EPS && p1.y < p2.y - EPS);
}

bool intersect(sf::Vector2f a, sf::Vector2f b, sf::Vector2f c, sf::Vector2f d)
{
    if (!intersect_1d(a.x, b.x, c.x, d.x) || !intersect_1d(a.y, b.y, c.y, d.y))
        return false;
    Segment m(a, b);
    Segment n(c, d);
    double zn = det(m.a, m.b, n.a, n.b);
    if (abs(zn) < EPS) {
        if (abs(m.dist(c)) > EPS || abs(n.dist(a)) > EPS)
            return false;
        if (less(b, a))
            swap(a, b);
        if (less(d, c))
            swap(c, d);
        return true;
    } else {
        auto lx = -det(m.c, m.b, n.c, n.b) / zn;
        auto ly = -det(m.a, m.c, n.a, n.c) / zn;
        return betw(a.x, b.x, lx) && betw(a.y, b.y, ly) &&
            betw(c.x, d.x, lx) && betw(c.y, d.y, ly);
    }
}

bool lineSegmentsCross(const sf::Vector2f &a, const sf::Vector2f &b, const sf::Vector2f &c, const sf::Vector2f &d)
{
    return intersect(a,b,c,d);
}

Facing _toFacing(UseDirection direction)
{
    switch (direction)
    {
    case UseDirection::Front:
        return Facing::FACE_FRONT;
    case UseDirection::Back:
        return Facing::FACE_BACK;
    case UseDirection::Left:
        return Facing::FACE_LEFT;
    case UseDirection::Right:
        return Facing::FACE_RIGHT;
    }
    throw std::logic_error("Invalid direction");
}

Facing getOppositeFacing(Facing facing)
{
    switch (facing)
    {
        case Facing::FACE_FRONT:
            return Facing::FACE_BACK;
        case Facing::FACE_BACK:
            return Facing::FACE_FRONT;
        case Facing::FACE_LEFT:
            return Facing::FACE_RIGHT;
        case Facing::FACE_RIGHT:
            return Facing::FACE_LEFT;
    }
    return Facing::FACE_BACK;
}

sf::IntRect _toRect(const ng::GGPackValue &json)
{
    sf::IntRect rect;
    rect.left = json["x"].getInt();
    rect.top = json["y"].getInt();
    rect.width = json["w"].getInt();
    rect.height = json["h"].getInt();
    return rect;
}

sf::Vector2i _toSize(const ng::GGPackValue &json)
{
    sf::Vector2i v;
    v.x = json["w"].getInt();
    v.y = json["h"].getInt();
    return v;
}

UseDirection _toDirection(const std::string &text)
{
    if (strcmp(text.c_str(), "DIR_FRONT") == 0)
    {
        return UseDirection::Front;
    }
    if (strcmp(text.c_str(), "DIR_LEFT") == 0)
    {
        return UseDirection::Left;
    }
    if (strcmp(text.c_str(), "DIR_BACK") == 0)
    {
        return UseDirection::Back;
    }
    if (strcmp(text.c_str(), "DIR_RIGHT") == 0)
    {
        return UseDirection::Right;
    }
    return UseDirection::Front;
}

sf::Vector2f _parsePos(const std::string &text)
{
    auto commaPos = text.find_first_of(',');
    auto x = std::strtof(text.substr(1, commaPos - 1).c_str(),nullptr);
    auto y = std::strtof(text.substr(commaPos + 1, text.length() - 1).c_str(),nullptr);
    return sf::Vector2f(x, y);
}

sf::IntRect _parseRect(const std::string &text)
{
    auto re = std::regex(R"(\{\{(\-?\d+),(\-?\d+)\},\{(\-?\d+),(\-?\d+)\}\})");
    std::smatch matches;
    std::regex_search(text, matches, re);
    auto left = std::strtol(matches[1].str().c_str(),nullptr,10);
    auto top = std::strtol(matches[2].str().c_str(),nullptr,10);
    auto right = std::strtol(matches[3].str().c_str(),nullptr,10);
    auto bottom = std::strtol(matches[4].str().c_str(),nullptr,10);
    return sf::IntRect(left, top, right - left, bottom - top);
}

void _parsePolygon(const std::string &text, std::vector<sf::Vector2i> &vertices, int roomHeight)
{
    int i = 1;
    int endPos;
    do
    {
        auto commaPos = text.find_first_of(',', i);
        auto x = std::strtol(text.substr(i, commaPos - i).c_str(), nullptr,10);
        endPos = text.find_first_of('}', commaPos + 1);
        auto y = std::strtol(text.substr(commaPos + 1, endPos - commaPos - 1).c_str(), nullptr,10);
        i = endPos + 3;
        vertices.emplace_back(x, roomHeight - y);
    } while (text.length() - 1 != endPos);
}

sf::Color _toColor(const std::string& color)
{
    auto c = std::strtol(color.c_str(),nullptr,16);
    return _fromRgb(c);
}

sf::Color _toColor(SQInteger color)
{
    auto col = static_cast<SQUnsignedInteger>(color);
    sf::Color c((col >> 16u) & 255u, (col >> 8u) & 255u, col & 255u, (col >> 24u) & 255u);
    return c;
}

sf::Color _fromRgb(SQInteger color)
{
    auto col = static_cast<SQUnsignedInteger>(color);
    sf::Color c((col >> 16u) & 255u, (col >> 8u) & 255u, col & 255u);
    return c;
}

sf::Vector2f toDefaultView(sf::Vector2i pos, sf::Vector2i fromSize)
{
    return sf::Vector2f((Screen::Width * pos.x)/fromSize.x,(Screen::Height * pos.y)/fromSize.y);
}

} // namespace ng

#include "GGPack.h"

namespace ng
{
static const unsigned char _magicBytes[] = {
    0x4f, 0xd0, 0xa0, 0xac, 0x4a, 0x5b, 0xb9, 0xe5, 0x93, 0x79, 0x45, 0xa5, 0xc1, 0xcb, 0x31, 0x93
};

GGPackValue GGPackValue::nullValue;

GGPackValue::GGPackValue() { type = 1; }
GGPackValue::GGPackValue(const GGPackValue &value)
    : type(value.type)
{
    switch (type)
    {
    case 1:
        break;
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
    case 5:
        int_value = value.int_value;
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

GGPackValue &GGPackValue::operator[](std::size_t index)
{
    if (type == 3)
        return array_value[index];
    throw std::logic_error("This is not an array");
}

GGPackValue &GGPackValue::operator[](const std::string &key)
{
    if (type == 2)
    {
        if (hash_value.find(key) == hash_value.end())
            return nullValue;
        return hash_value[key];
    }
    throw std::logic_error("This is not an hashtable");
}

GGPackValue &GGPackValue::operator=(const GGPackValue &other)
{
    if (this != &other)
    { // protect against invalid self-assignment
        type = other.type;
        switch (type)
        {
        case 1:
            break;
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
        case 5:
            int_value = other.int_value;
            break;
        case 6:
        {
            // double
            double_value = other.double_value;
            break;
        }
        }
    }
    // by convention, always return *this
    return *this;
}
GGPackValue::~GGPackValue() {}

GGPack::GGPack()
{
}

void GGPack::open(const std::string &path)
{
    _input.open(path);
    readPack();
}

void GGPack::readHashEntry(const std::string &name, GGPackValue &value)
{
    std::vector<char> data;
    readEntry(name, data);
    readHash(data, value);
}

void GGPack::readHash(std::vector<char> &buffer, GGPackValue &value)
{
    int sig;
    _bufferStream.setBuffer(buffer);
    _bufferStream.read((char *)&sig, 4);

    if (sig != 0x04030201)
        throw std::logic_error("GGPack directory signature incorrect");

    getOffsets();

    // read hash
    value.type = 2;
    _bufferStream.seek(12);
    readHash(value);
}

void GGPack::readPack()
{
    if (!_input.is_open())
        return;

    int dataOffset, dataSize;
    _input.read((char *)&dataOffset, 4);
    _input.read((char *)&dataSize, 4);

    std::vector<char> buf(dataSize);
    _input.seekg(dataOffset, std::ios::beg);
    _input.read(&buf[0], dataSize);

    // try to detect correct method to decode data
    int sig;
    for (_method = 0; _method < 3; _method++)
    {
        _input.seekg(dataOffset, std::ios::beg);
        _input.read(&buf[0], dataSize);
        decodeUnbreakableXor(&buf[0], dataSize);
        sig = *(int *)buf.data();
        if (sig == 0x04030201)
            break;
    }

    _bufferStream.setBuffer(buf);

    // read hash
    _entries.clear();
    GGPackValue entries;
    readHash(buf, entries);

    auto len = entries["files"].array_value.size();
    for (size_t i = 0; i < len; i++)
    {
        auto filename = entries["files"][i]["filename"].string_value;
        GGPackEntry entry;
        entry.offset = entries["files"][i]["offset"].int_value;
        entry.size = entries["files"][i]["size"].int_value;
        _entries.insert(std::pair<std::string, GGPackEntry>(filename, entry));
    }
}

bool GGPack::hasEntry(const std::string &name)
{
    return _entries.find(name) != _entries.end();
}

void GGPack::readEntry(const std::string &name, std::vector<char> &data)
{
    auto entry = _entries[name];
    data.resize(entry.size);
    _input.seekg(entry.offset, std::ios::beg);

    _input.read(data.data(), entry.size);
    decodeUnbreakableXor(data.data(), entry.size);
    data[entry.size - 1] = 0;
}

void GGPack::readString(int offset, std::string &key)
{
    auto pos = _bufferStream.tell();
    offset = _offsets[offset];
    auto off = offset;
    _bufferStream.seek(off);
    std::string s;
    char c;
    do
    {
        _bufferStream.read(&c, 1);
        if (c == 0)
            break;
        key.append(&c, 1);
    } while (true);
    _bufferStream.seek(pos);
}

void GGPack::readHash(GGPackValue &value)
{
    char c;
    _bufferStream.read(&c, 1);
    if (c != 2)
    {
        throw std::logic_error("trying to parse a non-hash");
    }
    int n_pairs;
    _bufferStream.read((char *)&n_pairs, 4);
    if (n_pairs == 0)
    {
        throw std::logic_error("empty hash");
    }
    for (auto i = 0; i < n_pairs; i++)
    {
        int key_plo_idx;
        _bufferStream.read((char *)&key_plo_idx, 4);

        std::string hash_key;
        readString(key_plo_idx, hash_key);
        GGPackValue hash_value;
        readValue(hash_value);
        value.hash_value[hash_key] = hash_value;
    }

    _bufferStream.read(&c, 1);
    if (c != 2)
        throw std::logic_error("unterminated hash");
}

void GGPack::readValue(GGPackValue &value)
{
    _bufferStream.read((char *)&value.type, 1);
    switch (value.type)
    {
    case 1:
        // null
        return;
    case 2:
        // hash
        _bufferStream.seek(_bufferStream.tell() - 1);
        readHash(value);
        return;
    case 3:
        // array
        {
            int length;
            _bufferStream.read((char *)&length, 4);
            for (int i = 0; i < length; i++)
            {
                GGPackValue item;
                readValue(item);
                value.array_value.push_back(item);
            }
            char c;
            _bufferStream.read(&c, 1);
            if (c != 3)
                throw std::logic_error("unterminated array");
            return;
        }
    case 4:
        // string
        {
            int plo_idx_int;
            _bufferStream.read((char *)&plo_idx_int, 4);
            readString(plo_idx_int, value.string_value);
            return;
        }
    case 5:
    case 6:
    {
        // int
        // double
        int plo_idx_int;
        _bufferStream.read((char *)&plo_idx_int, 4);
        std::string num_str;
        readString(plo_idx_int, num_str);
        if (value.type == 5)
        {
            value.int_value = std::atoi(num_str.data());
            return;
        }
        value.double_value = std::atof(num_str.data());
        return;
    }
    default:
    {
        std::stringstream s;
        s << "Not Implemented: value type " << value.type;
        throw std::logic_error(s.str());
    }
    }
}

char *GGPack::decodeUnbreakableXor(char *buffer, int length)
{
    int code = _method != 2 ? 0x6d : 0xad;
    char previous = length & 0xff;
    for (auto i = 0; i < length; i++)
    {
        auto x = (char)(buffer[i] ^ _magicBytes[i & 0xf] ^ (i * code));
        buffer[i] = (char)(x ^ previous);
        previous = x;
    }
    if (_method != 0)
    {
        //Loop through in blocks of 16 and xor the 6th and 7th bytes
        int i = 5;
        while (i + 1 < length)
        {
            buffer[i] = (char)(buffer[i] ^ 0x0d);
            buffer[i + 1] = (char)(buffer[i + 1] ^ 0x0d);
            i += 16;
        }
    }
    return buffer;
}

void GGPack::getOffsets()
{
    _bufferStream.seek(8);
    // read ptr list offset & point to first file name offset
    int plo;
    _bufferStream.read((char *)&plo, 4);
    if (plo < 12 || plo >= _bufferStream.getLength() - 4)
        throw std::logic_error("GGPack plo out of range");

    char c;
    _bufferStream.seek(plo);
    _bufferStream.read(&c, 1);
    if (c != 7)
    {
        throw std::logic_error("GGPack cannot find plo");
    }

    _offsets.clear();
    do
    {
        int offset;
        _bufferStream.read((char *)&offset, 4);
        if (offset == 0xFFFFFFFF)
            return;
        _offsets.push_back(offset);
    } while (true);
}
}; // namespace ng

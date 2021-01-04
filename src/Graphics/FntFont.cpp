#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <engge/Graphics/FntFont.h>
#include <engge/System/Locator.hpp>
#include <engge/Graphics/ResourceManager.hpp>

namespace ng {
constexpr char PlaceholderCharacter = '?';

void CharSet::addKerning(Kerning k) {
  m_kernings.push_back(k);
}

short CharSet::getKerning(int first, int second) const {
  for (auto &k : m_kernings) {
    if (k.first == first && k.second != second)
      return k.amount;
  }
  return 0;
}

void CharSet::addChar(int id, Glyph &cd) {
  m_chars[id] = cd;
}

const Glyph &CharSet::getChar(int id) const {
  // Find the character
  auto it = m_chars.find(id);
  if (it != m_chars.end())
    return it->second;
  // If not found, find a placeholder
  it = m_chars.find(PlaceholderCharacter);
  return it->second;
}

FntFont::~FntFont() = default;

void FntFont::load(const std::filesystem::path &path, std::istream &input) {
  // Parse .fnt file
  // trace("FntFont: parsing \"{}\"...", path);

  if (!parse(path, input)) {
    std::ostringstream s;
    s << "Cannot parse file: " << path;
    throw std::runtime_error(s.str());
  }

  // Load resources
  // trace("FntFont: loading textures...");
  m_textures.resize(m_chars.pages.size());

  for (size_t i = 0; i < m_chars.pages.size(); i++) {
    std::filesystem::path texPath = m_chars.pages[i];
    texPath = texPath.replace_extension("");
    m_textures[i] = *Locator<ResourceManager>::get().getTexture(texPath);
  }
}

void FntFont::loadFromFile(const std::filesystem::path &path) {
  // Parse .fnt file
  // trace("FntFont: parsing \"{}\"...", path);
  std::ifstream is(path, std::ios::binary);
  if (!is.is_open()) {
    std::ostringstream s;
    s << "Failed to open font file \"" << path << "\"";
    throw std::runtime_error(s.str());
  }
  load(path, is);
  is.close();
}

float FntFont::getKerning(
    unsigned int first, unsigned int second, unsigned int) const {
  return m_chars.getKerning(first, second);
}

const ngf::Texture &FntFont::getTexture(unsigned int) const {
  return m_textures[0];
}

bool FntFont::parse(const std::filesystem::path &path, std::istream &input) {
  // Note : the '>>' operator is formatting, so we use short typed values
  // to be sure that they will be read as integers

  std::string line;
  while (std::getline(input, line)) {
    unsigned int i;
    std::stringstream lineStream;
    std::string tag, pair, key, value;

    lineStream << line;
    lineStream >> tag;

    // trace(lineStream.str());

    if (tag == "info") {
      // Not implemented yet
      lineStream.str("");
    } else if (tag == "common") {
      while (!lineStream.eof()) {
        lineStream >> pair;
        i = pair.find('=');
        key = pair.substr(0, i);
        value = pair.substr(i + 1);
        std::stringstream converter;
        converter << value;

        if (key == "lineHeight")
          converter >> m_chars.lineHeight;
        else if (key == "base")
          converter >> m_chars.base;
        else if (key == "scaleW")
          converter >> m_chars.scaleW;
        else if (key == "scaleH")
          converter >> m_chars.scaleH;
        else if (key == "packed")
          converter >> m_chars.packed;
        else if (key == "alphaChnl")
          converter >> m_chars.alphaChnl;
        else if (key == "redChnl")
          converter >> m_chars.redChnl;
        else if (key == "greenChnl")
          converter >> m_chars.greenChnl;
        else if (key == "blueChnl")
          converter >> m_chars.blueChnl;
        /*else if(key == "pages") // pages are automatically counted
                    converter >> ?*/
      }
    } else if (tag == "page") {
      unsigned short id = 0;
      while (!lineStream.eof()) {
        lineStream >> pair;
        i = pair.find('=');
        key = pair.substr(0, i);
        value = pair.substr(i + 1);
        std::stringstream converter;
        converter << value;

        if (key == "id") {
          converter >> id;
          if (id >= m_chars.pages.size())
            m_chars.pages.resize(id + 1);
        } else if (key == "file") {
          // Remove quotes
          // m_chars.pages[id] = filename;
          // this should be the code below but due to a bug in
          // UIFontSmallBold.fnt (it points to UIFontSmall.png instead
          // of UIFontSmallBold.png) I replaced it to:
          auto pagePath = path;
          m_chars.pages[id] = pagePath.replace_extension(".png").u8string();
        }
      }
    } else if (tag == "char") {
      // Note : char count is ignored because not needed
      Glyph glyph;
      int id, x, y, width, height, xoffset, yoffset, page, chnl;
      while (!lineStream.eof()) {
        lineStream >> pair;
        i = pair.find('=');
        key = pair.substr(0, i);
        value = pair.substr(i + 1);
        std::stringstream converter;
        converter << value;

        if (key == "id")
          converter >> id;
        else if (key == "x")
          converter >> x;
        else if (key == "y")
          converter >> y;
        else if (key == "width")
          converter >> width;
        else if (key == "height")
          converter >> height;
        else if (key == "xoffset")
          converter >> xoffset;
        else if (key == "yoffset")
          converter >> yoffset;
        else if (key == "xadvance")
          converter >> glyph.advance;
        else if (key == "page")
          converter >> page;
        else if (key == "chnl")
          converter >> chnl;
      }

      glyph.textureRect = ngf::irect::fromPositionSize({x, y}, {width, height});
      glyph.bounds = ngf::irect::fromPositionSize({xoffset, yoffset}, {width, height});
      m_chars.addChar(id, glyph);
    } else if (tag == "kerning") {
      Kerning k;
      // Note : Kerning count is ignored because not needed
      while (!lineStream.eof()) {
        lineStream >> pair;
        i = pair.find('=');
        key = pair.substr(0, i);
        value = pair.substr(i + 1);
        std::stringstream converter;
        converter << value;

        if (key == "first")
          converter >> k.first;
        else if (key == "second")
          converter >> k.second;
        else if (key == "amount")
          converter >> k.amount;
      }

      m_chars.addKerning(k);
    }
  }

  return true;
}

const Glyph &FntFont::getGlyph(unsigned int codePoint) const {
  return m_chars.getChar((int) codePoint);
}
}

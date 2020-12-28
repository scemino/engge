#pragma once
#include <ngf/Graphics/Text.h>
#include <ngf/Graphics/Rect.h>
#include <ngf/Math/Transform.h>
#include <ngf/Graphics/Sprite.h>

namespace ng {
ngf::frect getGlobalBounds(const ngf::Text &text);
ngf::frect getGlobalBounds(const ngf::Sprite &sprite);
}

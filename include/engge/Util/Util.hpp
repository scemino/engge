#pragma once
#include <engge/Graphics/Text.hpp>
#include <ngf/Graphics/Rect.h>
#include <ngf/Math/Transform.h>
#include <ngf/Graphics/Sprite.h>
#include <ngf/IO/GGPackValue.h>
#include <squirrel.h>

namespace ng {
ngf::frect getGlobalBounds(const ng::Text &text);
ngf::frect getGlobalBounds(const ngf::Sprite &sprite);

ngf::GGPackValue toGGPackValue(HSQOBJECT obj);
}

#pragma once
#include "SFML/Graphics.hpp"

namespace ng
{
class OptionsDialog : public sf::Drawable
{
public:
    OptionsDialog();
    ~OptionsDialog();

    void setEngine(Engine* pEngine);
    void update(const sf::Time& elapsed);

private:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    struct Impl;
    std::unique_ptr<Impl> _pImpl;
};
}

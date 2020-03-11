#pragma once
#include "SFML/Graphics.hpp"

namespace ng
{
class QuitDialog : public sf::Drawable
{
public:
    typedef std::function<void(bool)> Callback;

public:
    QuitDialog();
    ~QuitDialog();

    void setCallback(Callback callback);
    void setEngine(Engine* pEngine);
    void update(const sf::Time& elapsed);
    void updateLanguage();

private:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    struct Impl;
    std::unique_ptr<Impl> _pImpl;
};
}

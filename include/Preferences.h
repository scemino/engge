#pragma once
#include <string>
#include <map>
#include <any>

namespace ng
{
class Preferences
{
  public:
    void setUserPreference(const std::string &name, std::any value)
    {
        _values[name] = value;
    }

    void removeUserPreference(const std::string &name)
    {
        auto it = _values.find(name);
        if (it != _values.end())
        {
            _values.erase(it);
        }
    }

    std::any getUserPreference(const std::string &name, std::any value) const
    {
        auto it = _values.find(name);
        if (it == _values.end())
        {
            return value;
        }
        return it->second;
    }

  private:
    std::map<std::string, std::any> _values;
};
} // namespace ng

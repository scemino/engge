#pragma once
#include <cmath>
#include <functional>

namespace ng
{
enum class InterpolationMethod
{
    None        = 0,
    Linear      = 1,
    EaseIn      = 2,
    EaseInOut   = 3,
    EaseOut     = 4,
    SlowEaseIn  = 5,
    SlowEaseOut = 6,
    Looping     = 0x10,
    Swing       = 0x20,
};

template<class T> inline T operator~ (T a) { return (T)~(int)a; }
template<class T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }
template<class T> inline T operator& (T a, T b) { return (T)((int)a & (int)b); }
template<class T> inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
template<class T> inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }

class Interpolations
{
  public:
    static float linear(float t) { return t; }

    static float easeIn(float t) { return t * t * t * t; }

    static float easeOut(float t)
    {
        float f = (t - 1);
        return f * f * f * (1 - t) + 1;
    }

    static float easeInOut(float t)
    {
        if (t < 0.5)
        {
            return 8 * t * t * t * t;
        }

        float f = (t - 1);
        return -8 * f * f * f * f + 1;
    }
};

class InterpolationHelper
{
    public:
        static std::function<float(float)> getInterpolationMethod(InterpolationMethod index)
        {
            switch (index)
            {
                case InterpolationMethod::SlowEaseIn:
                case InterpolationMethod::EaseIn:
                    return Interpolations::easeIn;
                case InterpolationMethod::EaseInOut:
                    return Interpolations::easeInOut;
                case InterpolationMethod::SlowEaseOut:
                case InterpolationMethod::EaseOut:
                    return Interpolations::easeOut;
                default:
                    return Interpolations::linear;
            }
        }
};
} // namespace ng

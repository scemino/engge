#pragma once

namespace ng
{
enum class InterpolationMethod
{
    Linear,
    EaseIn,
    EaseInOut,
    EaseOut,
};

class Interpolations
{
  public:
    static float linear(float t)
    {
        return t;
    }

    static float easeIn(float t)
    {
        return t * t * t * t;
    }

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
} // namespace ng

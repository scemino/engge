#include "Funnel.h"

namespace ng {
inline float triarea2(const float* a, const float* b, const float* c)
{
    const float ax = b[0] - a[0];
    const float ay = b[1] - a[1];
    const float bx = c[0] - a[0];
    const float by = c[1] - a[1];
    return bx*ay - ax*by;
}

float vdistsqr(const float* a, const float* b)
{
    const float x = b[0] - a[0];
    const float y = b[1] - a[1];
    return x*x* +y*y;
}

inline void vcpy(float* target, const float* source)
{
    target[0] = source[0];
    target[1] = source[1];
}

inline bool vequal(const float* a, const float* b)
{
    static const float eq = 0.001f*0.001f;
    return vdistsqr(a, b) < eq;
}

int stringPull(const float* portals, int nportals,
               float* pts, const int maxPts)
{
    // Find straight path.
    int npts = 0;
    // Init scan state
    float portalApex[2], portalLeft[2], portalRight[2];
    int apexIndex = 0, leftIndex = 0, rightIndex = 0;
    vcpy(portalApex, &portals[0]);
    vcpy(portalLeft, &portals[0]);
    vcpy(portalRight, &portals[2]);

    // Add start point.
    vcpy(&pts[npts * 2], portalApex);
    npts++;

    for (int i = 1; i < nportals && npts < maxPts; ++i)
    {
        const float* left = &portals[i * 4 + 0];
        const float* right = &portals[i * 4 + 2];

        // Update right vertex.
        if (triarea2(portalApex, portalRight, right) <= 0.0f)
        {
            if (vequal(portalApex, portalRight) || triarea2(portalApex, portalLeft, right) > 0.0f)
            {
                // Tighten the funnel.
                vcpy(portalRight, right);
                rightIndex = i;
            }
            else
            {
                // Right over left, insert left to path and restart scan from portal left point.
                vcpy(&pts[npts * 2], portalLeft);
                npts++;
                // Make current left the new apex.
                vcpy(portalApex, portalLeft);
                apexIndex = leftIndex;
                // Reset portal
                vcpy(portalLeft, portalApex);
                vcpy(portalRight, portalApex);
                leftIndex = apexIndex;
                rightIndex = apexIndex;
                // Restart scan
                i = apexIndex;
                continue;
            }
        }

        // Update left vertex.
        if (triarea2(portalApex, portalLeft, left) >= 0.0f)
        {
            if (vequal(portalApex, portalLeft) || triarea2(portalApex, portalRight, left) < 0.0f)
            {
                // Tighten the funnel.
                vcpy(portalLeft, left);
                leftIndex = i;
            }
            else
            {
                // Left over right, insert right to path and restart scan from portal right point.
                vcpy(&pts[npts * 2], portalRight);
                npts++;
                // Make current right the new apex.
                vcpy(portalApex, portalRight);
                apexIndex = rightIndex;
                // Reset portal
                vcpy(portalLeft, portalApex);
                vcpy(portalRight, portalApex);
                leftIndex = apexIndex;
                rightIndex = apexIndex;
                // Restart scan
                i = apexIndex;
                continue;
            }
        }
    }
    // Append last point to path.
    if (npts < maxPts)
    {
        vcpy(&pts[npts * 2], &portals[(nportals - 1) * 4 + 0]);
        npts++;
    }

    return npts;
}


void Funnel::makeFunnel(const std::vector<sf::Vector2f>& portals, std::vector<sf::Vector2f>& path)
{
    path.resize(portals.size());
    int size = stringPull((float *)portals.data(), portals.size() / 2, (float *)path.data(), path.size());
    path.resize(size);
}
}
#pragma once

#include "Math/Quaternion.h"
#include "Math/VQS.h"

namespace Parfait
{
	namespace Math
	{
        // Linear Interpolation
		static glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t)
		{
			return (1.0f - t) * a + t * b;
		}
		static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t)
		{
			return (1.0f - t) * q1 + t * q2;
		}

        // Spherical Interpolation
		static Quaternion Slerp(Quaternion q1, Quaternion q2, float t)
		{
            float dot = Dot(q1, q2);

            // If the dot product is NEGATIVE, Invert one quaternion to take the shorter path
            if (dot < 0.0f)
            {
                q2 = -1.0f * q2;
                dot = -dot;
            }

            // Clamp dot product to be in range [0, 1] to avoid out-of-range acos
            dot = fmin(fmax(dot, 0.0f), 1.0f);

            float alpha = acos(dot);
            // If the angle is small, use linear interpolation to avoid division by zero
            if (alpha < 1e-6)
            {
                return (1.0f - t) * q1 + t * q2;
            }

            float sinAlpha = sin(alpha);
            float factor1 = sin((1.0f - t) * alpha) / sinAlpha;
            float factor2 = sin(t * alpha) / sinAlpha;

            return factor1 * q1 + factor2 * q2;
		}

        // Exponential Interpolation
        static glm::vec3 Elerp(const glm::vec3& a, const glm::vec3& b, float t) 
        {
            // Clamp t to the range [0, 1]
            t = glm::clamp(t, 0.0f, 1.0f);

            return a + (b - a) * (1 - std::exp(-t));
        }
	}
}
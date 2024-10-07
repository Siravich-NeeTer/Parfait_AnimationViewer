#pragma once

#include "Math/Quaternion.h"
#include "Math/VQS.h"

namespace Parfait
{
	namespace Math
	{
		static glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t)
		{
			return (1.0f - t) * a + t * b;
		}
		static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t)
		{
			return (1.0f - t) * q1 + t * q2;
		}

		static Quaternion Slerp(Quaternion q1, Quaternion q2, float t)
		{
			float dot = Dot(q1, q2);
			// In Case: Dot product is NEGATIVE -> Flip q2
			if (dot < 0.0f)
			{
				q2 = -1.0f * q2;
				dot = -dot;
			}
			float alpha = acos(dot);

			if (alpha <= 0.0f)
				return q1;
			else if (alpha >= 1.0f)
				return q2;

			Quaternion ans = (sin(alpha - t * alpha) / sin(alpha)) * q1 + (sin(t * alpha) / sin(alpha)) * q2;
			return ans;
		}
	}
}
#pragma once

#include "Math/Quaternion.h"

namespace Parfait
{
	namespace Math
	{
		class VQS
		{
			public:
				// v = Translation
				// q = Rotation
				// s = Scale (Uniform)
				glm::vec3 v;
				Quaternion q;
				glm::vec3 s;

				VQS() = default;
				VQS(glm::vec3 _v, Quaternion _q, glm::vec3 _s)
					: v(_v), q(_q), s(_s)
				{
				}

				glm::vec3 operator*(const glm::vec3& rhs)
				{
					glm::vec3 r_vqs = rhs;

					r_vqs = this->s * r_vqs;
					r_vqs = q * r_vqs;
					r_vqs = v + r_vqs;

					return r_vqs;
				}
				VQS operator*(const VQS& rhs)
				{
					VQS vqs;
					vqs.v = (*this) * rhs.v;
					vqs.q = this->q * rhs.q;
					vqs.s = this->s * rhs.s;

					return vqs;
				}

			private:
		};
	}
}
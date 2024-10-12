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
				
				// VQS Concatenation
				VQS operator*(const VQS& rhs)
				{
					VQS vqs;
					vqs.v = (*this) * rhs.v;
					vqs.q = this->q * rhs.q;
					vqs.s = this->s * rhs.s;

					return vqs;
				}
				// VQS with Point/Vector
				glm::vec3 operator*(const glm::vec3& rhs)
				{
					glm::vec3 r_vqs = rhs;

					r_vqs = this->s * r_vqs;
					r_vqs = q * r_vqs;
					r_vqs = v + r_vqs;

					return r_vqs;
				}

				glm::mat4 Matrix() const
				{
					glm::mat4 scale = glm::scale(glm::mat4(1.0f), s);
					glm::mat4 rotate = q.Matrix();
					glm::mat4 translate = glm::translate(glm::mat4(1.0f), v);

					return translate * rotate * scale;
				}

				// Identity
				// v = (0, 0, 0)		-> Don't Move
				// q = [1, (0, 0, 0)]	-> Identity Quaternion
				// s = (1, 1, 1)		-> Base Scaling
				static VQS Identity()
				{
					return VQS(glm::vec3(0.0f), Quaternion::Identity(), glm::vec3(1.0f));
				}

			private:
		};

		static VQS MatrixToVQS(const glm::mat4& transform)
		{
			glm::vec3 v = { transform[3][0], transform[3][1], transform[3][2] };
			glm::vec3 s = {
				sqrt(transform[0][0] * transform[0][0] + transform[1][0] * transform[1][0] + transform[2][0] * transform[2][0]),
				sqrt(transform[0][1] * transform[0][1] + transform[1][1] * transform[1][1] + transform[2][1] * transform[2][1]),
				sqrt(transform[0][2] * transform[0][2] + transform[1][2] * transform[1][2] + transform[2][2] * transform[2][2])
			};

			// Calculate new transform that scaling has been removed
			glm::mat4 removedScaleTransform = transform;
			for (int i = 0; i < 3; ++i) 
			{
				for (int j = 0; j < 3; ++j) 
				{
					removedScaleTransform[i][j] = transform[i][j] / (i == 0 ? s.x : (i == 1 ? s.y : s.z));
				}
			}

			Quaternion q = MatrixToQuaternion(removedScaleTransform);

			return VQS(v, q, s);
		}
	}
}
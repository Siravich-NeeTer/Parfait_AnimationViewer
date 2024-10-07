#pragma once

#include <glm/glm.hpp>

namespace Parfait
{
	namespace Math
	{
		class Quaternion
		{
			public:
				float s = 1.0f;
				glm::vec3 v = glm::vec3(0.0f);

				Quaternion() = default;
				Quaternion(float _s, const glm::vec3& _v)
					: s(_s), v(_v)
				{
				}
				Quaternion(float _s, float _x, float _y, float _z)
					: s(_s), v(_x, _y, _z)
				{
				}

				Quaternion operator+(const Quaternion& rhs)
				{
					Quaternion q;
					q.s = this->s + rhs.s;
					q.v = this->v + rhs.v;
					return q;
				}
				Quaternion operator-(const Quaternion& rhs)
				{
					Quaternion q;
					q.s = this->s - rhs.s;
					q.v = this->v - rhs.v;
					return q;
				}
				Quaternion operator*(const Quaternion& rhs)
				{
					Quaternion q;
					q.s = (this->s * rhs.s) - glm::dot(this->v, rhs.v);
					q.v = (this->s * rhs.v) + (rhs.s * this->v) + glm::cross(this->v, rhs.v);
					return q;
				}
				Quaternion operator*(const float& scale)
				{
					Quaternion q;
					q.s = this->s * scale;
					q.v = this->v * scale;
					return q;
				}
				glm::vec3 operator*(const glm::vec3& pos)
				{
					Quaternion qPos(0.0f, pos);

					Quaternion finalPos = (*this) * qPos * this->Inverse();
					return { finalPos.v.x, finalPos.v.y, finalPos.v.z };
				}
				Quaternion operator/(const float& rhs)
				{
					Quaternion q;
					q.s = this->s / rhs;
					q.v = this->v / rhs;
					return q;
				}

				friend std::ostream& operator<< (std::ostream& os, const Quaternion& quaternion)
				{
					os << "[" << quaternion.s << ", (" << quaternion.v.x << ", " << quaternion.v.y << ", " << quaternion.v.z << ")]";
					return os;
				}

				float Length() const 
				{
					return sqrt(s * s + v.x * v.x + v.y * v.y + v.z * v.z);
				}
				float LengthSquare() const 
				{
					return s * s + v.x * v.x + v.y * v.y + v.z * v.z;
				}
				Quaternion Inverse() const 
				{
					float lengthSq = this->LengthSquare();

					Quaternion qi;
					qi.s = this->s / lengthSq;
					qi.v = -this->v / lengthSq;
					return qi;
				}
				Quaternion Conjugate() const 
				{
					return Quaternion(this->s, -this->v);
				}
				Quaternion Normalize() const 
				{
					return Quaternion(this->s, this->v) / this->Length();
				}
				glm::mat4 Matrix() const 
				{
					glm::mat4 qMat;
					qMat[0][0] = 1.0f - 2.0f * (v.y * v.y + v.z * v.z);
					qMat[1][0] = 2.0f * (v.x * v.y - s * v.z);
					qMat[2][0] = 2.0f * (v.x * v.z + s * v.y);
					qMat[3][0] = 0.0f;

					qMat[0][1] = 2.0f * (v.x * v.y + s * v.z);
					qMat[1][1] = 1.0f - 2.0f * (v.x * v.x + v.z * v.z);
					qMat[2][1] = 2.0f * (v.y * v.z - s * v.x);
					qMat[3][1] = 0.0f;

					qMat[0][2] = 2.0f * (v.x * v.z - s * v.y);
					qMat[1][2] = 2.0f * (v.y * v.z + s * v.x);
					qMat[2][2] = 1.0f - 2.0f * (v.x * v.x + v.y * v.y);
					qMat[3][2] = 0.0f;

					qMat[0][3] = 0.0f;
					qMat[1][3] = 0.0f;
					qMat[2][3] = 0.0f;
					qMat[3][3] = 1.0f;

					return qMat;
				}

				static Quaternion Identity()
				{
					return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
				}

			private:

		};

		static Quaternion operator*(const float& lhs, const Quaternion& rhs)
		{
			Quaternion q;
			q.s = rhs.s * lhs;
			q.v = rhs.v * lhs;
			return q;
		}
		static float Dot(const Quaternion& q1, const Quaternion& q2)
		{
			return q1.s * q2.s + glm::dot(q1.v, q2.v);
		}
	}
}
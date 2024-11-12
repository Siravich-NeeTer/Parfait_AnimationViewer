#pragma once

#include <glm/glm.hpp>

#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

namespace Parfait
{
	namespace Primitive
	{
		static std::vector<glm::vec3> CreateSphere(float _radius, uint32_t _sectorCount, uint32_t _stackCount)
		{
            std::vector<glm::vec3> vertices;

            float x, y, z, xy; // Vertex position
            float sectorStep = 2 * M_PI / _sectorCount;
            float stackStep = M_PI / _stackCount;
            float sectorAngle, stackAngle;

            // Generate vertices
            for (unsigned int i = 0; i <= _stackCount; ++i) {
                stackAngle = M_PI / 2 - i * stackStep;      // from pi/2 to -pi/2
                xy = _radius * cosf(stackAngle);        // radius of the circle at this stack
                z = _radius * sinf(stackAngle);         // z coordinate

                for (unsigned int j = 0; j <= _sectorCount; ++j) {
                    sectorAngle = j * sectorStep; // from 0 to 2pi

                    x = xy * cosf(sectorAngle); // x coordinate
                    y = xy * sinf(sectorAngle); // y coordinate

                    vertices.push_back({ x, y, z });
                }
            }
            return vertices;
		}
	}
}
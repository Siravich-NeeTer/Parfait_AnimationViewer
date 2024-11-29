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

            // Generate all the vertices for the sphere
            for (unsigned int i = 0; i <= _stackCount; ++i) {
                stackAngle = M_PI / 2 - i * stackStep;      // from pi/2 to -pi/2
                xy = _radius * cosf(stackAngle);        // radius of the circle at this stack
                z = _radius * sinf(stackAngle);         // z coordinate

                for (unsigned int j = 0; j <= _sectorCount; ++j) {
                    sectorAngle = j * sectorStep; // from 0 to 2pi

                    x = xy * cosf(sectorAngle); // x coordinate
                    y = xy * sinf(sectorAngle); // y coordinate

                    vertices.push_back({ x, y, z });  // Push each vertex into the vector
                }
            }

            // Now generate the triangles using the vertices we just generated
            std::vector<glm::vec3> triangleVertices;

            for (unsigned int i = 0; i < _stackCount; ++i) {
                for (unsigned int j = 0; j < _sectorCount; ++j) {
                    // Calculate indices for current stack and sector
                    unsigned int current = i * (_sectorCount + 1) + j;
                    unsigned int next = (i + 1) * (_sectorCount + 1) + j;
                    unsigned int nextSector = (j + 1) % _sectorCount;  // Wrap around to first sector
                    unsigned int currentNext = i * (_sectorCount + 1) + nextSector;
                    unsigned int nextNext = (i + 1) * (_sectorCount + 1) + nextSector;

                    // First triangle (current, next, currentNext)
                    triangleVertices.push_back(vertices[current]);
                    triangleVertices.push_back(vertices[next]);
                    triangleVertices.push_back(vertices[currentNext]);

                    // Second triangle (currentNext, next, nextNext)
                    triangleVertices.push_back(vertices[currentNext]);
                    triangleVertices.push_back(vertices[next]);
                    triangleVertices.push_back(vertices[nextNext]);
                }
            }

            return triangleVertices;
        }
	}
}
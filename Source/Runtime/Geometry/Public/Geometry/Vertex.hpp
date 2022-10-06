#pragma once

#include <glm/glm.hpp>

namespace Ilum
{
STRUCT(Vertex, Enable)
{
	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 normal;
	alignas(16) glm::vec3 tangent;
	alignas(16) glm::vec2 texcoord;
};
}        // namespace Ilum
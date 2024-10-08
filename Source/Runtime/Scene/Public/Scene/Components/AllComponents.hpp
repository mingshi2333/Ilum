#pragma once

#include "Camera/OrthographicCamera.hpp"
#include "Camera/PerspectiveCamera.hpp"
#include "Light/DirectionalLight.hpp"
#include "Light/EnvironmentLight.hpp"
#include "Light/PointLight.hpp"
#include "Light/RectLight.hpp"
#include "Light/SpotLight.hpp"
#include "Renderable/MeshRenderer.hpp"
#include "Renderable/SkinnedMeshRenderer.hpp"
#include "Transform.hpp"

struct ImGuiContext;

namespace Ilum
{
namespace Cmpt
{
void SetImGuiContext(ImGuiContext *context);
}
}        // namespace Ilum
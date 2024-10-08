#pragma once

#include "Light.hpp"

#include <glm/glm.hpp>

namespace Ilum
{
namespace Cmpt
{
class RectLight : public Light
{
  public:
	RectLight(Node *node);

	virtual ~RectLight() = default;

	virtual bool OnImGui() override;

	virtual void Save(OutputArchive &archive) const override;

	virtual void Load(InputArchive &archive) override;

	virtual std::type_index GetType() const override;

	virtual size_t GetDataSize() const override;

	virtual void *GetData(Camera *camera = nullptr) override;

  private:
	struct
	{
		glm::vec3 color = glm::vec3(1.f);

		float intensity = 100.f;

		glm::vec4 corner[4];

		alignas(16) uint32_t texture_id = ~0U;
		uint32_t two_side               = 0;
	} m_data;
};
}        // namespace Cmpt
}        // namespace Ilum
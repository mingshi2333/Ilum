#pragma once

#include "../Resource.hpp"

#include <RHI/RHIContext.hpp>

namespace Ilum
{
class MaterialGraphDesc;
class ResourceManager;
struct MaterialData;
struct MaterialCompilationContext;

template <>
class Resource<ResourceType::Material> final : public IResource
{
  public:
	Resource(RHIContext *rhi_context, const std::string &name);

	Resource(RHIContext *rhi_context, const std::string &name, MaterialGraphDesc &&desc);

	virtual ~Resource() override;

	virtual bool Validate() const override;

	virtual void Load(RHIContext *rhi_context) override;

	void Compile(RHIContext *rhi_context, ResourceManager *manager, RHITexture *dummy_texture, const std::string &layout = "");

	void Update(RHIContext *rhi_context, ResourceManager *manager, RHITexture *dummy_texture);

	void PostUpdate(RHIContext *rhi_context, uint32_t material_id, const std::vector<RHITexture *> &scene_texture_2d, const std::vector<RHISampler *> &samplers, RHIBuffer *material_buffers, RHIBuffer *material_offsets);

	const MaterialData &GetMaterialData() const;

	const MaterialCompilationContext &GetCompilationContext() const;

	const std::string &GetLayout() const;

	MaterialGraphDesc &GetDesc();

	bool IsValid() const;

  private:
	std::vector<uint8_t> RenderPreview(RHIContext *rhi_context, uint32_t material_id, const std::vector<RHITexture *> &scene_texture_2d, const std::vector<RHISampler *> &samplers, RHIBuffer *material_buffers, RHIBuffer *material_offsets);

  private:
	struct Impl;
	Impl *m_impl = nullptr;
};
}        // namespace Ilum
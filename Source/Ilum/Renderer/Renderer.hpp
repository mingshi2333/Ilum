#pragma once

#include "Utils/PCH.hpp"

#include "Camera.hpp"

#include "Engine/Context.hpp"
#include "Engine/Subsystem.hpp"

#include "Eventing/Event.hpp"

#include "Graphics/Image/Image.hpp"
#include "Graphics/Image/Sampler.hpp"

#include "RenderGraph/RenderGraphBuilder.hpp"

#include "Loader/ResourceCache.hpp"

#include <glm/glm.hpp>

namespace Ilum
{
class Renderer : public TSubsystem<Renderer>
{
  public:
	enum class SamplerType
	{
		Compare_Depth,
		Point_Clamp,
		Point_Wrap,
		Bilinear_Clamp,
		Bilinear_Wrap,
		Trilinear_Clamp,
		Trilinear_Wrap,
		Anisptropic_Clamp,
		Anisptropic_Wrap
	};

	enum class BufferType
	{
		MainCamera,
		DirectionalLight,
		PointLight,
		SpotLight
	};

  public:
	Renderer(Context *context = nullptr);

	~Renderer();

	virtual bool onInitialize() override;

	virtual void onPreTick() override;

	virtual void onPostTick() override;

	virtual void onShutdown() override;

	const RenderGraph *getRenderGraph() const;

	RenderGraph *getRenderGraph();

	ResourceCache &getResourceCache();

	void resetBuilder();

	void rebuild();

	bool hasImGui() const;

	void setImGui(bool enable);

	const Sampler &getSampler(SamplerType type) const;

	const BufferReference getBuffer(BufferType type) const;

	const VkExtent2D &getRenderTargetExtent() const;

	void resizeRenderTarget(VkExtent2D extent);

	const ImageReference getDefaultTexture() const;

  public:
	std::function<void(RenderGraphBuilder &)> buildRenderGraph = nullptr;

  private:
	void createSamplers();

	void createBuffers();

	void updateBuffers();

  private:
	std::function<void(RenderGraphBuilder &)> defaultBuilder;

	RenderGraphBuilder m_rg_builder;

	scope<RenderGraph> m_render_graph = nullptr;

	scope<ResourceCache> m_resource_cache = nullptr;

	std::unordered_map<SamplerType, Sampler> m_samplers;
	std::unordered_map<BufferType, Buffer>   m_buffers;

	VkExtent2D m_render_target_extent;

	Image m_default_texture;

	bool m_update = false;

	bool m_imgui = true;

	uint32_t m_texture_count = 0;

  public:
	Camera Main_Camera;

	struct CameraBuffer
	{
		glm::mat4 view_projection;
		glm::vec3 position;

		// TODO: Last info
	};

  public:
	Event<> Event_RenderGraph_Rebuild;
};
}        // namespace Ilum
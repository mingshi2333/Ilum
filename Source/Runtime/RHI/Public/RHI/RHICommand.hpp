#pragma once

#include "Fwd.hpp"
#include "RHIBuffer.hpp"
#include "RHITexture.hpp"

namespace Ilum
{
enum class CommandState
{
	Available,
	Initial,
	Recording,
	Executable,
	Pending
};

struct RHIDrawIndirectCommand
{
	uint32_t vertex_count;
	uint32_t instance_count;
	uint32_t vertex_offset;
	uint32_t instance_offset;
};

struct RHIDrawIndexedIndirectCommand
{
	uint32_t index_count;
	uint32_t instance_count;
	uint32_t index_offset;
	int32_t  vertex_offset;
	uint32_t instance_offset;
};

struct RHIDispatchIndirectCommand
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct RHIDrawMeshTasksIndirectCommand
{
	uint32_t group_count_x;
	uint32_t group_count_y;
	uint32_t group_count_z;
};

class RHICommand
{
  public:
	RHICommand(RHIDevice *device, RHIQueueFamily family);

	virtual ~RHICommand() = default;

	RHIQueueFamily GetQueueFamily() const;

	const std::string GetBackend() const;

	CommandState GetState() const;

	void Init();

	static std::unique_ptr<RHICommand> Create(RHIDevice *device, RHIQueueFamily family);

	virtual void SetName(const std::string &name) = 0;

	virtual void Begin() = 0;
	virtual void End()   = 0;

	virtual void BeginMarker(const std::string &name, float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f) = 0;
	virtual void EndMarker()                                                                                      = 0;

	virtual void BeginRenderPass(RHIRenderTarget *render_target) = 0;
	virtual void EndRenderPass()                                 = 0;

	virtual void BindVertexBuffer(uint32_t binding, RHIBuffer *vertex_buffer)    = 0;
	virtual void BindIndexBuffer(RHIBuffer *index_buffer, bool is_short = false) = 0;

	virtual void BindDescriptor(RHIDescriptor *descriptor)           = 0;
	virtual void BindPipelineState(RHIPipelineState *pipeline_state) = 0;

	virtual void SetViewport(float width, float height, float x = 0.f, float y = 0.f)                    = 0;
	virtual void SetScissor(uint32_t width, uint32_t height, int32_t offset_x = 0, int32_t offset_y = 0) = 0;

	// Drawcall
	virtual void Dispatch(uint32_t thread_x, uint32_t thread_y, uint32_t thread_z, uint32_t block_x, uint32_t block_y, uint32_t block_z) = 0;
	virtual void DispatchIndirect(RHIBuffer *buffer, size_t offset)                                                                      = 0;

	virtual void Draw(uint32_t vertex_count, uint32_t instance_count = 1, uint32_t first_vertex = 0, uint32_t first_instance = 0)                                   = 0;
	virtual void DrawIndirect(RHIBuffer *buffer, size_t offset, uint32_t draw_count, uint32_t stride)                                                               = 0;
	virtual void DrawIndirectCount(RHIBuffer *buffer, size_t offset, RHIBuffer *count_buffer, size_t count_buffer_offset, uint32_t max_draw_count, uint32_t stride) = 0;

	virtual void DrawIndexed(uint32_t index_count, uint32_t instance_count = 1, uint32_t first_index = 0, uint32_t vertex_offset = 0, uint32_t first_instance = 0)         = 0;
	virtual void DrawIndexedIndirect(RHIBuffer *buffer, size_t offset, uint32_t draw_count, uint32_t stride)                                                               = 0;
	virtual void DrawIndexedIndirectCount(RHIBuffer *buffer, size_t offset, RHIBuffer *count_buffer, size_t count_buffer_offset, uint32_t max_draw_count, uint32_t stride) = 0;

	virtual void DrawMeshTask(uint32_t thread_x, uint32_t thread_y, uint32_t thread_z, uint32_t block_x, uint32_t block_y, uint32_t block_z)                                 = 0;
	virtual void DrawMeshTasksIndirect(RHIBuffer *buffer, size_t offset, uint32_t draw_count, uint32_t stride)                                                               = 0;
	virtual void DrawMeshTasksIndirectCount(RHIBuffer *buffer, size_t offset, RHIBuffer *count_buffer, size_t count_buffer_offset, uint32_t max_draw_count, uint32_t stride) = 0;

	// RayTracing
	virtual void TraceRay(uint32_t width, uint32_t height, uint32_t depth) = 0;

	// Resource Copy
	virtual void CopyBufferToTexture(RHIBuffer *src_buffer, RHITexture *dst_texture, uint32_t mip_level, uint32_t base_layer, uint32_t layer_count) = 0;
	virtual void CopyTextureToBuffer(RHITexture *src_texture, RHIBuffer *dst_buffer, uint32_t mip_level, uint32_t base_layer, uint32_t layer_count) = 0;
	virtual void CopyBufferToBuffer(RHIBuffer *src_buffer, RHIBuffer *dst_buffer, size_t size, size_t src_offset = 0, size_t dst_offset = 0)        = 0;

	virtual void GenerateMipmaps(RHITexture *texture, RHIResourceState initial_state, RHIFilter filter)                                                                                                                                                  = 0;
	virtual void BlitTexture(RHITexture *src_texture, const TextureRange &src_range, const RHIResourceState &src_state, RHITexture *dst_texture, const TextureRange &dst_range, const RHIResourceState &dst_state, RHIFilter filter = RHIFilter::Linear) = 0;

	// Resource Reset
	virtual void FillBuffer(RHIBuffer *buffer, RHIResourceState state, size_t size, size_t offset = 0, uint32_t data = 0)    = 0;
	virtual void FillTexture(RHITexture *texture, RHIResourceState state, const TextureRange &range, const glm::vec4 &color) = 0;
	virtual void FillTexture(RHITexture *texture, RHIResourceState state, const TextureRange &range, float depth) = 0;

	// Resource Barrier
	virtual void ResourceStateTransition(const std::vector<TextureStateTransition> &texture_transitions, const std::vector<BufferStateTransition> &buffer_transitions) = 0;

  protected:
	RHIDevice     *p_device = nullptr;
	RHIQueueFamily m_family;
	CommandState   m_state = CommandState::Available;
};
}        // namespace Ilum
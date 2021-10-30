#include "GeometryPass.hpp"

#include "Graphics/Model/Model.hpp"
#include "Graphics/Model/Vertex.hpp"

#include "Renderer/Renderer.hpp"

#include "Scene/Component/MeshRenderer.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Scene.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace Ilum::pass
{
GeometryPass::GeometryPass(const std::string &output) :
    m_output(output)
{
	m_uniform_buffer = Buffer(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}

void GeometryPass::setupPipeline(PipelineState &state)
{
	state.shader.load(std::string(PROJECT_SOURCE_DIR) + "Asset/Shader/GLSL/geometry.vert", VK_SHADER_STAGE_VERTEX_BIT, Shader::Type::GLSL);
	state.shader.load(std::string(PROJECT_SOURCE_DIR) + "Asset/Shader/GLSL/geometry.frag", VK_SHADER_STAGE_FRAGMENT_BIT, Shader::Type::GLSL);

	state.dynamic_state.dynamic_states = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR};

	state.vertex_input_state.attribute_descriptions = {
	    VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
	    VkVertexInputAttributeDescription{1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texcoord)},
	    VkVertexInputAttributeDescription{2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
	    VkVertexInputAttributeDescription{3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)},
	    VkVertexInputAttributeDescription{4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, bitangent)}};

	state.vertex_input_state.binding_descriptions = {
	    VkVertexInputBindingDescription{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};

	state.color_blend_attachment_states.resize(2);

	state.descriptor_bindings.bind(0, 0, "mainCamera", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	state.descriptor_bindings.bind(0, 1, "textureArray", Renderer::instance()->getSampler(Renderer::SamplerType::Trilinear_Clamp), ImageViewType::Native, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	state.declareAttachment("gbuffer - normal", VK_FORMAT_R32G32B32A32_SFLOAT, Renderer::instance()->getRenderTargetExtent().width, Renderer::instance()->getRenderTargetExtent().height);
	state.declareAttachment("gbuffer - position", VK_FORMAT_R32G32B32A32_SFLOAT, Renderer::instance()->getRenderTargetExtent().width, Renderer::instance()->getRenderTargetExtent().height);
	state.declareAttachment("geometry - depth_stencil", VK_FORMAT_D32_SFLOAT_S8_UINT, Renderer::instance()->getRenderTargetExtent().width, Renderer::instance()->getRenderTargetExtent().height);

	state.addOutputAttachment("gbuffer - normal", AttachmentState::Clear_Color);
	state.addOutputAttachment("gbuffer - position", AttachmentState::Clear_Color);
	state.addOutputAttachment("geometry - depth_stencil", VkClearDepthStencilValue{1.f, 0u});
}

void GeometryPass::resolveResources(ResolveState &resolve)
{
	resolve.resolve("textureArray", Renderer::instance()->getResourceCache().getImageReferences());
	resolve.resolve("mainCamera", m_uniform_buffer);
}

void GeometryPass::render(RenderPassState &state)
{
	// Update camera buffer
	auto *data = m_uniform_buffer.map();
	std::memcpy(data, glm::value_ptr(Renderer::instance()->Main_Camera.camera.view_projection), sizeof(float) * 16);

	auto &cmd_buffer = state.command_buffer;

	auto &extent = Renderer::instance()->getRenderTargetExtent();

	VkViewport viewport = {0, static_cast<float>(extent.height), static_cast<float>(extent.width), -static_cast<float>(extent.height), 0, 1};
	VkRect2D   scissor  = {0, 0, extent.width, extent.height};

	vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
	vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

	auto view = Scene::instance()->getRegistry().view<cmpt::MeshRenderer>();

	view.each([&](cmpt::MeshRenderer &mesh_renderer) {
		if (Renderer::instance()->getResourceCache().hasModel(mesh_renderer.model))
		{
			auto &       model      = Renderer::instance()->getResourceCache().loadModel(mesh_renderer.model);
			VkDeviceSize offsets[1] = {0};
			vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &model.get().getVertexBuffer().get().getBuffer(), offsets);
			vkCmdBindIndexBuffer(cmd_buffer, model.get().getIndexBuffer().get().getBuffer(), 0, VK_INDEX_TYPE_UINT32);

			for (auto &submesh : model.get().getSubMeshes())
			{
				vkCmdDrawIndexed(cmd_buffer, submesh.getIndexCount(), 1, submesh.getIndexOffset(), 0, 0);
			}
		}
	});
}
}        // namespace Ilum::pass
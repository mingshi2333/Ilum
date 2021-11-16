#include "LightPass.hpp"

#include "Renderer/Renderer.hpp"

#include "Scene/Component/DirectionalLight.hpp"
#include "Scene/Component/Light.hpp"
#include "Scene/Component/Tag.hpp"
#include "Scene/Scene.hpp"

#include <entt.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace Ilum::pass
{
void LightPass::setupPipeline(PipelineState &state)
{
	state.shader.load(std::string(PROJECT_SOURCE_DIR) + "Asset/Shader/GLSL/directional_light.vert", VK_SHADER_STAGE_VERTEX_BIT, Shader::Type::GLSL);
	state.shader.load(std::string(PROJECT_SOURCE_DIR) + "Asset/Shader/GLSL/directional_light.frag", VK_SHADER_STAGE_FRAGMENT_BIT, Shader::Type::GLSL);

	state.dynamic_state.dynamic_states = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR};

	state.descriptor_bindings.bind(0, 0, "gbuffer - albedo", Renderer::instance()->getSampler(Renderer::SamplerType::Trilinear_Clamp), ImageViewType::Native, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	state.descriptor_bindings.bind(0, 1, "gbuffer - normal", Renderer::instance()->getSampler(Renderer::SamplerType::Trilinear_Clamp), ImageViewType::Native, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	state.descriptor_bindings.bind(0, 2, "gbuffer - position", Renderer::instance()->getSampler(Renderer::SamplerType::Trilinear_Clamp), ImageViewType::Native, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	state.descriptor_bindings.bind(0, 3, "gbuffer - depth", Renderer::instance()->getSampler(Renderer::SamplerType::Trilinear_Clamp), ImageViewType::Native, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	state.descriptor_bindings.bind(0, 4, "gbuffer - metallic", Renderer::instance()->getSampler(Renderer::SamplerType::Trilinear_Clamp), ImageViewType::Native, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	state.descriptor_bindings.bind(0, 5, "gbuffer - roughness", Renderer::instance()->getSampler(Renderer::SamplerType::Trilinear_Clamp), ImageViewType::Native, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	state.descriptor_bindings.bind(0, 6, "directional_light_buffer", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	state.descriptor_bindings.bind(0, 7, "point_light_buffer", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	state.descriptor_bindings.bind(0, 8, "spot_light_buffer", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	state.declareAttachment("lighting_result", VK_FORMAT_R32G32B32A32_SFLOAT, Renderer::instance()->getRenderTargetExtent().width, Renderer::instance()->getRenderTargetExtent().height);
	state.addOutputAttachment("lighting_result", AttachmentState::Clear_Color);
}

void LightPass::resolveResources(ResolveState &resolve)
{
	resolve.resolve("directional_light_buffer", Renderer::instance()->getBuffer(Renderer::BufferType::DirectionalLight));
	resolve.resolve("point_light_buffer", Renderer::instance()->getBuffer(Renderer::BufferType::PointLight));
	resolve.resolve("spot_light_buffer", Renderer::instance()->getBuffer(Renderer::BufferType::SpotLight));
}

void LightPass::render(RenderPassState &state)
{
	auto &cmd_buffer = state.command_buffer;

	VkRenderPassBeginInfo begin_info = {};
	begin_info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin_info.renderPass            = state.pass.render_pass;
	begin_info.renderArea            = state.pass.render_area;
	begin_info.framebuffer           = state.pass.frame_buffer;
	begin_info.clearValueCount       = static_cast<uint32_t>(state.pass.clear_values.size());
	begin_info.pClearValues          = state.pass.clear_values.data();

	vkCmdBeginRenderPass(cmd_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(cmd_buffer, state.pass.bind_point, state.pass.pipeline);

	for (auto &descriptor_set : state.pass.descriptor_sets)
	{
		vkCmdBindDescriptorSets(cmd_buffer, state.pass.bind_point, state.pass.pipeline_layout, descriptor_set.index(), 1, &descriptor_set.getDescriptorSet(), 0, nullptr);
	}

	auto &extent = Renderer::instance()->getRenderTargetExtent();

	VkViewport viewport = {0, 0, static_cast<float>(extent.width), static_cast<float>(extent.height), 0, 1};
	VkRect2D   scissor  = {0, 0, extent.width, extent.height};

	vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
	vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

	uint32_t   directional_light_count = 0, spot_light_count = 0, point_light_count = 0;
	const auto group = Scene::instance()->getRegistry().group<>(entt::get<cmpt::Light, cmpt::Tag>);
	group.each([&](const entt::entity &entity, const cmpt::Light &light, const cmpt::Tag &tag) {
		if (!tag.active || !light.impl)
		{
			return;
		}

		switch (light.type)
		{
			case cmpt::LightType::Directional:
				directional_light_count++;
				break;
			case cmpt::LightType::Spot:
				spot_light_count++;
				break;
			case cmpt::LightType::Point:
				point_light_count++;
				break;
			default:
				break;
		}
	});

	vkCmdPushConstants(cmd_buffer, state.pass.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &directional_light_count);
	vkCmdPushConstants(cmd_buffer, state.pass.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t), &spot_light_count);
	vkCmdPushConstants(cmd_buffer, state.pass.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 2 * sizeof(uint32_t), sizeof(uint32_t), &point_light_count);
	vkCmdPushConstants(cmd_buffer, state.pass.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 3 * sizeof(uint32_t), sizeof(glm::vec3), glm::value_ptr(Renderer::instance()->Main_Camera.position));
	vkCmdDraw(cmd_buffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(cmd_buffer);
}
}        // namespace Ilum::pass
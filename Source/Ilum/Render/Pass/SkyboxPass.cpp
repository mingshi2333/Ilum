#include "SkyboxPass.hpp"

#include <RHI/DescriptorState.hpp>
#include <RHI/FrameBuffer.hpp>

#include <Render/RGBuilder.hpp>
#include <Render/Renderer.hpp>

#include <Scene/Scene.hpp>

namespace Ilum
{
SkyboxPass::SkyboxPass() :
    RenderPass("SkyboxPass")
{
}

void SkyboxPass::Create(RGBuilder &builder)
{
	std::unique_ptr<RenderPass> pass = std::make_unique<SkyboxPass>();

	auto output = builder.CreateTexture(
	    "Output",
	    TextureDesc{
	        builder.GetRenderer().GetExtent().width,  /*width*/
	        builder.GetRenderer().GetExtent().height, /*height*/
	        1,                                        /*depth*/
	        1,                                        /*mips*/
	        1,                                        /*layers*/
	        VK_SAMPLE_COUNT_1_BIT,
	        VK_FORMAT_R16G16B16A16_SFLOAT,
	        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
	    TextureState{VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT});

	auto depth_buffer = builder.CreateTexture(
	    "Depth",
	    TextureDesc{
	        builder.GetRenderer().GetExtent().width,  /*width*/
	        builder.GetRenderer().GetExtent().height, /*height*/
	        1,                                        /*depth*/
	        1,                                        /*mips*/
	        1,                                        /*layers*/
	        VK_SAMPLE_COUNT_1_BIT,
	        VK_FORMAT_D32_SFLOAT,
	        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT},
	    TextureState{VK_IMAGE_USAGE_SAMPLED_BIT});

	pass->AddResource(output);
	pass->AddResource(depth_buffer);

	TextureViewDesc output_view_desc  = {};
	output_view_desc.aspect           = VK_IMAGE_ASPECT_COLOR_BIT;
	output_view_desc.view_type        = VK_IMAGE_VIEW_TYPE_2D;
	output_view_desc.base_array_layer = 0;
	output_view_desc.base_mip_level   = 0;
	output_view_desc.layer_count      = 1;
	output_view_desc.level_count      = 1;

	TextureViewDesc depth_buffer_view_desc  = {};
	depth_buffer_view_desc.aspect           = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depth_buffer_view_desc.view_type        = VK_IMAGE_VIEW_TYPE_2D;
	depth_buffer_view_desc.base_array_layer = 0;
	depth_buffer_view_desc.base_mip_level   = 0;
	depth_buffer_view_desc.layer_count      = 1;
	depth_buffer_view_desc.level_count      = 1;

	ShaderDesc vertex_shader  = {};
	vertex_shader.filename    = "./Source/Shaders/Shading/Skybox.hlsl";
	vertex_shader.entry_point = "VSmain";
	vertex_shader.stage       = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader.type        = ShaderType::HLSL;

	ShaderDesc fragment_shader  = {};
	fragment_shader.filename    = "./Source/Shaders/Shading/Skybox.hlsl";
	fragment_shader.entry_point = "PSmain";
	fragment_shader.stage       = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader.type        = ShaderType::HLSL;

	DynamicState dynamic_state = {};
	dynamic_state.dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);
	dynamic_state.dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);

	ColorBlendState color_blend_state = {};
	color_blend_state.attachment_states.push_back(ColorBlendAttachmentState{});

	DepthStencilState depth_buffer_state = {};
	depth_buffer_state.depth_compare_op  = VK_COMPARE_OP_LESS_OR_EQUAL;
	depth_buffer_state.depth_write_enable  = VK_FALSE;

	PipelineState pso;
	pso.SetDynamicState(dynamic_state);
	pso.SetColorBlendState(color_blend_state);
	pso.SetDepthStencilState(depth_buffer_state);

	pso.LoadShader(vertex_shader);
	pso.LoadShader(fragment_shader);

	pass->BindCallback([=](CommandBuffer &cmd_buffer, const RGResources &resource, Renderer &renderer) {
		FrameBuffer framebuffer;
		framebuffer.Bind(resource.GetTexture(output), output_view_desc, ColorAttachmentInfo{});
		cmd_buffer.BeginRenderPass(framebuffer);
		cmd_buffer.Bind(pso);
		cmd_buffer.Bind(
		    cmd_buffer.GetDescriptorState()
		        .Bind(0, 0, &renderer.GetScene()->GetMainCameraBuffer())
		        .Bind(0, 1, resource.GetTexture(depth_buffer)->GetView(TextureViewDesc{VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}))
		        .Bind(0, 2, renderer.GetSkybox().GetView(TextureViewDesc{VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6}))
		        .Bind(0, 3, renderer.GetSampler(SamplerType::TrilinearClamp)));
		cmd_buffer.SetViewport(static_cast<float>(renderer.GetExtent().width), -static_cast<float>(renderer.GetExtent().height), 0, static_cast<float>(renderer.GetExtent().height));
		cmd_buffer.SetScissor(renderer.GetExtent().width, renderer.GetExtent().height);
		cmd_buffer.Draw(36);
		cmd_buffer.EndRenderPass();
	});

	pass->BindImGui([=](ImGuiContext &, const RGResources &) {

	});

	builder.AddPass(std::move(pass));
}

}        // namespace Ilum
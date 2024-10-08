#include "Components/Renderable/SkinnedMeshRenderer.hpp"
#include <Scene/Node.hpp>

#include <imgui.h>

namespace Ilum
{
namespace Cmpt
{
SkinnedMeshRenderer::SkinnedMeshRenderer(Node *node) :
    Renderable("Skinned Mesh Renderer", node)
{
}

bool SkinnedMeshRenderer::OnImGui()
{
	if (ImGui::TreeNode("Submesh"))
	{
		for (auto &submesh : m_submeshes)
		{
			ImGui::PushID(submesh.c_str());

			if (ImGui::Button(submesh.c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.8f, 30.f)))
			{
				submesh  = "";
				m_update = true;
			}

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("SkinnedMesh", submesh.c_str(), submesh.length() + 1);
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const auto *pay_load = ImGui::AcceptDragDropPayload("SkinnedMesh"))
				{
					submesh  = static_cast<const char *>(pay_load->Data);
					m_update = true;
				}
			}

			ImGui::PopID();
		}

		if (ImGui::Button("+"))
		{
			m_submeshes.emplace_back("");
			m_update = true;
		}

		ImGui::SameLine();

		if (ImGui::Button("-"))
		{
			m_submeshes.pop_back();
			m_update = true;
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Animation"))
	{
		for (auto &animation : m_animations)
		{
			ImGui::PushID(animation.c_str());

			if (ImGui::Button(animation.c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.8f, 30.f)))
			{
				animation = "";
				m_update  = true;
			}

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("Animation", animation.c_str(), animation.length() + 1);
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const auto *pay_load = ImGui::AcceptDragDropPayload("Animation"))
				{
					animation = static_cast<const char *>(pay_load->Data);
					m_update  = true;
				}
			}

			ImGui::PopID();
		}

		if (ImGui::Button("+"))
		{
			m_animations.emplace_back("");
			m_update = true;
		}

		ImGui::SameLine();

		if (ImGui::Button("-"))
		{
			m_animations.pop_back();
			m_update = true;
		}

		ImGui::TreePop();
	}

	m_update |= Renderable::OnImGui();

	return m_update;
}

std::type_index SkinnedMeshRenderer::GetType() const
{
	return typeid(SkinnedMeshRenderer);
}
}        // namespace Cmpt
}        // namespace Ilum
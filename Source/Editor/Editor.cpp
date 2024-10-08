#include "Editor.hpp"
#include "ImGui/ImGuiContext.hpp"
#include "Widget.hpp"

#include <Core/Plugin.hpp>
#include <Scene/Components/Camera/Camera.hpp>
#include <Scene/Node.hpp>

#include <imgui.h>

#include <filesystem>
#include <regex>

namespace Ilum
{
struct Editor::Impl
{
	std::unique_ptr<GuiContext> imgui_context = nullptr;

	RHIContext *rhi_context = nullptr;
	Renderer   *renderer    = nullptr;

	std::vector<std::unique_ptr<Widget>> widgets;

	Node *select = nullptr;

	Cmpt::Camera *main_camera = nullptr;
};

Editor::Editor(Window *window, RHIContext *rhi_context, Renderer *renderer)
{
	m_impl = new Impl;

	m_impl->imgui_context = std::make_unique<GuiContext>(rhi_context, window);
	m_impl->renderer      = renderer;
	m_impl->rhi_context   = rhi_context;

	for (const auto &file : std::filesystem::directory_iterator("shared/Editor/"))
	{
		m_impl->widgets.emplace_back(std::unique_ptr<Widget>(std::move(PluginManager::GetInstance().Call<Widget *>(file.path().string(), "Create", this, ImGui::GetCurrentContext()))));
	}
}

Editor::~Editor()
{
	m_impl->imgui_context.reset();
	delete m_impl;
}

void Editor::PreTick()
{
	m_impl->imgui_context->BeginFrame();
}

void Editor::Tick()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Widget"))
		{
			for (auto &widget : m_impl->widgets)
			{
				ImGui::MenuItem(widget->GetName().c_str(), nullptr, &widget->GetActive());
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	for (auto &widget : m_impl->widgets)
	{
		if (widget->GetActive())
		{
			widget->Tick();
		}
	}
}

void Editor::PostTick()
{
	m_impl->imgui_context->EndFrame();
	m_impl->imgui_context->Render();
}

Renderer *Editor::GetRenderer() const
{
	return m_impl->renderer;
}

RHIContext *Editor::GetRHIContext() const
{
	return m_impl->rhi_context;
}

Window *Editor::GetWindow() const
{
	return m_impl->imgui_context->GetWindow();
}

void Editor::SelectNode(Node *node)
{
	m_impl->select = node;
}

void Editor::SetMainCamera(Cmpt::Camera *camera)
{
	m_impl->main_camera = camera;
}

Node *Editor::GetSelectedNode() const
{
	return m_impl->select;
}

Cmpt::Camera *Editor::GetMainCamera() const
{
	return m_impl->main_camera;
}
}        // namespace Ilum
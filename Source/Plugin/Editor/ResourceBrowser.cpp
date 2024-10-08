#include <Editor/Editor.hpp>
#include <Editor/Widget.hpp>
#include <Renderer/Renderer.hpp>
#include <Resource/Resource.hpp>
#include <Resource/ResourceManager.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <nfd.h>

using namespace Ilum;

class ResourceBrowser : public Widget
{
  public:
	ResourceBrowser(Editor *editor) :
	    Widget("Resource Browser", editor)
	{
	}

	virtual ~ResourceBrowser() = default;

	virtual void Tick() override
	{
		if (!ImGui::Begin(m_name.c_str()))
		{
			ImGui::End();
			return;
		}

		auto *resource_manager = p_editor->GetRenderer()->GetResourceManager();

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 200.f);

		ImGui::BeginChild("Resource Browser Category", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (ImGui::Button("Import"))
		{
			char *path = nullptr;
			if (NFD_OpenDialog("jpg,png,bmp,jpeg,dds,hdr,gltf,obj,glb,fbx,ply,blend,dae,mat", Path::GetInstance().GetCurrent(false).c_str(), &path) == NFD_OKAY)
			{
				ResourceType type = m_resource_map.at(Path::GetInstance().GetFileExtension(path));
				switch (type)
				{
					case ResourceType::Prefab:
						resource_manager->Import<ResourceType::Prefab>(path);
						break;
					case ResourceType::Texture2D:
						resource_manager->Import<ResourceType::Texture2D>(path);
						break;
					case ResourceType::TextureCube:
						resource_manager->Import<ResourceType::TextureCube>(path);
						break;
					default:
						break;
				}
			}
		}

		for (auto &[type, name] : m_resource_types)
		{
			bool open = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen | (m_current_type == type ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf);
			if (ImGui::IsItemClicked())
			{
				m_current_type = type;
			}

			if (open)
			{
				ImGui::TreePop();
			}
		}

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("Resource Browser Viewer", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		std::unordered_map<ResourceType, std::function<void(void)>> draw_resources = {
#define DRAW_RESOURCE(TYPE)                                          \
	{                                                                \
		TYPE, [&]() { DrawResource<TYPE>(resource_manager, 100.f); } \
	}

		    DRAW_RESOURCE(ResourceType::Prefab),
		    DRAW_RESOURCE(ResourceType::Mesh),
		    DRAW_RESOURCE(ResourceType::SkinnedMesh),
		    DRAW_RESOURCE(ResourceType::Texture2D),
		    DRAW_RESOURCE(ResourceType::TextureCube),
		    DRAW_RESOURCE(ResourceType::Material),
		    DRAW_RESOURCE(ResourceType::Animation),
		    DRAW_RESOURCE(ResourceType::RenderPipeline),
		    DRAW_RESOURCE(ResourceType::Scene),
		};

		draw_resources.at(m_current_type)();

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();

		ImGui::End();
	}

  private:
	template <ResourceType _Ty>
	inline void DrawResource(ResourceManager *manager, float button_size)
	{
		float width = 0.f;

		ImGuiStyle &style    = ImGui::GetStyle();
		style.ItemSpacing    = ImVec2(10.f, 10.f);
		float window_visible = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		const std::vector<std::string> resources = manager->GetResources<_Ty>(false);

		for (const auto &resource_name : resources)
		{
			auto *thumbnail = manager->GetThumbnail<_Ty>(resource_name);

			ImGui::PushID(resource_name.c_str());
			ImGui::ImageButton(thumbnail ? thumbnail : ImGui::GetIO().Fonts->TexID, ImVec2{button_size, button_size});

			// Drag&Drop source
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload(m_resource_types.at(_Ty), resource_name.c_str(), resource_name.length() + 1);
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginPopupContextItem(resource_name.c_str()))
			{
				if (ImGui::MenuItem("Delete"))
				{
					manager->Erase<_Ty>(resource_name);
					ImGui::EndPopup();
					ImGui::PopID();
					return;
				}
				ImGui::EndPopup();
			}
			else if (ImGui::IsItemHovered() && ImGui::IsWindowFocused())
			{
				ImVec2 pos = ImGui::GetIO().MousePos;
				ImGui::SetNextWindowPos(ImVec2(pos.x + 10.f, pos.y + 10.f));
				ImGui::Begin(resource_name.c_str(), NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
				ImGui::Text(resource_name.c_str());
				ImGui::End();
			}

			float last_button = ImGui::GetItemRectMax().x;
			float next_button = last_button + style.ItemSpacing.x + button_size;
			if (next_button < window_visible)
			{
				ImGui::SameLine();
			}

			ImGui::PopID();
		}
	}

  private:
	ResourceType m_current_type = ResourceType::Prefab;

	std::unordered_map<ResourceType, const char *const> m_resource_types = {
	    {ResourceType::Mesh, "Mesh"},
	    {ResourceType::SkinnedMesh, "SkinnedMesh"},
	    {ResourceType::Prefab, "Prefab"},
	    {ResourceType::Texture2D, "Texture2D"},
	    {ResourceType::TextureCube, "TextureCube"},
	    {ResourceType::Animation, "Animation"},
	    {ResourceType::Material, "Material"},
	    {ResourceType::RenderPipeline, "RenderPipeline"},
	    {ResourceType::Scene, "Scene"},
	};

	std::unordered_map<std::string, ResourceType> m_resource_map = {
	    {".jpg", ResourceType::Texture2D},
	    {".png", ResourceType::Texture2D},
	    {".bmp", ResourceType::Texture2D},
	    {".jpeg", ResourceType::Texture2D},
	    {".dds", ResourceType::Texture2D},
	    {".hdr", ResourceType::TextureCube},
	    {".gltf", ResourceType::Prefab},
	    {".obj", ResourceType::Prefab},
	    {".glb", ResourceType::Prefab},
	    {".fbx", ResourceType::Prefab},
	    {".ply", ResourceType::Prefab},
	    {".dae", ResourceType::Prefab},
	};
};

extern "C"
{
	EXPORT_API ResourceBrowser *Create(Editor *editor, ImGuiContext *context)
	{
		ImGui::SetCurrentContext(context);
		return new ResourceBrowser(editor);
	}
}
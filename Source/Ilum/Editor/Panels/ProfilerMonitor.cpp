#include "ProfilerMonitor.hpp"

#include "Graphics/GraphicsContext.hpp"
#include "Graphics/Profiler.hpp"

#include "Renderer/Renderer.hpp"

#include "Timing/Timer.hpp"

#include <imgui.h>

namespace Ilum::panel
{
ProfilerMonitor::ProfilerMonitor()
{
	m_name = "Profiler";
	m_stopwatch.start();
}

void ProfilerMonitor::draw(float delta_time)
{
	ImGui::Begin("Profiler", &active);

	if (m_stopwatch.elapsedSecond() > 0.1f)
	{
		m_profile_result = GraphicsContext::instance()->getProfiler().getResult();

		if (Timer::instance()->getFPS() != 0.0)
		{
			if (m_frame_times.size() < 50)
			{
				m_frame_times.push_back(1000.0f / static_cast<float>(Timer::instance()->getFPS()));
			}
			else
			{
				std::rotate(m_frame_times.begin(), m_frame_times.begin() + 1, m_frame_times.end());
				m_frame_times.back() = 1000.0f / static_cast<float>(Timer::instance()->getFPS());
			}
		}

		m_stopwatch.start();
	}

	float min_frame_time = 0.f, max_frame_time = 0.f;
	float max_cpu_time = 0.f, max_gpu_time = 0.f;

	if (!m_frame_times.empty())
	{
		min_frame_time = *std::min_element(m_frame_times.begin(), m_frame_times.end());
		max_frame_time = *std::max_element(m_frame_times.begin(), m_frame_times.end());
	}

	std::vector<float> cpu_times;
	std::vector<float> gpu_times;
	uint32_t           index = 0;

	if (ImGui::BeginTable("CPU&GPU Time", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
	{
		ImGui::TableSetupColumn("Index");
		ImGui::TableSetupColumn("Pass");
		ImGui::TableSetupColumn("CPU Time (ms)");
		ImGui::TableSetupColumn("GPU Time (ms)");
		ImGui::TableHeadersRow();

		for (auto &[name, res] : m_profile_result)
		{
			auto [cpu_time, gpu_time] = res;

			cpu_times.push_back(cpu_time);
			gpu_times.push_back(gpu_time);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%d", index++);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", name.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%f", cpu_time);
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%f", gpu_time);
		}

		ImGui::EndTable();
	}

	if (!m_profile_result.empty())
	{
		max_cpu_time = *std::max_element(cpu_times.begin(), cpu_times.end());
		max_gpu_time = *std::max_element(gpu_times.begin(), gpu_times.end());
	}

	ImGui::PlotLines(("Frame Times (" + std::to_string(static_cast<uint32_t>(Timer::instance()->getFPS())) + "fps)").c_str(), m_frame_times.data(), static_cast<int>(m_frame_times.size()), 0, nullptr, min_frame_time * 0.8f, max_frame_time * 1.2f, ImVec2{0, 80});
	ImGui::PlotHistogram("CPU Times", cpu_times.data(), static_cast<int>(cpu_times.size()), 0, nullptr, 0.f, max_cpu_time * 1.2f, ImVec2(0, 80.0f));
	ImGui::PlotHistogram("GPU Times", gpu_times.data(), static_cast<int>(gpu_times.size()), 0, nullptr, 0.f, max_gpu_time * 1.2f, ImVec2(0, 80.0f));

	ImGui::Text("Total Triangle Count: %d", Renderer::instance()->Indices_Count / 3);
	ImGui::Text("Total Instance Count: %d", Renderer::instance()->Instance_Count);
	ImGui::Text("Total Meshlet Count: %d", Renderer::instance()->Meshlet_Count);
	ImGui::Text("Total Meshlet Visible: %d", Renderer::instance()->Meshlet_Visible);

	ImGui::End();
}
}        // namespace Ilum::panel
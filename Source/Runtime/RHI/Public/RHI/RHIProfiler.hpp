#pragma once

#include "Fwd.hpp"

namespace Ilum
{
struct ProfileState
{
	std::chrono::time_point<std::chrono::high_resolution_clock> cpu_start;
	std::chrono::time_point<std::chrono::high_resolution_clock> cpu_end;

	uint64_t gpu_start = 0;
	uint64_t gpu_end   = 0;

	float cpu_time = 0.f;
	float gpu_time = 0.f;

	std::thread::id thread_id;
};

class RHIProfiler
{
  public:
	RHIProfiler(RHIDevice *device, uint32_t frame_count = 3);

	virtual ~RHIProfiler() = default;

	static std::unique_ptr<RHIProfiler> Create(RHIDevice *device, uint32_t frame_count = 3);

	const ProfileState &GetProfileState() const;

	virtual void Begin(RHICommand *cmd_buffer, uint32_t frame_index) = 0;

	virtual void End(RHICommand *cmd_buffer) = 0;

  protected:
	RHIDevice *p_device = nullptr;
	uint32_t   m_frame_count;
	uint32_t   m_current_index = 0;
	ProfileState m_state;
};
}        // namespace Ilum
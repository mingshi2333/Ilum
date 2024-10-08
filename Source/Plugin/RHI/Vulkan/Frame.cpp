#include "Frame.hpp"
#include "Command.hpp"
#include "Device.hpp"
#include "Synchronization.hpp"
#include "Descriptor.hpp"

namespace Ilum::Vulkan
{
Frame::Frame(RHIDevice *device) :
    RHIFrame(device)
{
}

Frame::~Frame()
{
	p_device->WaitIdle();

	m_fences.clear();
	m_semaphores.clear();
	m_commands.clear();

	for (auto &[hash, pool] : m_command_pools)
	{
		vkDestroyCommandPool(static_cast<Device *>(p_device)->GetDevice(), pool, nullptr);
	}

	m_command_pools.clear();
}

RHIFence *Frame::AllocateFence()
{
	if (m_fences.size() > m_active_fence_index)
	{
		return m_fences[m_active_fence_index++].get();
	}

	while (m_fences.size() <= m_active_fence_index)
	{
		m_fences.emplace_back(std::make_unique<Fence>(p_device));
	}

	m_active_fence_index++;

	return m_fences.back().get();
}

RHISemaphore *Frame::AllocateSemaphore()
{
	if (m_semaphores.size() > m_active_semaphore_index)
	{
		return m_semaphores[m_active_semaphore_index++].get();
	}

	while (m_semaphores.size() <= m_active_semaphore_index)
	{
		m_semaphores.emplace_back(std::make_unique<Semaphore>(p_device));
	}

	m_active_semaphore_index++;

	return m_semaphores.back().get();
}

RHICommand *Frame::AllocateCommand(RHIQueueFamily family)
{
	size_t hash = 0;
	HashCombine(hash, family, std::this_thread::get_id());

	if (m_command_pools.find(hash) == m_command_pools.end())
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		VkCommandPoolCreateInfo create_info = {};
		create_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.flags                   = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		create_info.queueFamilyIndex        = static_cast<Device *>(p_device)->GetQueueFamily(family);

		VkCommandPool pool = VK_NULL_HANDLE;
		vkCreateCommandPool(static_cast<Device *>(p_device)->GetDevice(), &create_info, nullptr, &pool);

		m_command_pools.emplace(hash, pool);
	}

	if (m_commands.find(hash) == m_commands.end())
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_commands.emplace(hash, std::vector<std::unique_ptr<Command>>{});
		m_active_cmd_index[hash] = 0;
	}

	if (m_commands.at(hash).size() > m_active_cmd_index.at(hash))
	{
		auto &cmd = m_commands.at(hash).at(m_active_cmd_index.at(hash));
		cmd->Init();
		m_active_cmd_index[hash]++;
		return cmd.get();
	}

	while (m_commands.at(hash).size() <= m_active_cmd_index.at(hash))
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_commands[hash].emplace_back(std::make_unique<Command>(p_device, m_command_pools[hash], family));
	}

	m_active_cmd_index[hash]++;

	auto &cmd = m_commands[hash].back();
	cmd->Init();
	return cmd.get();
}

RHIDescriptor *Frame::AllocateDescriptor(const ShaderMeta &meta)
{
	size_t hash = 0;
	HashCombine(hash, meta.hash, std::this_thread::get_id());

	if (m_descriptors.find(hash) == m_descriptors.end())
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_descriptors.emplace(hash, std::vector<std::unique_ptr<Descriptor>>{});
		m_active_descriptor_index[hash] = 0;
	}

	if (m_descriptors[hash].size() > m_active_descriptor_index[hash])
	{
		auto &descriptor = m_descriptors[hash][m_active_descriptor_index[hash]];
		m_active_descriptor_index[hash]++;
		return descriptor.get();
	}

	while (m_descriptors[hash].size() <= m_active_descriptor_index[hash])
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_descriptors[hash].emplace_back(std::make_unique<Descriptor>(p_device, meta));
	}

	m_active_descriptor_index[hash]++;

	auto &descriptor = m_descriptors[hash].back();
	return descriptor.get();
}

void Frame::Reset()
{
	std::vector<VkFence> fences;

	fences.reserve(m_fences.size());

	for (uint32_t i = 0; i < m_active_fence_index; i++)
	{
		fences.push_back(m_fences[i]->GetHandle());
	}

	if (!fences.empty())
	{
		vkWaitForFences(static_cast<Device *>(p_device)->GetDevice(), static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX);
		vkResetFences(static_cast<Device *>(p_device)->GetDevice(), static_cast<uint32_t>(fences.size()), fences.data());
	}

	for (auto &[hash, pool] : m_command_pools)
	{
		vkResetCommandPool(static_cast<Device *>(p_device)->GetDevice(), pool, 0);
		m_active_cmd_index[hash] = 0;
		for (auto &cmd : m_commands[hash])
		{
			cmd->SetState(CommandState::Available);
		}
	}

	for (auto& [hash, index] : m_active_descriptor_index)
	{
		index = 0;
	}

	m_active_fence_index     = 0;
	m_active_semaphore_index = 0;
}
}        // namespace Ilum::Vulkan
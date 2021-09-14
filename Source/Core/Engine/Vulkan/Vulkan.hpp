#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

#include <string>

#define VK_CHECK(result) Ilum::vk_check(result)
#define VK_ASSERT(result) Ilum::vk_assert(result)

namespace Ilum
{
const bool vk_check(VkResult result);

void vk_assert(VkResult result);
}

namespace std
{
std::string to_string(VkResult result);
}
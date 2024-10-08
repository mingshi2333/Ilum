#pragma once

#include "Fwd.hpp"

namespace Ilum::CUDA
{
class PipelineState : public RHIPipelineState
{
  public:
	PipelineState(RHIDevice *device);

	~PipelineState() = default;
};
}        // namespace Ilum::CUDA
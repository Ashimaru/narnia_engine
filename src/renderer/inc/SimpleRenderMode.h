#pragma once
#include "AbstractRenderMode.h"

class SimpleRenderMode : public AbstractRenderMode
{
public:
	explicit SimpleRenderMode(GPUPtr &gpu);
	~SimpleRenderMode() override = default;

	virtual void cleanUp() override;
	virtual vk::CommandBuffer* getCommandBuffer(const size_t index) override;

	friend class SimpleRenderModeFactory;

private:
	GPUPtr m_gpu;
};

using SimpleRenderModePtr = std::unique_ptr<SimpleRenderMode>;


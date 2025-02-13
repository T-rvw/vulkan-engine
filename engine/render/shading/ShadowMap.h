#pragma once

#include "CommonIncludes.h"
#include "render/shading/IShadowCaster.h"
#include "utils/Math.h"

class Renderer;
class IShadowCaster;

namespace Device
{
	class VulkanRenderState;
}

namespace ECS
{
	namespace systems
	{
		class CullingSystem;
	}
}

namespace render {
	
	struct ShadowCasterData;

	class ShadowMap {
	public:
		ShadowMap(unsigned int resolutionX, unsigned int resolutionY);

		ivec2 GetAtlasSize() const { return ivec2(4024, 4024); }
		void SetupShadowCasters(std::vector<ShadowCasterData>& shadow_casters);
	
	private:
		uvec2 _resolution;
		uvec2 _cellPixelSize;
		vec2 _cellSize;
		unsigned int _pixelSpacing = 2;
		uint32_t _shadowmapBlock;
		int32_t _shadowCasterCount;

	private:
		Rect getCellPixelRect(unsigned int index);
		Rect getCellRect(unsigned int index);
	};

}
#pragma once 

#include "CommonIncludes.h"

struct GLFWwindow;

namespace Device
{
	class VulkanRenderState;
	class ShaderCache;
}

namespace render
{
	struct EnvironmentSettings;
	struct DebugSettings;
}

namespace render { 

	namespace DebugUI 
	{
		void Initialize(GLFWwindow* window, Device::ShaderCache* shader_cache, EnvironmentSettings& environment_settings, DebugSettings& debug_settings);
		void Deinitialize();
		void NewFrame();
		void Update(float dt);
		void Render(Device::VulkanRenderState& state);
		void SwitchEngineStats();

		bool WantCaptureMouse();
		bool WantCaptureKeyboard();

		void SetEngineStatsVisible(bool visible);
		void SetMainWidgetVisible(bool visible);
		void SetEnvironmentWidgetVisible(bool visible);

	} 

}
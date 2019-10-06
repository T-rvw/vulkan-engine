#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <memory>
#include <optional>
#include <array>
#include <math.h>
#include <string>
#include <functional>
#include <atomic>
#include <filesystem>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb/stb_image.h"

#include "nlohmann/json.hpp"

#include <VulkanMemoryAllocator/vk_mem_alloc.h>

#include "utils/NonCopyable.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PI M_PI
#define RAD(value) (value * (float)PI / 180)
#define DEG(value) (value / (float)PI * 180)
#define M_TO_STRING(value) to_string(value)

using namespace glm;
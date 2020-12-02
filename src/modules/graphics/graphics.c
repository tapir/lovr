#include "graphics/graphics.h"
#include "event/event.h"
#include "core/gpu.h"
#include "core/os.h"
#include "core/util.h"
#include <string.h>

#ifdef LOVR_VK
const char** lovrPlatformGetVulkanInstanceExtensions(uint32_t* count);
uint32_t lovrPlatformCreateVulkanSurface(void* instance, void** surface);
#endif

static struct {
  bool initialized;
  bool debug;
  gpu_limits limits;
  int width;
  int height;
} state;

void onDebugMessage(void* context, const char* message, int severe) {
  lovrLog(severe ? LOG_ERROR : LOG_DEBUG, "GPU", message);
}

void onQuitRequest() {
  lovrEventPush((Event) { .type = EVENT_QUIT, .data.quit = { .exitCode = 0 } });
}

void onResizeWindow(int width, int height) {
  state.width = width;
  state.height = height;
  lovrEventPush((Event) { .type = EVENT_RESIZE, .data.resize = { width, height } });
}

bool lovrGraphicsInit(bool debug) {
  state.debug = debug;
  return false;
}

void lovrGraphicsDestroy() {
  if (!state.initialized) return;
  gpu_thread_detach();
  gpu_destroy();
  memset(&state, 0, sizeof(state));
}

void lovrGraphicsCreateWindow(WindowFlags* flags) {
  flags->debug = state.debug;
  lovrAssert(!state.initialized, "Window is already created");
  lovrAssert(lovrPlatformCreateWindow(flags), "Could not create window");
  lovrPlatformSetSwapInterval(flags->vsync); // Force vsync in case lovr.headset changed it in a previous restart
  lovrPlatformOnQuitRequest(onQuitRequest);
  lovrPlatformOnWindowResize(onResizeWindow);
  lovrPlatformGetFramebufferSize(&state.width, &state.height);

  gpu_config config = {
    .debug = state.debug,
    .limits = &state.limits,
    .callback = onDebugMessage,
    .vk.surface = true,
    .vk.vsync = flags->vsync,
    .vk.getExtraInstanceExtensions = lovrPlatformGetVulkanInstanceExtensions,
    .vk.createSurface = lovrPlatformCreateVulkanSurface
  };

  lovrAssert(gpu_init(&config), "Could not initialize GPU");
  gpu_thread_attach();
  state.initialized = true;
}

void lovrGraphicsGetLimits(double limits[MAX_LIMITS]) {
  limits[LIMIT_TEXTURE_SIZE_2D] = state.limits.textureSize2D;
  limits[LIMIT_TEXTURE_SIZE_3D] = state.limits.textureSize3D;
  limits[LIMIT_TEXTURE_SIZE_CUBE] = state.limits.textureSizeCube;
  limits[LIMIT_TEXTURE_LAYERS] = state.limits.textureLayers;
  limits[LIMIT_RENDER_WIDTH] = state.limits.canvasSize[0];
  limits[LIMIT_RENDER_HEIGHT] = state.limits.canvasSize[1];
  limits[LIMIT_RENDER_VIEWS] = state.limits.canvasViews;
  limits[LIMIT_SHADER_GROUPS] = state.limits.bundleCount;
  limits[LIMIT_SHADER_GROUP_ITEMS] = state.limits.bundleSlots;
  limits[LIMIT_INPUT_BUFFER_RANGE] = state.limits.uniformBufferRange;
  limits[LIMIT_INPUT_BUFFER_ALIGN] = state.limits.uniformBufferAlign;
  limits[LIMIT_COMPUTE_BUFFER_RANGE] = state.limits.storageBufferRange;
  limits[LIMIT_COMPUTE_BUFFER_ALIGN] = state.limits.storageBufferAlign;
  limits[LIMIT_VERTEX_ATTRIBUTES] = state.limits.vertexAttributes;
  limits[LIMIT_VERTEX_ATTRIBUTE_OFFSET] = state.limits.vertexAttributeOffset;
  limits[LIMIT_VERTEX_BUFFERS] = state.limits.vertexBuffers;
  limits[LIMIT_VERTEX_BUFFER_STRIDE] = state.limits.vertexBufferStride;
  limits[LIMIT_VERTEX_SHADER_OUTPUTS] = state.limits.vertexShaderOutputs;
  limits[LIMIT_COMPUTE_WIDTH] = state.limits.computeCount[0];
  limits[LIMIT_COMPUTE_HEIGHT] = state.limits.computeCount[1];
  limits[LIMIT_COMPUTE_DEPTH] = state.limits.computeCount[2];
  limits[LIMIT_COMPUTE_GROUP_WIDTH] = state.limits.computeGroupSize[0];
  limits[LIMIT_COMPUTE_GROUP_HEIGHT] = state.limits.computeGroupSize[1];
  limits[LIMIT_COMPUTE_GROUP_DEPTH] = state.limits.computeGroupSize[2];
  limits[LIMIT_COMPUTE_GROUP_VOLUME] = state.limits.computeGroupVolume;
  limits[LIMIT_COMPUTE_SHARED_MEMORY] = state.limits.computeSharedMemory;
  limits[LIMIT_ALLOCATION_SIZE] = state.limits.allocationSize;
  limits[LIMIT_POINT_SIZE] = state.limits.pointSize[1];
  limits[LIMIT_ANISOTROPY] = state.limits.anisotropy;
}

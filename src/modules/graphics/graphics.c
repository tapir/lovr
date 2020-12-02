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

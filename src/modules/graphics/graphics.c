#include "graphics/graphics.h"
#include "event/event.h"
#include "core/gpu.h"
#include "core/ref.h"
#include "core/os.h"
#include "core/util.h"
#include <string.h>
#include <stdlib.h>

struct Buffer {
  gpu_buffer* gpu;
  BufferInfo info;
};

#ifdef LOVR_VK
const char** lovrPlatformGetVulkanInstanceExtensions(uint32_t* count);
uint32_t lovrPlatformCreateVulkanSurface(void* instance, void** surface);
#endif

static struct {
  bool initialized;
  bool debug;
  gpu_features features;
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
    .features = &state.features,
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

bool lovrGraphicsHasWindow() {
  return lovrPlatformHasWindow();
}

uint32_t lovrGraphicsGetWidth() {
  return state.width;
}

uint32_t lovrGraphicsGetHeight() {
  return state.height;
}

float lovrGraphicsGetPixelDensity() {
  int width, height, framebufferWidth, framebufferHeight;
  lovrPlatformGetWindowSize(&width, &height);
  lovrPlatformGetFramebufferSize(&framebufferWidth, &framebufferHeight);
  if (width == 0 || framebufferWidth == 0) {
    return 0.f;
  } else {
    return (float) framebufferWidth / (float) width;
  }
}

void lovrGraphicsGetFeatures(GraphicsFeatures* features) {
  features->bptc = state.features.bptc;
  features->astc = state.features.astc;
  features->pointSize = state.features.pointSize;
  features->wireframe = state.features.wireframe;
  features->anisotropy = state.features.anisotropy;
  features->clipDistance = state.features.clipDistance;
  features->cullDistance = state.features.cullDistance;
  features->fullIndexBufferRange = state.features.fullIndexBufferRange;
  features->indirectDrawCount = state.features.indirectDrawCount;
  features->indirectDrawFirstInstance = state.features.indirectDrawFirstInstance;
  features->extraShaderInputs = state.features.extraShaderInputs;
  features->multiview = state.features.multiview;
}

void lovrGraphicsGetLimits(GraphicsLimits* limits) {
  limits->textureSize2D = state.limits.textureSize2D;
  limits->textureSize3D = state.limits.textureSize3D;
  limits->textureSizeCube = state.limits.textureSizeCube;
  limits->textureLayers = state.limits.textureLayers;
  limits->canvasSize[0] = state.limits.canvasSize[0];
  limits->canvasSize[1] = state.limits.canvasSize[1];
  limits->canvasViews = state.limits.canvasViews;
  limits->bundleCount = state.limits.bundleCount;
  limits->bundleSlots = state.limits.bundleSlots;
  limits->uniformBufferRange = state.limits.uniformBufferRange;
  limits->storageBufferRange = state.limits.storageBufferRange;
  limits->uniformBufferAlign = state.limits.uniformBufferAlign;
  limits->storageBufferAlign = state.limits.storageBufferAlign;
  limits->vertexAttributes = state.limits.vertexAttributes;
  limits->vertexAttributeOffset = state.limits.vertexAttributeOffset;
  limits->vertexBuffers = state.limits.vertexBuffers;
  limits->vertexBufferStride = state.limits.vertexBufferStride;
  limits->vertexShaderOutputs = state.limits.vertexShaderOutputs;
  memcpy(limits->computeCount, state.limits.computeCount, 3 * sizeof(uint32_t));
  memcpy(limits->computeGroupSize, state.limits.computeGroupSize, 3 * sizeof(uint32_t));
  limits->computeGroupVolume = state.limits.computeGroupVolume;
  limits->computeSharedMemory = state.limits.computeSharedMemory;
  limits->indirectDrawCount = state.limits.indirectDrawCount;
  limits->allocationSize = state.limits.allocationSize;
  limits->pointSize[0] = state.limits.pointSize[0];
  limits->pointSize[1] = state.limits.pointSize[1];
  limits->anisotropy = state.limits.anisotropy;
}

void lovrGraphicsBegin() {
  gpu_begin();
}

void lovrGraphicsFlush() {
  gpu_flush();
}

Buffer* lovrBufferCreate(BufferInfo* info) {
  Buffer* buffer = _lovrAlloc(sizeof(Buffer) + gpu_sizeof_buffer());
  buffer->gpu = (gpu_buffer*) (buffer + 1);
  buffer->info = *info;

  gpu_buffer_usage gpuBufferUsage[] = {
    [BUFFER_VERTEX] = GPU_BUFFER_USAGE_VERTEX,
    [BUFFER_INDEX] = GPU_BUFFER_USAGE_INDEX,
    [BUFFER_UNIFORM] = GPU_BUFFER_USAGE_UNIFORM,
    [BUFFER_COMPUTE] = GPU_BUFFER_USAGE_STORAGE,
    [BUFFER_ARGUMENT] = GPU_BUFFER_USAGE_INDIRECT,
    [BUFFER_UPLOAD] = GPU_BUFFER_USAGE_UPLOAD,
    [BUFFER_DOWNLOAD] = GPU_BUFFER_USAGE_DOWNLOAD
  };

  uint32_t usage = 0;
  for (uint32_t i = 0; i < sizeof(gpuBufferUsage) / sizeof(gpuBufferUsage[0]); i++) {
    if (info->usage & (1 << i)) {
      usage |= gpuBufferUsage[i];
    }
  }

  gpu_buffer_info gpuInfo = {
    .size = info->size,
    .usage = usage,
    .label = info->label
  };

  if (!gpu_buffer_init(buffer->gpu, &gpuInfo)) {
    lovrRelease(Buffer, buffer);
    return NULL;
  }

  return buffer;
}

void lovrBufferDestroy(void* ref) {
  Buffer* buffer = ref;
  gpu_buffer_destroy(buffer->gpu);
}

const BufferInfo* lovrBufferGetInfo(Buffer* buffer) {
  return &buffer->info;
}

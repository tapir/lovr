#include <stdbool.h>
#include <stdint.h>

#pragma once

struct WindowFlags;

typedef struct {
  bool bptc;
  bool astc;
  bool pointSize;
  bool wireframe;
  bool anisotropy;
  bool clipDistance;
  bool cullDistance;
  bool fullIndexBufferRange;
  bool indirectDrawCount;
  bool indirectDrawFirstInstance;
  bool extraShaderInputs;
  bool multiview;
} GraphicsFeatures;

typedef struct {
  uint32_t textureSize2D;
  uint32_t textureSize3D;
  uint32_t textureSizeCube;
  uint32_t textureLayers;
  uint32_t canvasSize[2];
  uint32_t canvasViews;
  uint32_t bundleCount;
  uint32_t bundleSlots;
  uint32_t uniformBufferRange;
  uint32_t storageBufferRange;
  uint32_t uniformBufferAlign;
  uint32_t storageBufferAlign;
  uint32_t vertexAttributes;
  uint32_t vertexAttributeOffset;
  uint32_t vertexBuffers;
  uint32_t vertexBufferStride;
  uint32_t vertexShaderOutputs;
  uint32_t computeCount[3];
  uint32_t computeGroupSize[3];
  uint32_t computeGroupVolume;
  uint32_t computeSharedMemory;
  uint32_t indirectDrawCount;
  uint64_t allocationSize;
  uint32_t pointSize[2];
  float anisotropy;
} GraphicsLimits;

bool lovrGraphicsInit(bool debug);
void lovrGraphicsDestroy(void);
void lovrGraphicsCreateWindow(struct WindowFlags* flags);
void lovrGraphicsGetFeatures(GraphicsFeatures* features);
void lovrGraphicsGetLimits(GraphicsLimits* limits);

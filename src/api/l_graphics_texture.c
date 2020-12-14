#include "api.h"
#include "graphics/graphics.h"
#include "data/textureData.h"
#include "core/ref.h"
#include <stdlib.h>

static int l_lovrTextureGetType(lua_State* L) {
  Texture* texture = luax_checktype(L, 1, Texture);
  const TextureInfo* info = lovrTextureGetInfo(texture);
  luax_pushenum(L, TextureType, info->type);
  return 1;
}

static int l_lovrTextureGetFormat(lua_State* L) {
  Texture* texture = luax_checktype(L, 1, Texture);
  const TextureInfo* info = lovrTextureGetInfo(texture);
  luax_pushenum(L, TextureFormat, info->format);
  return 1;
}

static int l_lovrTextureGetWidth(lua_State* L) {
  Texture* texture = luax_checktype(L, 1, Texture);
  const TextureInfo* info = lovrTextureGetInfo(texture);
  lua_pushinteger(L, info->size[0]);
  return 1;
}

static int l_lovrTextureGetHeight(lua_State* L) {
  Texture* texture = luax_checktype(L, 1, Texture);
  const TextureInfo* info = lovrTextureGetInfo(texture);
  lua_pushinteger(L, info->size[1]);
  return 1;
}

static int l_lovrTextureGetDepth(lua_State* L) {
  Texture* texture = luax_checktype(L, 1, Texture);
  const TextureInfo* info = lovrTextureGetInfo(texture);
  lua_pushinteger(L, info->size[2]);
  return 1;
}

static int l_lovrTextureGetDimensions(lua_State* L) {
  Texture* texture = luax_checktype(L, 1, Texture);
  const TextureInfo* info = lovrTextureGetInfo(texture);
  lua_pushinteger(L, info->size[0]);
  lua_pushinteger(L, info->size[1]);
  lua_pushinteger(L, info->size[2]);
  return 3;
}

static int l_lovrTextureGetMipmapCount(lua_State* L) {
  Texture* texture = luax_checktype(L, 1, Texture);
  const TextureInfo* info = lovrTextureGetInfo(texture);
  lua_pushinteger(L, info->mipmaps);
  return 1;
}

static int l_lovrTextureGetSampleCount(lua_State* L) {
  Texture* texture = luax_checktype(L, 1, Texture);
  const TextureInfo* info = lovrTextureGetInfo(texture);
  lua_pushinteger(L, info->samples);
  return 1;
}

typedef struct {
  lua_State* L;
  int ref;
  TextureFormat format;
  uint32_t width;
  uint32_t height;
} luax_readback;

static void onReadback(void* data, uint64_t size, void* context) {
  luax_readback* readback = context;

  TextureData* textureData = lovrTextureDataCreate(readback->width, readback->height, NULL, 0, readback->format);
  memcpy(textureData->blob->data, data, size);

  lua_rawgeti(readback->L, LUA_REGISTRYINDEX, readback->ref);
  luax_pushtype(readback->L, TextureData, textureData);
  lovrRelease(TextureData, textureData);
  lua_call(readback->L, 1, 0);
  free(readback);
}

static int l_lovrTextureGetPixels(lua_State* L) {
  Texture* texture = luax_checktype(L, 1, Texture);
  const TextureInfo* info = lovrTextureGetInfo(texture);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  uint32_t x = luaL_optinteger(L, 3, 0);
  uint32_t y = luaL_optinteger(L, 4, 0);
  uint32_t w = luaL_optinteger(L, 5, info->size[0] - x);
  uint32_t h = luaL_optinteger(L, 6, info->size[1] - y);
  uint32_t layer = luaL_optinteger(L, 7, 1);
  uint32_t level = luaL_optinteger(L, 8, 1);
  luax_readback* readback = malloc(sizeof(luax_readback));
  lovrAssert(readback, "Out of memory");
  readback->L = L;
  lua_pushvalue(L, 2);
  readback->ref = luaL_ref(L, LUA_REGISTRYINDEX);
  readback->format = info->format;
  readback->width = w;
  readback->height = h;
  lovrTextureGetPixels(texture, x, y, w, h, layer, level, onReadback, readback);
  return 0;
}

static int l_lovrTextureSetPixels(lua_State* L) {
  //
  return 0;
}

const luaL_Reg lovrTexture[] = {
  { "getType", l_lovrTextureGetType },
  { "getFormat", l_lovrTextureGetFormat },
  { "getWidth", l_lovrTextureGetWidth },
  { "getHeight", l_lovrTextureGetHeight },
  { "getDepth", l_lovrTextureGetDepth },
  { "getDimensions", l_lovrTextureGetDimensions },
  { "getMipmapCount", l_lovrTextureGetMipmapCount },
  { "getSampleCount", l_lovrTextureGetSampleCount },
  { "getPixels", l_lovrTextureGetPixels },
  { "setPixels", l_lovrTextureSetPixels },
  { NULL, NULL }
};

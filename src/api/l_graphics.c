#include "api.h"
#include "graphics/graphics.h"
#include "data/textureData.h"
#include "core/os.h"
#include "core/ref.h"
#include <stdlib.h>

// Must be released when done
static TextureData* luax_checktexturedata(lua_State* L, int index, bool flip) {
  TextureData* textureData = luax_totype(L, index, TextureData);

  if (textureData) {
    lovrRetain(textureData);
  } else {
    Blob* blob = luax_readblob(L, index, "Texture");
    textureData = lovrTextureDataCreateFromBlob(blob, flip);
    lovrRelease(Blob, blob);
  }

  return textureData;
}

static int l_lovrGraphicsCreateWindow(lua_State* L) {
  WindowFlags flags;
  memset(&flags, 0, sizeof(flags));

  if (!lua_toboolean(L, 1)) {
    return 0;
  }

  luaL_checktype(L, 1, LUA_TTABLE);

  lua_getfield(L, 1, "width");
  flags.width = luaL_optinteger(L, -1, 1080);
  lua_pop(L, 1);

  lua_getfield(L, 1, "height");
  flags.height = luaL_optinteger(L, -1, 600);
  lua_pop(L, 1);

  lua_getfield(L, 1, "fullscreen");
  flags.fullscreen = lua_toboolean(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "resizable");
  flags.resizable = lua_toboolean(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "msaa");
  flags.msaa = lua_tointeger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "title");
  flags.title = luaL_optstring(L, -1, "LÖVR");
  lua_pop(L, 1);

  lua_getfield(L, 1, "icon");
  TextureData* textureData = NULL;
  if (!lua_isnil(L, -1)) {
    textureData = luax_checktexturedata(L, -1, false);
    flags.icon.data = textureData->blob->data;
    flags.icon.width = textureData->width;
    flags.icon.height = textureData->height;
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "vsync");
  flags.vsync = lua_tointeger(L, -1);
  lua_pop(L, 1);

  lovrGraphicsCreateWindow(&flags);
  luax_atexit(L, lovrGraphicsDestroy); // The lua_State that creates the window shall be the one to destroy it
  lovrRelease(TextureData, textureData);
  return 0;
}

static int l_lovrGraphicsGetFeatures(lua_State* L) {
  if (lua_istable(L, 1)) {
    lua_settop(L, 1);
  } else {
    lua_newtable(L);
  }

  GraphicsFeatures features;
  lovrGraphicsGetFeatures(&features);

  lua_pushboolean(L, features.bptc);
  lua_setfield(L, -2, "bptc");
  lua_pushboolean(L, features.astc);
  lua_setfield(L, -2, "astc");
  lua_pushboolean(L, features.pointSize);
  lua_setfield(L, -2, "pointSize");
  lua_pushboolean(L, features.wireframe);
  lua_setfield(L, -2, "wireframe");
  lua_pushboolean(L, features.anisotropy);
  lua_setfield(L, -2, "anisotropy");
  lua_pushboolean(L, features.clipDistance);
  lua_setfield(L, -2, "clipDistance");
  lua_pushboolean(L, features.cullDistance);
  lua_setfield(L, -2, "cullDistance");
  lua_pushboolean(L, features.fullIndexBufferRange);
  lua_setfield(L, -2, "fullIndexBufferRange");
  lua_pushboolean(L, features.indirectDrawCount);
  lua_setfield(L, -2, "indirectDrawCount");
  lua_pushboolean(L, features.indirectDrawFirstInstance);
  lua_setfield(L, -2, "indirectDrawFirstInstance");
  lua_pushboolean(L, features.extraShaderInputs);
  lua_setfield(L, -2, "extraShaderInputs");
  lua_pushboolean(L, features.multiview);
  lua_setfield(L, -2, "multiview");
  return 1;
}

static int l_lovrGraphicsGetLimits(lua_State* L) {
  if (lua_istable(L, 1)) {
    lua_settop(L, 1);
  } else {
    lua_newtable(L);
  }

  GraphicsLimits limits;
  lovrGraphicsGetLimits(&limits);

  lua_pushinteger(L, limits.textureSize2D);
  lua_setfield(L, -2, "textureSize2D");

  lua_pushinteger(L, limits.textureSize3D);
  lua_setfield(L, -2, "textureSize3D");

  lua_pushinteger(L, limits.textureSizeCube);
  lua_setfield(L, -2, "textureSizeCube");

  lua_pushinteger(L, limits.textureLayers);
  lua_setfield(L, -2, "textureLayers");

  lua_createtable(L, 2, 0);
  lua_pushinteger(L, limits.canvasSize[0]);
  lua_rawseti(L, -2, 1);
  lua_pushinteger(L, limits.canvasSize[1]);
  lua_rawseti(L, -2, 2);
  lua_setfield(L, -2, "canvasSize");

  lua_pushinteger(L, limits.canvasViews);
  lua_setfield(L, -2, "canvasViews");

  lua_pushinteger(L, limits.bundleCount);
  lua_setfield(L, -2, "bundleCount");

  lua_pushinteger(L, limits.bundleSlots);
  lua_setfield(L, -2, "bundleSlots");

  lua_pushinteger(L, limits.uniformBufferRange);
  lua_setfield(L, -2, "uniformBufferRange");

  lua_pushinteger(L, limits.storageBufferRange);
  lua_setfield(L, -2, "storageBufferRange");

  lua_pushinteger(L, limits.uniformBufferAlign);
  lua_setfield(L, -2, "uniformBufferAlign");

  lua_pushinteger(L, limits.storageBufferAlign);
  lua_setfield(L, -2, "storageBufferAlign");

  lua_pushinteger(L, limits.vertexAttributes);
  lua_setfield(L, -2, "vertexAttributes");

  lua_pushinteger(L, limits.vertexAttributeOffset);
  lua_setfield(L, -2, "vertexAttributeOffset");

  lua_pushinteger(L, limits.vertexBuffers);
  lua_setfield(L, -2, "vertexBuffers");

  lua_pushinteger(L, limits.vertexBufferStride);
  lua_setfield(L, -2, "vertexBufferStride");

  lua_pushinteger(L, limits.vertexShaderOutputs);
  lua_setfield(L, -2, "vertexShaderOutputs");

  lua_createtable(L, 3, 0);
  lua_pushinteger(L, limits.computeCount[0]);
  lua_rawseti(L, -2, 1);
  lua_pushinteger(L, limits.computeCount[1]);
  lua_rawseti(L, -2, 2);
  lua_pushinteger(L, limits.computeCount[2]);
  lua_rawseti(L, -2, 3);
  lua_setfield(L, -2, "computeCount");

  lua_createtable(L, 3, 0);
  lua_pushinteger(L, limits.computeGroupSize[0]);
  lua_rawseti(L, -2, 1);
  lua_pushinteger(L, limits.computeGroupSize[1]);
  lua_rawseti(L, -2, 2);
  lua_pushinteger(L, limits.computeGroupSize[2]);
  lua_rawseti(L, -2, 3);
  lua_setfield(L, -2, "computeGroupSize");

  lua_pushinteger(L, limits.computeGroupVolume);
  lua_setfield(L, -2, "computeGroupVolume");

  lua_pushinteger(L, limits.computeSharedMemory);
  lua_setfield(L, -2, "computeSharedMemory");

  lua_pushinteger(L, limits.indirectDrawCount);
  lua_setfield(L, -2, "indirectDrawCount");

  lua_pushinteger(L, limits.allocationSize);
  lua_setfield(L, -2, "allocationSize");

  lua_pushinteger(L, limits.pointSize[0]);
  lua_setfield(L, -2, "allocationSize");

  lua_createtable(L, 2, 0);
  lua_pushinteger(L, limits.pointSize[0]);
  lua_rawseti(L, -2, 1);
  lua_pushinteger(L, limits.pointSize[1]);
  lua_rawseti(L, -2, 2);
  lua_setfield(L, -2, "pointSize");

  lua_pushnumber(L, limits.anisotropy);
  lua_setfield(L, -2, "anisotropy");
  return 1;
}

static const luaL_Reg lovrGraphics[] = {
  { "createWindow", l_lovrGraphicsCreateWindow },
  { "getFeatures", l_lovrGraphicsGetFeatures },
  { "getLimits", l_lovrGraphicsGetLimits },
  { NULL, NULL }
};

int luaopen_lovr_graphics(lua_State* L) {
  lua_newtable(L);
  luax_register(L, lovrGraphics);

  bool debug = false;
  luax_pushconf(L);
  lua_getfield(L, -1, "graphics");
  if (lua_istable(L, -1)) {
    lua_getfield(L, -1, "debug");
    debug = lua_toboolean(L, -1);
    lua_pop(L, 1);
  }
  lua_pop(L, 1);

  lovrGraphicsInit(debug);

  lua_pushcfunction(L, l_lovrGraphicsCreateWindow);
  lua_getfield(L, -2, "window");
  lua_call(L, 1, 0);
  lua_pop(L, 1);
  return 1;
}

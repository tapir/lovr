#include "api.h"
#include "graphics/graphics.h"
#include "data/textureData.h"
#include "core/os.h"
#include "core/ref.h"
#include <stdlib.h>

StringEntry lovrGraphicsLimit[] = {
  [LIMIT_TEXTURE_SIZE_2D] = ENTRY("texturesize2d"),
  [LIMIT_TEXTURE_SIZE_3D] = ENTRY("texturesize3d"),
  [LIMIT_TEXTURE_SIZE_CUBE] = ENTRY("texturesizecube"),
  [LIMIT_TEXTURE_LAYERS] = ENTRY("texturelayers"),
  [LIMIT_RENDER_WIDTH] = ENTRY("renderwidth"),
  [LIMIT_RENDER_HEIGHT] = ENTRY("renderheight"),
  [LIMIT_RENDER_VIEWS] = ENTRY("renderviews"),
  [LIMIT_SHADER_GROUPS] = ENTRY("shadergroups"),
  [LIMIT_SHADER_GROUP_ITEMS] = ENTRY("shadergroupitems"),
  [LIMIT_INPUT_BUFFER_RANGE] = ENTRY("inputbufferrange"),
  [LIMIT_INPUT_BUFFER_ALIGN] = ENTRY("inputbufferalign"),
  [LIMIT_COMPUTE_BUFFER_RANGE] = ENTRY("computebufferrange"),
  [LIMIT_COMPUTE_BUFFER_ALIGN] = ENTRY("computebufferalign"),
  [LIMIT_VERTEX_ATTRIBUTES] = ENTRY("vertexattributes"),
  [LIMIT_VERTEX_ATTRIBUTE_OFFSET] = ENTRY("vertexattributeoffset"),
  [LIMIT_VERTEX_BUFFERS] = ENTRY("vertexbuffers"),
  [LIMIT_VERTEX_BUFFER_STRIDE] = ENTRY("vertexbufferstride"),
  [LIMIT_VERTEX_SHADER_OUTPUTS] = ENTRY("vertexshaderoutputs"),
  [LIMIT_COMPUTE_WIDTH] = ENTRY("computewidth"),
  [LIMIT_COMPUTE_HEIGHT] = ENTRY("computeheight"),
  [LIMIT_COMPUTE_DEPTH] = ENTRY("computedepth"),
  [LIMIT_COMPUTE_GROUP_WIDTH] = ENTRY("computegroupwidth"),
  [LIMIT_COMPUTE_GROUP_HEIGHT] = ENTRY("computegroupheight"),
  [LIMIT_COMPUTE_GROUP_DEPTH] = ENTRY("computegroupdepth"),
  [LIMIT_COMPUTE_GROUP_VOLUME] = ENTRY("computegroupvolume"),
  [LIMIT_COMPUTE_SHARED_MEMORY] = ENTRY("computesharedmemory"),
  [LIMIT_ALLOCATION_SIZE] = ENTRY("allocationsize"),
  [LIMIT_POINT_SIZE] = ENTRY("pointsize"),
  [LIMIT_ANISOTROPY] = ENTRY("anisotropy"),
  { 0 }
};

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

static int l_lovrGraphicsGetLimits(lua_State* L) {
  if (lua_istable(L, 1)) {
    lua_settop(L, 1);
  } else {
    lua_createtable(L, 0, MAX_LIMITS);
  }

  double limits[MAX_LIMITS];
  lovrGraphicsGetLimits(limits);
  for (uint32_t i = 0; i < MAX_LIMITS; i++) {
    lua_pushnumber(L, limits[i]);
    lua_setfield(L, -2, lovrGraphicsLimit[i].string);
  }

  return 1;
}

static const luaL_Reg lovrGraphics[] = {
  { "createWindow", l_lovrGraphicsCreateWindow },
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

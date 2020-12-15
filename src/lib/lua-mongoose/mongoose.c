#include <stdlib.h>
#include <string.h>

// For lua5.2 support, instead we could replace all the luaL_register's with whatever
// lua5.2's equivalent function is, but this is easier so whatever.
#define LUA_COMPAT_MODULE
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <mongoose.h>

int luaopen_mongoose(lua_State *l) {
	return 1;
}

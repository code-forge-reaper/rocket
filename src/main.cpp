#include "lua_ffi.hpp"
#include "funcs.cpp"
#include "../libs/raylib/raylib.cpp"
#include <dlfcn.h>
#include <lua.h>
#include <lua.hpp>
#include <stdio.h>
#include <unistd.h>

void rocketFunctions(lua_State *L) {
  luaL_openlibs(L);
  luaopen_raylib(L);
  initFuncs(L);
}

// Function to push all elements from argv onto a Lua table
void pushArgvToLuaTable(lua_State *L, int argc, const char *argv[]) {
  lua_newtable(L); // Create a new table on the Lua stack

  for (int i = 2; i < argc; ++i) {
    lua_pushnumber(L,
                   i - 1); // Push the index (adjusted for Lua 1-based indexing)
    lua_pushstring(L, argv[i]); // Push the corresponding argument
    lua_settable(L, -3);        // Set the table at the given index
  }

  lua_pushnumber(L, 0);       // Push the index for the script name
  lua_pushstring(L, argv[1]); // Push the script name
  lua_settable(L, -3);        // Set the table at index 0
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <lua file>\n", argv[0]);
    return 1;
  }

  lua_State *L = luaL_newstate();
  rocketFunctions(L);
  pushArgvToLuaTable(L, argc, argv);
  lua_setglobal(L, "arg"); // Set the table as a global variable named "args"

  int res = luaL_dofile(L, argv[1]); // Load Lua script
  if (res != LUA_OK) {
    printf("Error loading Lua script: %s\n", lua_tostring(L, -1));
    lua_close(L);
    return 1;
  }

  lua_getglobal(L, "main");
  res = lua_pcall(L, 0, 0, 0);
  if (res != LUA_OK) {
    printf("%s\n", lua_tostring(L, -1));
    lua_close(L);
    return 1;
  }

  lua_close(L);
  return 0;
}


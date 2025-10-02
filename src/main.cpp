#include "lua_ffi.hpp"
#include "funcs.cpp"
#include "../libs/raylib/raylib.cpp"
#include <dlfcn.h>
#include <lua.h>
#include <lua.hpp>
#include <stdio.h>
#include <unistd.h>

static int loadLib(lua_State *L) {
  if (lua_gettop(L) != 1) {
    luaL_error(L, "Expected 1 argument, got %d", lua_gettop(L));
    return 1;
  }
  const char *lib = luaL_checkstring(L, 1);
  // check if it exists
  if (access(lib, F_OK) != -1) {
    void *handle = dlopen(lib, RTLD_LAZY);
    if (!handle) {
      std::cerr << "Error loading shared object file: " << dlerror()
                << std::endl;
      exit(1); // exit on failure
    }
    lua_func library_open_function = (lua_func)dlsym(handle, "rocket_init");

    if (!library_open_function) {
      std::cerr << "Error finding library open function: " << dlerror()
                << std::endl;
      std::cerr << "library:" << lib << std::endl;

      dlclose(handle); // Close the handle
      exit(1);         // exit on failure
    }

    int r = library_open_function(L);
    return r;
  }
  luaL_error(L, "Could not load library: %s", lib);
  return 1;
}

void rocketFunctions(lua_State *L) {
  luaL_openlibs(L);

  lua_newtable(L);
  lua_pushcfunction(L, loadLib);
  lua_setfield(L, -2, "load");
  lua_setglobal(L, "ffi");

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
  lua_pushboolean(L, true);
  lua_setglobal(L, "isRocket");
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


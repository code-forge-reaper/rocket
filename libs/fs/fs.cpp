#include <lua.hpp>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <filesystem>

static int writeFile(lua_State* L) {
    const char* filename = lua_tostring(L, 1);
    const char* content = lua_tostring(L, 2);

    FILE* file = fopen(filename, "w");
    if (!file) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "Cannot open file '%s' for writing", filename);
        return 2;
    }

    size_t len = strlen(content);
    size_t written = fwrite(content, sizeof(char), len, file);
    fclose(file);

    if (written == len) {
        lua_pushboolean(L, 1);
        return 1;
    } else {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Error writing to file");
        return 2;
    }
}
static int readFile(lua_State* L) {
    const char* filename = lua_tostring(L, 1);

    std::ifstream file(filename);
    if (!file.is_open()) {
        lua_pushnil(L);
        lua_pushfstring(L, "Cannot open file '%s'", filename);
        return 2;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string contents = buffer.str();

    lua_pushstring(L, contents.c_str());
    return 1;
}

static int readDir(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    lua_newtable(L);
    int i = 1;
    for(const auto & entry : std::filesystem::directory_iterator(path)) {
        // push only the content from path, since the user probably will concat the root anyways
        lua_pushstring(L, entry.path().filename().c_str());
        lua_rawseti(L, -2, i++);
    }
    return 1;
}
static int removeFile(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    int result = std::remove(filename);
    lua_pushboolean(L, result == 0);
    return 1;
}

static int fileExists(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    lua_pushboolean(L, std::filesystem::exists(filename));
    return 1;
}

extern "C" int luaopen_fs(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, readFile);
    lua_setfield(L, -2, "readFile");
    
    lua_pushcfunction(L, writeFile);
    lua_setfield(L, -2, "writeFile");

    lua_pushcfunction(L, readDir);
    lua_setfield(L, -2, "readDir");

    lua_pushcfunction(L, removeFile);
    lua_setfield(L, -2, "removeFile");

    lua_pushcfunction(L, fileExists);
    lua_setfield(L, -2, "fileExists");

    return 1;
}

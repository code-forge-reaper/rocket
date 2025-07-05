#include <lua.hpp>
#include <raylib.h>
#include <vector>
#include "../../../libs/lua_ffi.hpp" // needs: pushPtr, getPtr
#include "ray-color.cpp" // needs: Color lua_getColor(lua_State*, int)
#include <algorithm>     // std::remove

struct Img {
    Image image;
    Texture2D texture;
};

// Manual image pool tracking
static std::vector<Img*> imgPool;
static int l_LoadAndResize(lua_State* L){
    const char* path = luaL_checkstring(L, 1);
    int width = luaL_checkinteger(L, 2);
    int height = luaL_checkinteger(L, 3);

    Image img = LoadImage(path);
    if (!img.data) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to load image");
        return 2;
    }
    ImageResize(&img, width, height);

    Texture2D tex = LoadTextureFromImage(img);
    if (!tex.id) {
        UnloadImage(img);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to create texture");
        return 2;
    }

    Img* wrapper = new Img{ img, tex };
    imgPool.push_back(wrapper); // Track for cleanup

    pushPtr(L, wrapper); // Pushed as userdata, no __gc
    return 1;
}

static int l_LoadAndScale(lua_State* L){
    const char* path = luaL_checkstring(L, 1);
    float scale = luaL_checknumber(L, 2);

    Image img = LoadImage(path);
    if (!img.data) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to load image");
        return 2;
    }

    ImageResize(&img, (int)(img.width * scale), (int)(img.height * scale));

    Texture2D tex = LoadTextureFromImage(img);
    if (!tex.id) {
        UnloadImage(img);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to create texture");
        return 2;
    }

    Img* wrapper = new Img{ img, tex };
    imgPool.push_back(wrapper); // Track for cleanup

    pushPtr(L, wrapper); // Pushed as userdata, no __gc
    return 1;
}

static int l_LoadImage(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);

    Image img = LoadImage(path);
    if (!img.data) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to load image");
        return 2;
    }

    Texture2D tex = LoadTextureFromImage(img);
    if (!tex.id) {
        UnloadImage(img);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to create texture");
        return 2;
    }

    Img* wrapper = new Img{ img, tex };
    imgPool.push_back(wrapper); // Track for cleanup

    pushPtr(L, wrapper); // Pushed as userdata, no __gc
    return 1;
}

static int l_Draw(lua_State* L) {
    Img* img = getPtr<Img>(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    Color c = lua_getColor(L, 4);
    DrawTexture(img->texture, x, y, c);
    return 0;
}

static int l_GetSize(lua_State* L) {
    Img* img = getPtr<Img>(L, 1);
    lua_newtable(L);
    lua_pushinteger(L, img->texture.width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, img->texture.height);
    lua_setfield(L, -2, "height");
    return 1;
}

// Unload one specific image
static int l_UnloadImage(lua_State* L) {
    Img* img = getPtr<Img>(L, 1);
    if (!img) return 0;

    if (img->texture.id) UnloadTexture(img->texture);
    if (img->image.data) UnloadImage(img->image);

    // Remove from pool
    imgPool.erase(std::remove(imgPool.begin(), imgPool.end(), img), imgPool.end());

    delete img;
    return 0;
}

// Unload all tracked images manually
static int l_UnloadAll(lua_State* L) {
    for (Img* img : imgPool) {
        if (img->texture.id) UnloadTexture(img->texture);
        if (img->image.data) UnloadImage(img->image);
        delete img;
    }
    imgPool.clear();
    return 0;
}

// Register Img methods (no __gc)
static void registerImageClass(lua_State* L) {
    const char* type = typeid(Img).name();
    if (luaL_newmetatable(L, type)) {
        lua_pushstring(L, "__index");
        lua_newtable(L);

        lua_pushcfunction(L, l_Draw);
        lua_setfield(L, -2, "draw");

        lua_pushcfunction(L, l_GetSize);
        lua_setfield(L, -2, "getSize");

        lua_pushcfunction(L, l_UnloadImage);
        lua_setfield(L, -2, "unload");

        lua_settable(L, -3); // metatable.__index = table
    }
    lua_pop(L, 1);
}

// Lua module setup
static luaL_Reg imgFuncs[] = {
    { "load", l_LoadImage },
    { "unloadAll", l_UnloadAll },
    { "loadAndResize", l_LoadAndResize },
    { "loadAndScale", l_LoadAndScale },
    { NULL, NULL }
};

extern "C" void init_raylib_img(lua_State* L) {
    registerImageClass(L);
    newModule("Image", imgFuncs, L);
}

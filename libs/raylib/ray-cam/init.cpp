#include <lua.hpp>
#include <raylib.h>
#include "lua_ffi.hpp" // getArgByName

#define LUA lua_State* ctx

Camera2D globalCam2d;
Camera3D globalCam3d;

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

#define GET_VEC2(var, idx) \
    float var##X = getArgByName(ctx, "x", idx); \
    float var##Y = getArgByName(ctx, "y", idx); \
    Vector2 var = { var##X, var##Y }

#define GET_VEC3(var, idx) \
    float var##X = getArgByName(ctx, "x", idx); \
    float var##Y = getArgByName(ctx, "y", idx); \
    float var##Z = getArgByName(ctx, "z", idx); \
    Vector3 var = { var##X, var##Y, var##Z }

#define RETURN_VEC2(vec) \
    lua_newtable(ctx); \
    lua_pushnumber(ctx, vec.x); lua_setfield(ctx, -2, "x"); \
    lua_pushnumber(ctx, vec.y); lua_setfield(ctx, -2, "y")

#define RETURN_VEC3(vec) \
    lua_newtable(ctx); \
    lua_pushnumber(ctx, vec.x); lua_setfield(ctx, -2, "x"); \
    lua_pushnumber(ctx, vec.y); lua_setfield(ctx, -2, "y"); \
    lua_pushnumber(ctx, vec.z); lua_setfield(ctx, -2, "z")

// ─────────────────────────────────────────────────────────────────────────────
// Camera2D
// ─────────────────────────────────────────────────────────────────────────────

int l_SetupCamera2D(LUA) {
    GET_VEC2(offset, 1);
    GET_VEC2(target, 2);
    float rotation = lua_tonumber(ctx, 3);
    float zoom     = lua_tonumber(ctx, 4);
    globalCam2d = { offset, target, rotation, zoom };
    return 0;
}

int l_UseCamera2D(LUA) {
    BeginMode2D(globalCam2d);
    return 0;
}

int l_StopCamera2D(LUA) {
    EndMode2D();
    return 0;
}

int l_WorldToScreen2D(LUA) {
    GET_VEC2(pos, 1);
    Vector2 result = GetWorldToScreen2D(pos, globalCam2d);
    RETURN_VEC2(result);
    return 1;
}

int l_ScreenToWorld2D(LUA) {
    GET_VEC2(pos, 1);
    Vector2 result = GetScreenToWorld2D(pos, globalCam2d);
    RETURN_VEC2(result);
    return 1;
}

int l_SetCamera2DField(LUA) {
    const char* field = luaL_checkstring(ctx, 1);
    GET_VEC2(val, 2);

    if (strcmp(field, "target") == 0) globalCam2d.target = val;
    else if (strcmp(field, "offset") == 0) globalCam2d.offset = val;

    return 0;
}

int l_SetCamera2DNumber(LUA) {
    const char* field = luaL_checkstring(ctx, 1);
    float value = luaL_checknumber(ctx, 2);

    if (strcmp(field, "rotation") == 0) globalCam2d.rotation = value;
    else if (strcmp(field, "zoom") == 0) globalCam2d.zoom = value;

    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Camera3D
// ─────────────────────────────────────────────────────────────────────────────

int l_SetupCamera3D(LUA) {
    GET_VEC3(position, 1);
    GET_VEC3(target, 2);
    GET_VEC3(up, 3);
    float fovy = luaL_checknumber(ctx, 4);
    int projection = luaL_checkinteger(ctx, 5);
    globalCam3d = { position, target, up, fovy, projection };
    return 0;
}

int l_UseCamera3D(LUA) {
    BeginMode3D(globalCam3d);
    return 0;
}

int l_StopCamera3D(LUA) {
    EndMode3D();
    return 0;
}

int l_SetCamera3DField(LUA) {
    const char* field = luaL_checkstring(ctx, 1);
    GET_VEC3(val, 2);

    if (strcmp(field, "target") == 0) globalCam3d.target = val;
    else if (strcmp(field, "position") == 0) globalCam3d.position = val;
    else if (strcmp(field, "up") == 0) globalCam3d.up = val;

    return 0;
}

int l_SetCamera3DNumber(LUA) {
    const char* field = luaL_checkstring(ctx, 1);
    float value = luaL_checknumber(ctx, 2);

    if (strcmp(field, "fovy") == 0) globalCam3d.fovy = value;
    else if (strcmp(field, "projection") == 0) globalCam3d.projection = static_cast<int>(value);

    return 0;
}

int l_UpdateCamera3D(LUA) {
    int mode = luaL_checkinteger(ctx, 1);
    UpdateCamera(&globalCam3d, mode);
    return 0;
}

void pushCamera2D(LUA) {
    lua_newtable(ctx);
    lua_pushcfunction(ctx, l_SetupCamera2D);          lua_setfield(ctx, -2, "setup");
    lua_pushcfunction(ctx, l_UseCamera2D);            lua_setfield(ctx, -2, "begin");
    lua_pushcfunction(ctx, l_StopCamera2D);           lua_setfield(ctx, -2, "stop");
    lua_pushcfunction(ctx, l_WorldToScreen2D);        lua_setfield(ctx, -2, "worldToScreen");
    lua_pushcfunction(ctx, l_ScreenToWorld2D);        lua_setfield(ctx, -2, "screenToWorld");
    lua_pushcfunction(ctx, l_SetCamera2DField);       lua_setfield(ctx, -2, "setVec2");
    lua_pushcfunction(ctx, l_SetCamera2DNumber);      lua_setfield(ctx, -2, "setNumber");
    lua_setglobal(ctx, "Camera2D");
}

void pushCamera3D(LUA) {
    lua_newtable(ctx);
    lua_pushcfunction(ctx, l_SetupCamera3D);          lua_setfield(ctx, -2, "setup");
    lua_pushcfunction(ctx, l_UseCamera3D);            lua_setfield(ctx, -2, "begin");
    lua_pushcfunction(ctx, l_StopCamera3D);           lua_setfield(ctx, -2, "stop");
    lua_pushcfunction(ctx, l_SetCamera3DField);       lua_setfield(ctx, -2, "setVec3");
    lua_pushcfunction(ctx, l_SetCamera3DNumber);      lua_setfield(ctx, -2, "setNumber");
    lua_pushcfunction(ctx, l_UpdateCamera3D);         lua_setfield(ctx, -2, "update");

    // Add camera mode constants
    lua_pushinteger(ctx, CAMERA_PERSPECTIVE);         lua_setfield(ctx, -2, "CAMERA_PERSPECTIVE");
    lua_pushinteger(ctx, CAMERA_ORTHOGRAPHIC);        lua_setfield(ctx, -2, "CAMERA_ORTHOGRAPHIC");
    lua_pushinteger(ctx, CAMERA_FREE);                lua_setfield(ctx, -2, "CAMERA_FREE");
    lua_pushinteger(ctx, CAMERA_ORBITAL);             lua_setfield(ctx, -2, "CAMERA_ORBITAL");
    lua_pushinteger(ctx, CAMERA_FIRST_PERSON);        lua_setfield(ctx, -2, "CAMERA_FIRST_PERSON");
    lua_pushinteger(ctx, CAMERA_THIRD_PERSON);        lua_setfield(ctx, -2, "CAMERA_THIRD_PERSON");

    lua_setglobal(ctx, "Camera3D");
}

void initRaylibCamera(LUA) {
    pushCamera2D(ctx);
    pushCamera3D(ctx);
}

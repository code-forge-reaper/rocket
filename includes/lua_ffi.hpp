#pragma once
#include <lua.hpp>
#include <typeinfo>
#include <string>
#include <iostream>
#include <type_traits>
#include <stdexcept>

// Generic function pointer for Lua C functions
typedef int (*lua_func)(lua_State *);

#define LUA_DEF_I(name, value, L) \
    lua_pushnumber(L, value); \
    lua_setfield(L, -2, name)
#define LUA_DEF_IG(name, value, L) \
    lua_pushnumber(L, value); \
    lua_setglobal(L, name)

#define LUA_DEF_I_M(name, value, target, L) \
    lua_getglobal(L, target); \
    lua_pushnumber(L, value); \
    lua_setfield(L, -2, name)

// ----------------- GC wrapper for template types -----------------
template<typename T>
static int lua_gc_wrapper(lua_State* L){
    void* ud = lua_touserdata(L, 1);
    if (!ud) return 0;
    T* p = *reinterpret_cast<T**>(ud);
    if (p) delete p;
    return 0;
}

// ----------------- register a metatable and methods once --------------
inline void register_metatable(lua_State* L, const char* metaname, const luaL_Reg funcs[]){
    // Check if metatable already exists on registry
    luaL_getmetatable(L, metaname); // pushes metatable or nil
    if (!lua_isnil(L, -1)) {
        // ensure __index is set to the metatable itself
        lua_pushvalue(L, -1);             // push metatable
        lua_setfield(L, -2, "__index");   // metatable.__index = metatable
        lua_pop(L, 1); // pop metatable
        return;
    }
    // got nil -> pop it and create new metatable
    lua_pop(L, 1);
    luaL_newmetatable(L, metaname);      // push new metatable

    // set functions on the metatable
#if LUA_VERSION_NUM >= 502
    luaL_setfuncs(L, funcs, 0);
#else
    for (int i = 0; funcs[i].name != nullptr; ++i) {
        lua_pushcfunction(L, funcs[i].func);
        lua_setfield(L, -2, funcs[i].name);
    }
#endif

    // metatable.__index = metatable
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1); // pop metatable
}

// ---------- push userdata (pointer) and attach existing metatable ----------
template<typename T>
void pushPtrWithMeta(lua_State* L, T* ptr, const char* metaname, bool gc = true){
    if(!ptr){
        lua_pushnil(L);
        return;
    }

    // create userdata holding pointer (we store pointer value)
    void** ud = reinterpret_cast<void**>(lua_newuserdata(L, sizeof(void*)));
    *ud = ptr;

    // ensure metatable exists (create minimal one if needed) so we can attach GC
    luaL_getmetatable(L, metaname);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaL_newmetatable(L, metaname); // pushes new metatable
        // make sure __index points to itself
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
    }
    // stack: userdata, metatable

    if (gc) {
        // attach GC function (overwrite if already present)
        lua_pushcfunction(L, &lua_gc_wrapper<T>);
        lua_setfield(L, -2, "__gc");
    }

    // set the metatable on userdata (this pops metatable)
    lua_setmetatable(L, -2); // userdata now has metatable on top of stack
}

// ---------- convenience macro wrappers ----------
#define ATTACH_TYPE(L, NAME, FUNCS) register_metatable((L), (NAME), (FUNCS))
#define PUSH_ATTACHED(L, T, PTR, NAME, GC) pushPtrWithMeta<T>((L), (PTR), (NAME), (GC))

// ---------------- Pointer FFI: push/get pointers with optional GC ----------------

template<typename T>
T* getPtr(lua_State* L, int index) {
    if (!lua_isuserdata(L, index)) {
        luaL_error(L, "Expected userdata at index %d", index);
        return nullptr; // unreachable because luaL_error longjumps, but keeps compiler happy
    }
    void* userdata = lua_touserdata(L, index);
    return static_cast<T*>(*reinterpret_cast<void**>(userdata));
}

// safer overload which checks metatable name (preferred when you have a stable metatable name)
template<typename T>
T* getPtr(lua_State* L, int index, const char* metaname) {
    void** ud = reinterpret_cast<void**>(luaL_checkudata(L, index, metaname));
    if (!ud) {
        luaL_error(L, "Invalid userdata or wrong metatable at index %d (expected %s)", index, metaname);
        return nullptr;
    }
    return static_cast<T*>(*ud);
}

// non-throwing variant: returns nullptr instead of luaL_error
template<typename T>
T* getRawPtr(lua_State* L, int index) {
    if (!lua_isuserdata(L, index)) return nullptr;
    void* userdata = lua_touserdata(L, index);
    if (!userdata) return nullptr;
    return static_cast<T*>(*reinterpret_cast<void**>(userdata));
}

// pushPtr that uses typeid(T).name() as metatable, creating __gc if requested
template<typename T>
void pushPtr(lua_State* L, T* ptr, bool gc = true) {
    if (!ptr) {
        lua_pushnil(L);
        return;
    }

    void** userdata = reinterpret_cast<void**>(lua_newuserdata(L, sizeof(void*)));
    *userdata = ptr;

    const char* metatype = typeid(T).name();
    // luaL_newmetatable pushes the metatable on the stack and returns 1 if it was created
    if (luaL_newmetatable(L, metatype)) {
        // new metatable created - set __index to itself
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        if (gc) {
            lua_pushcfunction(L, &lua_gc_wrapper<T>);
            lua_setfield(L, -2, "__gc");
        }
    } else {
        // metatable already existed; if user wants GC ensure __gc is present/overwritten
        if (gc) {
            lua_pushcfunction(L, &lua_gc_wrapper<T>);
            lua_setfield(L, -2, "__gc");
        }
    }
    // set metatable on userdata and pop metatable
    lua_setmetatable(L, -2);
}

// ---------------- Table argument parsing helpers ----------------

inline double getArgByName(lua_State* L, const char* key, int index) {
    if (!lua_istable(L, index)) {
        luaL_error(L, "Expected table at index %d", index);
        return 0;
    }
    lua_getfield(L, index, key);
    if (!lua_isnumber(L, -1)) {
        lua_pop(L, 1);
        luaL_error(L, "Expected number for field '%s'", key);
        return 0;
    }
    double value = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return value;
}

inline double getGlobalNumber(lua_State* L, const char* key) {
    lua_getglobal(L, key);
    if (!lua_isnumber(L, -1)) {
        lua_pop(L, 1);
        luaL_error(L, "Expected number global '%s'", key);
        return 0;
    }
    double value = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return value;
}

// ---------------- Lua module helpers ----------------

inline void push_funcs(lua_State* L, luaL_Reg funcs[]) {
    for (int i = 0; funcs[i].name != nullptr; i++) {
        lua_pushcfunction(L, funcs[i].func);
        lua_setfield(L, -2, funcs[i].name);
    }
}

// ---------- Small helpers for class "table" creation ----------
inline void create_class_table(lua_State* L, const char* luaName, const luaL_Reg class_funcs[]) {
#if LUA_VERSION_NUM >= 502
    lua_createtable(L, 0, 0);            // push table
    luaL_setfuncs(L, class_funcs, 0);   // set fields
#else
    lua_newtable(L);
    for (int i = 0; class_funcs[i].name != nullptr; ++i) {
        lua_pushcfunction(L, class_funcs[i].func);
        lua_setfield(L, -2, class_funcs[i].name);
    }
#endif
    lua_setglobal(L, luaName); // global LuaName = the table
}

// ---------- Primary macro (keeps your original convenience) ----------
#define LUA_CLASS(Type, LuaName, METHOD_INITS...)                                        \
    static const luaL_Reg Type##_methods[] = { METHOD_INITS, {NULL, NULL} };             \
    static const luaL_Reg Type##_class[]   = { {"new", Type##_new}, {NULL, NULL} };      \
    inline void register_##Type(lua_State* L) {                                          \
        ATTACH_TYPE(L, LuaName, (luaL_Reg*)Type##_methods);                              \
        create_class_table(L, LuaName, (luaL_Reg*)Type##_class);                         \
    }

#define LUA_CLASS_AUTO(Type, METHOD_INITS...) \
    LUA_CLASS(Type, #Type, METHOD_INITS)

inline void newModule(const char* name, luaL_Reg funcs[], lua_State* L) {
    lua_newtable(L);
    push_funcs(L, funcs);
    lua_setglobal(L, name);
}

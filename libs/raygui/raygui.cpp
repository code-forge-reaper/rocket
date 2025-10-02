// raygui.cpp
// this file is included by raylib.cpp
// so you can use rgui functions without having to deal with
// having to somehow pass the window to it
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <lua.hpp>
#include "lua_ffi.hpp" // getArgByName

static int lua_rgui_button(lua_State *L){
	//GuiButton(Rectangle bounds, const char *text)
	Rectangle rect = {
		(float)getArgByName(L, "x",1),
		(float)getArgByName(L, "y",1),
		(float)getArgByName(L, "width",1),
		(float)getArgByName(L, "height",1)
	};
	
	const char* text = luaL_checkstring(L, 2);
	lua_pushboolean(L, GuiButton(rect, text));
	return 1;
}


static int lua_rgui_label(lua_State *L){
	//GuiLabel(Rectangle bounds, const char *text)
	Rectangle rect = {
		(float)getArgByName(L, "x", 1),
		(float)getArgByName(L, "y", 1),
		(float)getArgByName(L, "width", 1),
		(float)getArgByName(L, "height", 1)
	};
	
	const char* text = luaL_checkstring(L, 2);
	GuiLabel(rect, text);
	return 0;
}

static int lua_rgui_checkbox(lua_State *L){
	Rectangle rect = {
		(float)getArgByName(L, "x", 1),
		(float)getArgByName(L, "y", 1),
		(float)getArgByName(L, "width", 1),
		(float)getArgByName(L, "height", 1)
	};

	const char* text = luaL_checkstring(L, 2);
	bool checked = lua_toboolean(L, 3); // third arg = current state
	bool result = GuiCheckBox(rect, text, &checked);

	lua_pushboolean(L, checked);
	return 1;
}
static int lua_rgui_slider(lua_State *L){
    Rectangle bounds = {
        (float)getArgByName(L, "x", 1),
        (float)getArgByName(L, "y", 1),
        (float)getArgByName(L, "width", 1),
        (float)getArgByName(L, "height", 1)
    };

    const char* textLeft = luaL_checkstring(L, 2);
    const char* textRight = luaL_checkstring(L, 3);
    float value = (float)luaL_checknumber(L, 4);
    float minValue = (float)luaL_checknumber(L, 5);
    float maxValue = (float)luaL_checknumber(L, 6);

    // GuiSlider modifies the value in-place
    GuiSlider(bounds, textLeft, textRight, &value, minValue, maxValue);

    // Return updated value to Lua
    lua_pushnumber(L, value);
    return 1;
}

static int lua_rgui_textbox(lua_State *L){
	Rectangle bounds = {
		(float)getArgByName(L, "x", 1),
		(float)getArgByName(L, "y", 1),
		(float)getArgByName(L, "width", 1),
		(float)getArgByName(L, "height", 1)
	};

	const char* input = luaL_checkstring(L, 2);
	int maxSize = luaL_checkinteger(L, 3);
	bool editMode = lua_toboolean(L, 4);

	// Allocate a writable buffer and copy the string
	char* buffer = new char[maxSize];
	strncpy(buffer, input, maxSize);
	buffer[maxSize - 1] = '\0'; // Ensure null termination

	// Call raygui
	GuiTextBox(bounds, buffer, maxSize, editMode);

	// Push the result string back to Lua
	lua_pushstring(L, buffer);

	// Clean up
	delete[] buffer;
	return 1;
}


static luaL_Reg rayguiFunctions[] = {
	{"button", lua_rgui_button},
	{"label", lua_rgui_label},
	{"checkbox", lua_rgui_checkbox},
	{"slider", lua_rgui_slider},
	{"textbox", lua_rgui_textbox},
	{NULL, NULL}
};


void init_raygui(lua_State *L){
	lua_newtable(L);
    // Iterate over the functions and add them to the table
	for (int i = 0; rayguiFunctions[i].name; i++) {
		lua_pushcfunction(L, rayguiFunctions[i].func);
		lua_setfield(L, -2, rayguiFunctions[i].name);
	}

	lua_setglobal(L, "rgui");
}

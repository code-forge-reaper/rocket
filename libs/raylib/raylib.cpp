#include <lua.h>
#include <raylib.h>
#include <lua.hpp>
#include "../../../libs/lua_ffi.hpp"
#include "ray-init.cpp"
#include "ray-img.cpp"
#include "ray-sound.cpp"
#include "ray-cam/init.cpp"

#include <iostream>


static luaL_Reg funcs[] = {
	// Add functions to the table using addRaylibFunction
	{ "InitWindow", lua_create_window },
	{ "DrawText", lua_draw_text },
	{ "NewColor", lua_create_color },
	{ "SetWindowTitle", change_title},
	{ "CloseWindow", lua_close_window },
	{ "DrawCircle",lua_draw_circle},
	{ "DrawRectangle", lua_draw_rectangle },
	{"DrawRectangleRec", lua_draw_rectangle_rect},
	{ "ClearBackground", lua_fill_bg },
	{ "BeginDrawing", lua_start_drawing },
	{ "EndDrawing", lua_stop_drawing },
	{ "IsKeyDown", lua_is_key_down },
	{ "IsKeyPressed", lua_is_key_pressed },
	{"MeasureText", lua_measure_text},
	{ "SetTargetFPS", lua_set_target_fps },
	{ "WindowShouldClose", lua_should_close_window },
	{ "DrawFPS", lua_draw_fps },
	{ "GetMousePosition", lua_get_mouse_position },
	{ "GetMouseX", getMouseX },
	{ "GetMouseY", getMouseY },
	{ "GetScreenWidth", lua_get_screen_width },
	{ "GetScreenHeight", lua_get_screen_height },
	{ "GetFrameTime", lua_get_frame_time },
	{ "GetTime", lua_get_time },
	{ "GetFPS", lua_get_fps },
	{ "IsMouseButtonDown", lua_is_mouse_button_down },
	{ "IsMouseButtonPressed", lua_is_mouse_button_pressed },
	{ "IsMouseButtonReleased", lua_is_mouse_button_released },
	{ NULL,NULL}
};
#include "../raygui/raygui.cpp"

extern "C" int luaopen_raylib(lua_State *L) {
    // Iterate over the functions and add them to the table
	for (int i = 0; funcs[i].name; i++) {
		lua_pushcfunction(L, funcs[i].func);
		lua_setglobal(L, funcs[i].name);
	}
    lua_init_colors(L);

    init_raylib_keys(L);
    init_raylib_img(L);
    init_raylib_sound(L);
	initRaylibCamera(L);
	init_raygui(L);

	return 1;
}

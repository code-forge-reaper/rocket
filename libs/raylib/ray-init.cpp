#include <lua.h>
#include <lua.hpp>
#include "ray-color.cpp"
#include "../../../libs/lua_ffi.hpp"
#include <raylib.h>
#include <vector>
#include <cstring>

static int lua_is_key_down(lua_State *L) {
  int key = lua_tonumber(L, 1);
  lua_pushboolean(L, IsKeyDown(key));
  return 1;
}

static int lua_is_key_pressed(lua_State *L) {
  int key = lua_tonumber(L, 1);
  lua_pushboolean(L, IsKeyPressed(key));
  return 1;
}

static int lua_draw_fps(lua_State *L) {
  int x, y;
  x = lua_tonumber(L, 1);
  y = lua_tonumber(L, 2);
  DrawFPS(x, y);
  return 0;
}
static int lua_get_screen_width(lua_State*L){
  lua_pushnumber(L, GetScreenWidth());
  return 1;
}
static int lua_get_screen_height(lua_State*L){
  lua_pushnumber(L, GetScreenHeight());
  return 1;
}

static int lua_set_target_fps(lua_State *L) {
  int TargetFpsToMatch =
      luaL_optinteger(L, 1, 60); // defaults to 60 fps, if no value given
  SetTargetFPS(TargetFpsToMatch);
  return 0;
}


static int lua_draw_rectangle(lua_State *L) {
  int posX = lua_tonumber(L, 1);
  int posY = lua_tonumber(L, 2);
  int width = lua_tonumber(L, 3);
  int height = lua_tonumber(L, 4);

  Color color;
  color = lua_getColor(L, 5);

  DrawRectangle(posX, posY, width, height, color);
  return 0;
}

static int lua_draw_circle(lua_State *L) {
  int posX = lua_tonumber(L, 1);
  int posY = lua_tonumber(L, 2);
  int radius = lua_tonumber(L, 3);
  Color color;
  color = lua_getColor(L, 4);

  DrawCircle(posX, posY, radius, color);
  return 0;
}

static int lua_draw_rectangle_rect(lua_State *L) {
  Rectangle rect = {
    (float)getArgByName(L,"x", 1),
    (float)getArgByName(L,"y", 1),
    (float)getArgByName(L,"width", 1),
    (float)getArgByName(L,"height", 1)
  };

  Color color;
  color = lua_getColor(L, 2);

  DrawRectangleRec(rect, color);
  return 0;
};

static int lua_measure_text(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  int fontSize = lua_tonumber(L, 2);
  lua_pushinteger(L, MeasureText(text, fontSize));
  return 1;
}

// Wrapper function to create a window
static int lua_create_window(lua_State *L) {
  int width = lua_tonumber(L, 1);
  int height = lua_tonumber(L, 2);
  const char *title = luaL_checkstring(L, 3);
  #ifdef no_debug
  SetTraceLogLevel(LOG_NONE);
  #endif
  InitWindow(width, height, title);
  return 0; // No return values
}

// Wrapper function to close the window
static int lua_close_window(lua_State *L) {
  CloseWindow();
  return 0; // No return values
}

// Wrapper function to clear the background
static int lua_fill_bg(lua_State *L) {
  Color color;
  color = lua_getColor(L, 1);

  ClearBackground(color);
  return 0;
}

// Wrapper function to begin drawing
static int lua_start_drawing(lua_State *L) {
  BeginDrawing();
  return 0;
}

// Wrapper function to end drawing
static int lua_stop_drawing(lua_State *L) {
  EndDrawing();
  return 0;
}

static int change_title(lua_State *L){
  SetWindowTitle(lua_tostring(L,1));
  return 0;
}


static int lua_should_close_window(lua_State *L) {
  lua_pushboolean(L, WindowShouldClose());
  return 1;
}

// Wrapper function to draw text
static int lua_draw_text(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  int posX = lua_tonumber(L, 2);
  int posY = lua_tonumber(L, 3);
  int fontSize = lua_tonumber(L, 4);

  Color color;
  color = lua_getColor(L, 5);

  DrawText(text, posX, posY, fontSize, color);

  return 0;
}

static int addRaylibFunction(lua_State *L, int (*function)(lua_State *),
                             const char *name) {
  lua_pushcfunction(L, function);
  lua_setfield(L, -2, name);
  return 1;
}
// RLAPI Vector2 GetMousePosition(void);
static int lua_get_mouse_position(lua_State *L) {
  lua_newtable(L);
  Vector2 mouse_pos = GetMousePosition();
  lua_pushinteger(L, mouse_pos.x);
  lua_setfield(L, -2, "x");
  lua_pushinteger(L, mouse_pos.y);
  lua_setfield(L, -2, "y");
  return 1;
}

int getMouseX(lua_State *L) {
  lua_pushinteger(L, GetMouseX());
  return 1;
}

int getMouseY(lua_State *L) {
  lua_pushinteger(L, GetMouseY());
  return 1;
}

static int lua_get_time(lua_State *L) {
    lua_pushnumber(L, GetTime());  // Ensure this returns a float
    return 1;
}

static int lua_get_frame_time(lua_State *L) {
    lua_pushnumber(L, GetFrameTime());  // Ensure this returns a float
    return 1;
}

static int lua_get_fps(lua_State *L) {
    lua_pushinteger(L, GetFPS());  // FPS is fine as an integer
    return 1;
}

static int lua_is_mouse_button_down(lua_State *L) {
  int key = lua_tonumber(L, 1);
  lua_pushboolean(L, IsMouseButtonDown(key));
  return 1;
}

static int lua_is_mouse_button_released(lua_State *L) {
  int key = lua_tonumber(L, 1);
  lua_pushboolean(L, IsMouseButtonReleased(key));
  return 1;
}

static int lua_is_mouse_button_pressed(lua_State *L) {
  int key = lua_tonumber(L, 1);
  lua_pushboolean(L, IsMouseButtonPressed(key));
  return 1;
}



#include "ray-keys.cpp"
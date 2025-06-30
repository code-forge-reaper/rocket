#pragma once
#include <raylib.h>
#include <lua.hpp>
#include <cstring>
static void lua_pushColor(lua_State *L, Color color, const char *name) {
  lua_newtable(L);

  lua_pushstring(L, "r");
  lua_pushinteger(L, color.r);
  lua_settable(L, -3);

  lua_pushstring(L, "g");
  lua_pushinteger(L, color.g);
  lua_settable(L, -3);

  lua_pushstring(L, "b");
  lua_pushinteger(L, color.b);
  lua_settable(L, -3);

  lua_pushstring(L, "a");
  lua_pushinteger(L, color.a);
  lua_settable(L, -3);

  lua_setglobal(L, name);
}

static Color lua_getColor(lua_State *L, int StartOfTable) {
  Color color = {0}; // Default to black in case of failure

  // Check if the provided argument is a table
  if (!lua_istable(L, StartOfTable)) {
    luaL_error(L, "Argument is not a table");
    return color;
  }

  // Iterate through the table
  lua_pushnil(L); // Push a nil key to start the iteration

  while (lua_next(L, StartOfTable) != 0) {
    // Key is at index -2 and value is at index -1

    // Assuming you have 'r', 'g', 'b', and 'a' keys in the table
    const char *key = lua_tostring(L, -2);

    if (key) {
      if (strcmp(key, "r") == 0) {
        color.r = lua_tointeger(L, -1);
      } else if (strcmp(key, "g") == 0) {
        color.g = lua_tointeger(L, -1);
      } else if (strcmp(key, "b") == 0) {
        color.b = lua_tointeger(L, -1);
      } else if (strcmp(key, "a") == 0) {
        color.a = lua_tointeger(L, -1);
      }
    }

    // Pop the value, leaving the key for the next iteration
    lua_pop(L, 1);
  }

  return color;
}

// Function to create a new color from a lua/lua table
static int lua_create_color(lua_State *L) {
  // Check if the argument is a table
  if (!lua_istable(L, 1)) {
    return luaL_error(L, "Argument is not a table");
  }
  const char *name = luaL_checkstring(L, 2);

  // Default color
  Color color = lua_getColor(L, 1);
  printf("%i\n", color.r);
  printf("%i\n", color.g);
  printf("%i\n", color.b);
  printf("%i\n", color.a);
  printf("%s\n", name);

  // Push the color onto the lua stack with a specified name
  lua_pushColor(L, color, name);

  return 0; // Number of return values
}
static int lua_init_colors(lua_State *L) {
  lua_pushColor(L, RED, "red");
  lua_pushColor(L, GREEN, "green");
  lua_pushColor(L, BLUE, "blue");
  lua_pushColor(L, WHITE, "white");
  lua_pushColor(L, LIGHTGRAY, "lightgray");
  lua_pushColor(L, BLACK, "black");
  lua_pushColor(L, MAGENTA, "magenta");
  lua_pushColor(L, YELLOW, "yellow");
  lua_pushColor(L, BLANK, "blank"); // transparent

  // True colors
  lua_pushColor(L, Color{255,0,0,255},"TRUERED");
  lua_pushColor(L, Color{0,255,0,255},"TRUESGREEN");
  lua_pushColor(L, Color{0,0,255,255},"TRUEBLUE");
  lua_pushColor(L, Color{0,0,200,255},"TRUEDARKBLUE");
  lua_pushColor(L, Color{255,255,0,255},"TRUEYELLOW");
  lua_pushColor(L, Color{255,0,255,255},"TRUEMAGENTA");
  lua_pushColor(L, Color{0,255,255,255},"TRUECYAN");


  return 1;
}


/*
from raylib

// Some Basic Colors
// NOTE: Custom raylib color palette for amazing visuals on WHITE background
#define LIGHTGRAY  CLITERAL(Color){ 200, 200, 200, 255 }   // Light Gray
#define GRAY       CLITERAL(Color){ 130, 130, 130, 255 }   // Gray
#define DARKGRAY   CLITERAL(Color){ 80, 80, 80, 255 }      // Dark Gray
#define YELLOW     CLITERAL(Color){ 253, 249, 0, 255 }     // Yellow
#define GOLD       CLITERAL(Color){ 255, 203, 0, 255 }     // Gold
#define ORANGE     CLITERAL(Color){ 255, 161, 0, 255 }     // Orange
#define PINK       CLITERAL(Color){ 255, 109, 194, 255 }   // Pink
#define RED        CLITERAL(Color){ 230, 41, 55, 255 }     // Red
#define MAROON     CLITERAL(Color){ 190, 33, 55, 255 }     // Maroon
#define GREEN      CLITERAL(Color){ 0, 228, 48, 255 }      // Green
#define LIME       CLITERAL(Color){ 0, 158, 47, 255 }      // Lime
#define DARKGREEN  CLITERAL(Color){ 0, 117, 44, 255 }      // Dark Green
#define SKYBLUE    CLITERAL(Color){ 102, 191, 255, 255 }   // Sky Blue
#define BLUE       CLITERAL(Color){ 0, 121, 241, 255 }     // Blue
#define DARKBLUE   CLITERAL(Color){ 0, 82, 172, 255 }      // Dark Blue
#define PURPLE     CLITERAL(Color){ 200, 122, 255, 255 }   // Purple
#define VIOLET     CLITERAL(Color){ 135, 60, 190, 255 }    // Violet
#define DARKPURPLE CLITERAL(Color){ 112, 31, 126, 255 }    // Dark Purple
#define BEIGE      CLITERAL(Color){ 211, 176, 131, 255 }   // Beige
#define BROWN      CLITERAL(Color){ 127, 106, 79, 255 }    // Brown
#define DARKBROWN  CLITERAL(Color){ 76, 63, 47, 255 }      // Dark Brown

#define WHITE      CLITERAL(Color){ 255, 255, 255, 255 }   // White
#define BLACK      CLITERAL(Color){ 0, 0, 0, 255 }         // Black
#define BLANK      CLITERAL(Color){ 0, 0, 0, 0 }           // Blank (Transparent)
#define MAGENTA    CLITERAL(Color){ 255, 0, 255, 255 }     // Magenta
#define RAYWHITE   CLITERAL(Color){ 245, 245, 245, 255 }   // My own White (raylib logo)
*/
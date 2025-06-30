extern "C" {
#include "microui.h"
#include "microui.c"
}
#include "../../ffi.hpp" // getArgByName
#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.hpp"

#define FONT_SIZE 16

int text_width(mu_Font font, const char *str, int len) {
  return MeasureText(str, FONT_SIZE);
}

int text_height(mu_Font font) { return FONT_SIZE; }
static int init_mic(lua_State *L) {
  mu_Context *ctx = (mu_Context *)malloc(sizeof(mu_Context));
  mu_init(ctx);
  ctx->text_height = text_height;
  ctx->text_width = text_width;
  pushPtr(L, ctx); // or false if you're managing it yourself

  return 1;
}
static int begin_mic(lua_State *L) {
  mu_Context *ctx = getPtr<mu_Context>(L, 1);
  mu_begin(ctx);
  return 0;
}
static mu_Rect unclipped_rect = {0, 0, 0x1000000, 0x1000000};

static int update_mic(lua_State *L) {
  mu_Context *ctx = getPtr<mu_Context>(L, 1);

  int k = 0;
  while ((k = GetCharPressed()) != 0) {
    char chr[2] = {char(k), 0};
    mu_input_text(ctx, chr);
  }

  /*
  enum {
    MU_MOUSE_LEFT       = (1 << 0),
    MU_MOUSE_RIGHT      = (1 << 1),
    MU_MOUSE_MIDDLE     = (1 << 2)
  };
  */
  int _mx = GetMouseX();
  int _my = GetMouseY();
  mu_input_mousemove(ctx, _mx, _my);
  for (int btn = MOUSE_LEFT_BUTTON; btn <= MOUSE_BUTTON_MIDDLE; btn++) {
    if (IsMouseButtonPressed(btn)) {
      mu_input_mousedown(ctx, _mx, _my, 1 << btn);
    } else if (IsMouseButtonReleased(btn)) {
      mu_input_mouseup(ctx, _mx, _my, 1 << btn);
    }
  }
  Vector2 scroll = GetMouseWheelMoveV();
  mu_input_scroll(ctx, scroll.x, scroll.y);

  /*

  enum {
          MU_KEY_SHIFT        = (1 << 0),
          MU_KEY_CTRL         = (1 << 1),
          MU_KEY_ALT          = (1 << 2),
          MU_KEY_BACKSPACE    = (1 << 3),
          MU_KEY_RETURN       = (1 << 4)
  };
  */
  int keys[8][2] = {
      // i literaly had no clue this thing would work, i guess i understand a
      // bit of C
      {KEY_LEFT_SHIFT, MU_KEY_SHIFT},  {KEY_RIGHT_SHIFT, MU_KEY_SHIFT},
      {KEY_LEFT_CONTROL, MU_KEY_CTRL}, {KEY_RIGHT_CONTROL, MU_KEY_CTRL},
      {KEY_LEFT_ALT, MU_KEY_ALT},      {KEY_RIGHT_ALT, MU_KEY_ALT},

      {KEY_ENTER, MU_KEY_RETURN},      {KEY_BACKSPACE, MU_KEY_BACKSPACE},
  };

  for (int i = 0; i < 8; i++) {
    if (IsKeyDown(keys[i][0])) {
      mu_input_keydown(ctx, keys[i][1]);
    } else if (IsKeyReleased(keys[i][0])) {
      mu_input_keyup(ctx, keys[i][1]);
    }
  }

  mu_Command *cmd = NULL;
  // mu_Command **cmd
  while (mu_next_command(ctx, &cmd)) {
    switch (cmd->type) {
    case MU_COMMAND_JUMP: {
      const char *msg =
          TextFormat("%s:%i: not handled yet\n", __FILE__, __LINE__ + 1);
      fprintf(stderr, "%s\n", msg);
      abort();

    } break;
    case MU_COMMAND_CLIP: {
      if (memcmp(&cmd->clip.rect, &unclipped_rect, sizeof(mu_Rect)) == 0) {
        EndScissorMode();
      } else {
        BeginScissorMode(cmd->clip.rect.x, cmd->clip.rect.y, cmd->clip.rect.w,
                         cmd->clip.rect.h);
      }
    } break;
    case MU_COMMAND_RECT: {
      DrawRectangle(cmd->rect.rect.x, cmd->rect.rect.y, cmd->rect.rect.w,
                    cmd->rect.rect.h,
                    (Color){cmd->rect.color.r, cmd->rect.color.g,
                            cmd->rect.color.b, cmd->rect.color.a});
    } break;
    case MU_COMMAND_TEXT: {
      int posX = cmd->text.pos.x;
      int posY = cmd->text.pos.y;

      int w = MeasureText(cmd->text.str, FONT_SIZE);

      DrawText(cmd->text.str, posX, posY, FONT_SIZE,
               (Color){cmd->text.color.r, cmd->text.color.g, cmd->text.color.b,
                       cmd->text.color.a});

    } break;
    case MU_COMMAND_ICON: {
      // i have no clue how to render the icons microui wants
      DrawRectangle(cmd->icon.rect.x, cmd->icon.rect.y, cmd->icon.rect.w,
                    cmd->icon.rect.h, RED);
      if (cmd->icon.id == MU_ICON_CLOSE) {
        DrawLine(cmd->icon.rect.x, cmd->icon.rect.y,
                 cmd->icon.rect.x + cmd->icon.rect.w,
                 cmd->icon.rect.y + cmd->icon.rect.h, WHITE);
        DrawLine(cmd->icon.rect.x + cmd->icon.rect.w, cmd->icon.rect.y,
                 cmd->icon.rect.x, cmd->icon.rect.y + cmd->icon.rect.h, WHITE);
      }

      // DrawText(TextFormat("%i",cmd->icon.id), cmd->icon.rect.x,
      // cmd->icon.rect.y, 8, WHITE);

      /*
      const char*msg =TextFormat("%s:%i: not handled yet\n", __FILE__,
      __LINE__+1); fprintf(stderr, "%s\n", msg); abort();
      */

    } break;
    default:
      break;
    }
  }
  return 0;
}

static int end_mic(lua_State *L) {
  mu_Context *ctx = getPtr<mu_Context>(L, 1);
  mu_end(ctx);
  return 0;
}

void setup_microui(lua_State *L) {
  static luaL_Reg funcs[] = {
      {"Init", init_mic},     {"Begin", begin_mic}, {"End", end_mic},
      {"Update", update_mic}, {NULL, NULL},
  };

  newModule("UI", funcs, L);
}

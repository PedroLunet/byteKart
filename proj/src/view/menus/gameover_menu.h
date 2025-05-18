#ifndef GAMEOVER_MENU_H
#define GAMEOVER_MENU_H

#include <stdint.h>
#include "model/game_state.h"
#include "sprites/sprite.h"
#include "macros.h"
#include "xpm/xpm_files.h"
#include "macros.h"
#include "controller/video_card.h"
#include "controller/mouse.h"
#include "fonts/font.h"
#include "view/utils/ui.h"
#include "view/utils/ui_utils.h"

typedef enum {
  GAMEOVER_MENU,
  GAMEOVER_RESTART,
  GAMEOVER_MAIN_MENU,
  GAMEOVER_QUIT_BUTTON,
  GAMEOVER_EXITED
} GameOverSubstate;

typedef struct {
  GameState base;
  GameOverSubstate currentGameOverSubstate;
  Sprite *gameOverSprite;
  Sprite *yourScoreSprite;
  Sprite *restartSprite;
  Sprite *mainMenuSprite;
  Sprite *quitSprite;
  int selectedOption;
  UIComponent *uiRoot;
} GameOver;

GameOver *gameover_menu_create();
void gameover_menu_destroy(GameOver *this);
void gameover_draw(GameOver *this);
void gameover_process_event(GameOver *this, EventType event);
GameOverSubstate gameover_get_current_substate(GameOver *this);
void gameover_reset_state(GameOver *this);


#endif






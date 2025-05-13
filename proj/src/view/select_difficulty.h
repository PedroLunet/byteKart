#ifndef SELECT_DIFFICULTY_H
#define SELECT_DIFFICULTY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "model/game_state.h"
#include "view/ui.h"
#include "macros.h"
#include "controller/video_card.h"
#include "controller/mouse.h"

typedef enum {
    DIFFICULTY_START,
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD,
    DIFFICULTY_EXITED
} DifficultyLevel;

typedef struct SelectDifficulty {
    GameState base;
    int selectedOption;
    DifficultyLevel chosenLevel;
    UIComponent *uiRoot;
} SelectDifficulty;

// Public SelectDifficulty Class Methods
SelectDifficulty *select_difficulty_create();
void select_difficulty_destroy(SelectDifficulty *this);
void select_difficulty_draw(SelectDifficulty *this);
void select_difficulty_process_event(SelectDifficulty *this, EventType event);
DifficultyLevel select_difficulty_get_chosen_level(SelectDifficulty *this);
void select_difficulty_reset_state(SelectDifficulty *this);

#endif //SELECT_DIFFICULTY_H

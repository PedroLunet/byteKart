#ifndef MAIN_H
#define MAIN_H

typedef enum {
    EVENT_NONE,
    EVENT_TIMER,
    EVENT_KEYBOARD,
    EVENT_MOUSE,
    // EVENT_SERIAL,
    NUM_EVENTS
} EventType;

#endif //MAIN_H

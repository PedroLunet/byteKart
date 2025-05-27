#ifndef RACE_RESULT_H
#define RACE_RESULT_H

// Structure to hold race result data
typedef struct {
    int position;
    char name[20];
    int id;
    int lap;
    int segment;
    int score;
    float race_time; 
} RaceResult;

#endif

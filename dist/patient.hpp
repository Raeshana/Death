#pragma once
#include "DialogueMode.hpp"
#include <vector>

struct Regret_Item{
    bool collected = false;
};

struct Patient{
    // Pointer to this patient's dialogue mode
    // To keep the dialogue state 
    // After player starts dialogue with someone else
    std::shared_ptr< DialogueMode > dialogue;
    string script_file;

    // Pointer to this patient's room mode
    // To keep the room state
    // After player leaves the room
    std::shared_ptr< RoomMode > room = 0;

    // This is the index of the room in room maps
    int room_num;

    // Basic info
    string name;
    std::vector<Regret_Item> regret_items; // list of items that can reduce regret
    int difficulty = 5; // current soul capture difficulty (easiest 1-5 hardest)

    // functions
};  
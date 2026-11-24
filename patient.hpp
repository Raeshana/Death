#pragma once

#include <memory>
#include <vector>

#include "DialogueMode.hpp"
#include "RoomMode.hpp"
#include <string>

struct Regret_Item{
    // true if the item has been collected
    bool collected;
};

struct Patient{
    // Pointer to this patient's dialogue mode
    // To keep the dialogue state
    // After player starts dialogue with someone else
    std::shared_ptr< DialogueMode > dialogue = nullptr;
    std::string script_file = " ";

    // Pointer to this patient's room mode
    // To keep the room state
    // After player leaves the room
    std::shared_ptr< RoomMode > room = nullptr;

    // This is the index of the room in room maps
    int room_num = 0;

    // Basic info
    std::string name = " ";
    // list of items player can collect to lower difficulty
    std::vector<Regret_Item> regret_items; 
    int difficulty = 5; // current sould capture difficulty

    // functions
};
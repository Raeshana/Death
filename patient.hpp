#pragma once

#include <memory>
#include <vector>

#include "DialogueMode.hpp"
#include "PlayMode.hpp"
#include <string>

struct Regret_Item{
    // true if the item has been collected
    bool collected;
};

struct Patient{
    Patient(std::string patient_name, int room_num);
    ~Patient();
    // Pointer to this patient's dialogue mode
    // To keep the dialogue state
    // After player starts dialogue with someone else
    std::shared_ptr< Mode > dialogue;

    // This is the index of the room in room maps
    int room_num = 0;

    // // Pointer to this patient's room mode
    // // To keep the room state
    // // After player leaves the room
    std::shared_ptr< Mode > room;

    // Basic info
    std::string name = " ";
    // list of items player can collect to lower difficulty
    Regret_Item regret_item; 
    int difficulty = 5; // current sould capture difficulty

    // void create_patient(std::string patient_name);
};

struct PatientList{
    PatientList(std::string name1, std::string name2, std::string name3, std::string name4, std::string name5);
    ~PatientList();
    std::vector<Patient> patients;
};
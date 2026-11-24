#include "patient.hpp";

Patient::Patient(){
    dialogue = null_ptr;
    script_file = "";
    room = null_ptr;
    room_num = 0;
    name = "";
    regret_items = empty;
    difficulty = 5; // highest by default
}

Patient::~Patient(){
}
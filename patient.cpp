#include "patient.hpp"

PatientList::PatientList(std::string name1, std::string name2, std::string name3, std::string name4, std::string name5){
    patients = {Patient(name1, 1), Patient(name2, 2), Patient(name3, 3), Patient(name4, 4), Patient(name5, 5)};
}

PatientList::~PatientList(){
}

Patient::Patient(std::string patient_name, int room_num){
    // pointers
    dialogue = std::make_shared< DialogueMode >("final_script.bin");
    room = std::make_shared< PlayMode >(room_num);

    // room num and increment curr num
    room_num = room_num;

    // other stuff
    name = patient_name;
}

Patient::~Patient(){
}
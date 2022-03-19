#include "include/utilities.h"
#include <string>
#include <iostream>
void debugPrint(std::string string){
    if(config::debug){
        std::cout << string << "\r\n";
    }
    return;
}
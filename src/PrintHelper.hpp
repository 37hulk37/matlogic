#ifndef PRINT_HELPER
#define PRINT_HELPER

#include "BDDHelper.hpp"

std::string to_string(bddHelper::Object obj);
std::string to_string(bddHelper::Property prop);
std::string to_string(bddHelper::Hair col);
std::string to_string(bddHelper::Nation col);
std::string to_string(bddHelper::Transport col);
std::string to_string(bddHelper::Owns col);

#endif

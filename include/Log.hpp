#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include "MainHeader.hpp"

class Log {
    public:
        static void    console(e_log log, const std::string &msg);
};
#endif
#ifndef MAIN_HEADER_HPP
#define MAIN_HEADER_HPP

#include "SimpleSocket.hpp"
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include "BindingSocket.hpp"
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <sys/socket.h>
#include <ctime>

#define RED "\033[0;31m"
#define YELLOW "\x1b[38;5;228m"
#define GREEN "\033[0;32m"
#define RESET "\033[0m"
#define CLEAR "\033[2J\033[H"

typedef enum s_log
{
    ERROR,
    WARNING,
    MESSAGE,
} e_log;

# define DEF_CONFIG "./conf/valid/example.conf"
#endif
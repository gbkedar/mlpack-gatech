/**
 * @file log.cpp
 * @author Matthew Amidon
 *
 * Implementation of the Log class.
 */
#include <iostream>

#include "log.hpp"

// Color code escape sequences.
#define BASH_RED "\033[0;31m"
#define BASH_GREEN "\033[0;32m"
#define BASH_YELLOW "\033[0;33m"
#define BASH_CYAN "\033[0;36m"
#define BASH_CLEAR "\033[0m"

using namespace mlpack;
using namespace mlpack::io;

// Only output debugging output if in debug mode.
#ifdef DEBUG
PrefixedOutStream Log::Debug = PrefixedOutStream(std::cout,
    BASH_CYAN "[DEBUG] " BASH_CLEAR);
#else
NullOutStream Log::Debug = NullOutStream();
#endif

PrefixedOutStream Log::Info = PrefixedOutStream(std::cout,
    BASH_GREEN "[INFO ] " BASH_CLEAR, true /* unless --verbose */, false);
PrefixedOutStream Log::Warn = PrefixedOutStream(std::cout,
    BASH_YELLOW "[WARN ] " BASH_CLEAR, false, false);
PrefixedOutStream Log::Fatal = PrefixedOutStream(std::cerr,
    BASH_RED "[FATAL] " BASH_CLEAR, false, true /* fatal */);

std::ostream& Log::cout = std::cout;

// Only do anything for Assert() if in debugging mode.
#ifdef DEBUG
void Log::Assert(bool condition, const char* message)
{
  if(!condition)
  {
    Log::Debug << message << std::endl;
    exit(1);
  }
}
#else
void Log::Assert(bool condition, const char* message)
{ }
#endif

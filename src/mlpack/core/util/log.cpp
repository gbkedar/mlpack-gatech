/**
 * @file log.cpp
 * @author Matthew Amidon
 *
 * Implementation of the Log class.
 */
#include <cxxabi.h>
#include <execinfo.h>

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
  if (!condition)
  {
    void* array[25];
    size_t size = backtrace (array, sizeof(array)/sizeof(void*));
    char** messages = backtrace_symbols(array, size);

    // skip first stack frame (points here)
    for (size_t i = 1; i < size && messages != NULL; ++i)
    {
      char *mangledName = 0, *offsetBegin = 0, *offsetEnd = 0;

      // find parantheses and +address offset surrounding mangled name
      for (char *p = messages[i]; *p; ++p)
      {
        if (*p == '(')
        {
          mangledName = p;
        }
        else if (*p == '+')
        {
          offsetBegin = p;
        }
        else if (*p == ')')
        {
          offsetEnd = p;
          break;
        }
      }

      // if the line could be processed, attempt to demangle the symbol
      if (mangledName && offsetBegin && offsetEnd &&
          mangledName < offsetBegin)
      {
        *mangledName++ = '\0';
        *offsetBegin++ = '\0';
        *offsetEnd++ = '\0';

        int status;
        char* realName = abi::__cxa_demangle(mangledName, 0, 0, &status);

        // if demangling is successful, output the demangled function name
        if (status == 0)
        {
          Log::Debug << "[bt]: (" << i << ") " << messages[i] << " : "
                    << realName << "+" << offsetBegin << offsetEnd
                    << std::endl;

        }
        // otherwise, output the mangled function name
        else
        {
          Log::Debug << "[bt]: (" << i << ") " << messages[i] << " : "
                    << mangledName << "+" << offsetBegin << offsetEnd
                    << std::endl;
        }
        free(realName);
      }
      // otherwise, print the whole line
      else
      {
          Log::Debug << "[bt]: (" << i << ") " << messages[i] << std::endl;
      }
    }
    Log::Debug << message << std::endl;
    free(messages);

    //backtrace_symbols_fd (array, size, 2);
    exit(1);
  }
}
#else
void Log::Assert(bool condition, const char* message)
{ }
#endif

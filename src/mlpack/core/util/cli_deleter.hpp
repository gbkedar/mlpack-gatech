/**
 * @file cli_deleter.hpp
 * @author Ryan Curtin
 *
 * Definition of the CLIDeleter() class.
 */
#ifndef __MLPACK_CORE_IO_CLI_DELETER_HPP
#define __MLPACK_CORE_IO_CLI_DELETER_HPP

namespace mlpack {
namespace io {

/**
 * Extremely simple class whose only job is to delete the existing CLI object at
 * the end of execution.  This is meant to allow the user to avoid typing
 * 'CLI::Destroy()' at the end of their program.  The file also defines a static
 * CLIDeleter class, which will be initialized at the beginning of the program
 * and deleted at the end.  The destructor destroys the CLI singleton.
 */
class CLIDeleter
{
 public:
  CLIDeleter();
  ~CLIDeleter();
};

//! Declare the deleter.
static CLIDeleter cliDeleter;

}; // namespace io
}; // namespace mlpack

#endif

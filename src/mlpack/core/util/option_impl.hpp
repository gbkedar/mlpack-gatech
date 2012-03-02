/**
 * @file option_impl.hpp
 * @author Matthew Amidon
 *
 * Implementation of template functions for the Option class.
 */
#ifndef __MLPACK_CORE_IO_OPTION_IMPL_HPP
#define __MLPACK_CORE_IO_OPTION_IMPL_HPP

// Just in case it has not been included.
#include "option.hpp"

namespace mlpack {
namespace io {

/**
 * Registers a parameter with CLI.
 */
template<typename N>
Option<N>::Option(bool ignoreTemplate,
                N defaultValue,
                const std::string& identifier,
         	const std::string& description,
         	const std::string& alias,
                bool required)
{
  if (ignoreTemplate)
  {
    CLI::Add(identifier, description, alias, required);
  }
  else
  {
    CLI::Add<N>(identifier, description, alias, required);

    CLI::GetParam<N>(identifier) = defaultValue;
  }
}


/**
 * Registers a flag parameter with CLI.
 */
template<typename N>
Option<N>::Option(const std::string& identifier,
         	const std::string& description,
         	const std::string& alias)
{
  CLI::AddFlag(identifier, description, alias);
}

}; // namespace io
}; // namespace mlpack

#endif

/**
 * @file kernel_traits.hpp
 * @author Ryan Curtin
 *
 * This provides the KernelTraits class, a template class to get information
 * about various kernels.
 */
#ifndef __MLPACK_CORE_KERNELS_KERNEL_TRAITS_HPP
#define __MLPACK_CORE_KERNELS_KERNEL_TRAITS_HPP

namespace mlpack {
namespace kernel {

/**
 * This is a template class that can provide information about various kernels.
 * By default, this class will provide the weakest possible assumptions on
 * kernels, and each kernel should override values as necessary.  If a kernel
 * doesn't need to override a value, then there's no need to write a
 * KernelTraits specialization for that class.
 */
template<typename KernelType>
class KernelTraits
{
 public:
  /**
   * If true, then the kernel is normalized: K(x, x) = K(y, y) = 1 for all x.
   */
  static const bool IsNormalized = false;
};

}; // namespace kernel
}; // namespace mlpack

#endif

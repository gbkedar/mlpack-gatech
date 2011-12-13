/**
 * @file example_kernel.hpp
 * @author Ryan Curtin
 *
 * This is an example kernel.  If you are making your own kernel, follow the
 * outline specified in this file.
 */
#ifndef __MLPACK_CORE_KERNELS_EXAMPLE_KERNEL_H
#define __MLPACK_CORE_KERNELS_EXAMPLE_KERNEL_H

core.h>/core.h>

namespace mlpack {

/**
 * @brief Kernel functions.
 *
 * This namespace contains kernel functions, which evaluate some kernel function
 * @f$ K(x, y) @f$ for some arbitrary vectors @f$ x @f$ and @f$ y @f$ of the
 * same dimension.  The single restriction on the function @f$ K(x, y) @f$ is
 * that it must satisfy Mercer's condition:
 *
 * @f[
 * \int \int K(x, y) g(x) g(y) dx dy \ge 0
 * @f]
 *
 * for all square integrable functions @f$ g(x) @f$.
 *
 * The kernels in this namespace all implement the same methods as the
 * ExampleKernel class.  Any additional custom kernels should implement all the
 * methods that class implements; in addition, any method using a kernel should
 * rely on any arbitrary kernel function class having a default constructor and
 * a function
 *
 * @code
 * double Evaluate(arma::vec&, arma::vec&);
 * @endcode
 */
namespace kernel {

/**
 * An example kernel function.  This is not a useful kernel, but it implements
 * the two functions necessary to satisfy the Kernel policy (so that a class can
 * be used whenever an MLPACK method calls for a `typename Kernel` template
 * parameter.
 *
 * All that is necessary is a constructor and an `Evaluate()` function.  More
 * methods could be added; for instance, one useful idea is a constructor which
 * takes parameters for a kernel (for instance, the width of the Gaussian for a
 * Gaussian kernel).  However, MLPACK methods cannot count on these various
 * constructors existing, which is why most methods allow passing an
 * already-instantiated kernel object (and by default the method will construct
 * the kernel with the default constructor).  So, for instance,
 *
 * @code
 * GaussianKernel k(5.0);
 * KDE<GaussianKernel> kde(dataset, k);
 * @endcode
 *
 * will set up KDE using a Gaussian kernel with a width of 5.0, but
 *
 * @code
 * KDE<GaussianKernel> kde(dataset);
 * @endcode
 *
 * will create the kernel with the default constructor.  It is important (but
 * not strictly mandatory) that your default constructor still gives a working
 * kernel.
 *
 * @note
 * Not all kernels require state.  For instance, the regular dot product needs
 * no parameters.  In that case, no local variables are necessary and
 * `Evaluate()` can (and should) be declared static.  However, for greater
 * generalization, MLPACK methods expect all kernels to require state and hence
 * must store instantiated kernel functions; this is why a default constructor
 * is necessary.
 * @endnote
 */
class ExampleKernel
{
 public:
  /**
   * The default constructor, which takes no parameters.  Because our simple
   * example kernel has no internal parameters that need to be stored, the
   * constructor does not need to do anything.  For a more complex example, see
   * the GaussianKernel, which stores an internal parameter.
   */
  ExampleKernel() { }

  /**
   * Evaluates the kernel function for two given vectors.  In this case, because
   * our simple example kernel has no internal parameters, we can declare the
   * function static.  For a more complex example which cannot be declared
   * static, see the GaussianKernel, which stores an internal parameter.
   *
   * @tparam VecType Type of vector (arma::vec, arma::spvec should be expected).
   * @param a First vector.
   * @param b Second vector.
   * @return K(a, b).
   */
  template<typename VecType>
  static double Evaluate(const VecType& a, const VecType& b) { return 0; }
};

}; // namespace kernel
}; // namespace mlpack

#endif

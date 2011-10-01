/**
 * @file fastlib.h
 *
 * Includes all of fastlib.
 */

#ifndef FASTLIB_FASTLIB_H
#define FASTLIB_FASTLIB_H

#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <stdint.h>

#include "data/dataset.h"
#include "math/math_lib.h"
#include "file/textfile.h"
#include "tree/spacetree.h"
#include "tree/bounds.h"
#include "tree/statistic.h"
#include "fx/io.h"

#include "base/arma_extend.h"

#endif

/**
 * @mainpage FASTlib Documentation
 *
 * @section intro_sec Introduction
 *
 * First make sure you have looked at the tutorial on the wiki 
 * and sample code in @c examples/platonic_allnn.  The
 * sample code is meant as a starting point -- you should begin by hacking
 * onto the sample code, though make sure to make your own copy so that
 * you don't check it into subversion.
 *
 * The cookbook should be built as a sort of micro-howto for small tasks that
 * you are interested in doing.
 * However, this Doxygen documentation will always be the
 * most complete -- and Doxygen links to the cross-referenced code if the
 * comments aren't good enough.  If you must declare classes in header
 * files that you don't want documented because they are just helpers,
 * remove them to a file ending in "_impl.h" and it will be skipped.
 *
 * @section using_doxygen Using Doxygen in Your Code
 *
 * So, you want your code to be documented like this, too?  Doxygen will,
 * like Javadoc, scrape your source code for comments with the specific
 * slash-star-star begining.  It's best to comment header files and not
 * source files -- even without Doxygen, header files remain a natural place
 * for documentation in any case.
 *
 * To generate this HTML documentation yourself, go in to the source
 * directory and type:
 *
 * @code
 * doxygen
 * @endcode
 *
 * Then, visit @c doc/html/index.html (inside the same directory).
 *
 * @section remarks Final Remarks
 *
 * This software was written at Georgia Institute of Technology.
 * This software is not yet ready for distribution.
 *
 * The current core maintainers are rriegel@cc.gatech.edu
 * and nadeem@cc.gatech.edu.  
 *
 * The source for this main page is in @c fastlib/fastlib.h.
 */

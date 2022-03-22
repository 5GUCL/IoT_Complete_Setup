/*!
 * \file
 * \brief Helpers for loops.
 * \since
 * v.3.0.0
 */

#pragma once

#include <cpp_util_3/ensure.hpp>

#include <type_traits>
#include <stdexcept>

namespace cpp_util_3 {

/*!
 * \brief Helper for doing some action for N times.
 *
 * Usage example:
 * \code
	cpp_util_3::n_times( 3, []{ cout << "===" << endl; } );
 * \endcode
 *
 * \note
 * This function is implemented as:
 * \code
	for(COUNT_TYPE i{}; i < n; ++i) body();
 * \endcode
 * It means that in case when COUNT_TYPE is signed and \a n is a negative
 * there won't be an invocation of \a body.
 *
 * \tparam COUNT_TYPE type of loop counter.
 * \tparam LAMBDA lambda or functional object to be body of the loop.
 */
template< typename COUNT_TYPE, typename LAMBDA >
void
n_times(
	//! Count of invocations of \a body.
	const COUNT_TYPE n,
	//! An action which should be performed \a n times.
	LAMBDA && body )
	{
		for( COUNT_TYPE i{}; i < n; ++i )
			body();
	}

} /* namespace cpp_util_3 */


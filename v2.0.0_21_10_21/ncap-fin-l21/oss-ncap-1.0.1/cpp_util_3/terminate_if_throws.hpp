/*!
 * \file
 * \brief A helper function to ensure that terminate() will be called in
 * the case if block of code throws an exception.
 *
 * \since
 * v.3.0.0
 */

#pragma once

namespace cpp_util_3 {

/*!
 * \brief A helper function to ensure that terminate() will be called in
 * the case if block of code throws an exception.
 *
 * Usage example:
 * \code
 * void some_complex_class::modify_state(const modification_request & req)
 * {
 * 	// Preparation for modification. Exceptions can be thrown here.
 * 	check_first_condition(req);
 * 	check_second_condition(req);
 * 	...
 * 	check_last_condition(req);
 *
 * 	// All conditions are fulfilled. We can do modification.
 * 	// We don't expect exceptions here. std::terminate must be called
 * 	// if some exception will be thrown.
 * 	cpp_util_3::terminate_if_throws( [&] {
 * 		modification_action_one(req);
 * 		modifiaction_action_two(req);
 * 		...
 * 		modification_action_last(req);
 * 	});
 * }
 * \endcode
 *
 * \note
 * This function can be used only with C++11 compilers which support
 * noexcept keyword.
 *
 * \since
 * v.3.0.0
 */
template< typename L >
auto terminate_if_throws( L lambda ) noexcept -> decltype(lambda())
	{
		return lambda();
	}

} /* namespace cpp_util_3 */


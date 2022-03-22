/*!
 * \file
 * \brief A simple implementation of %at_scope_exit concept.
 *
 * \since
 * v.2.7.2
 */

#pragma once

#include <utility>

namespace cpp_util_3 {

namespace scope_exit_details {

/*!
 * \brief Helper class for scope exit implementation.
 * \since
 * v.2.7.2
 */
template< typename L >
class at_exit_t
	{
		L m_lambda;
		bool m_callable{true};

	public :
		at_exit_t( const L & l ) : m_lambda{ l } {}
		at_exit_t( L && l ) : m_lambda{ std::move(l) } {}
		at_exit_t( at_exit_t && o ) : m_lambda{ std::move(o.m_lambda) }
			{
				o.m_callable = false;
			}
		~at_exit_t() { if(m_callable) m_lambda(); }
	};

} /* namespace scope_exit_details */

/*!
 * \brief Helper function for creation action to be performed at scope exit.
 *
 * Usage example:
 * \code
	if( needs_wait )
	{
		m_threads_to_wakeup += 1;
		auto decrement_threads = at_scope_exit( [&m_threads_to_wakeup] {
			--m_threads_to_wakeup;
		} );
		m_sleep_cond.wait_for( some_time, some_predicate );
	}
 * \endcode
 *
 * \since
 * v.2.7.2
 */
template< typename L >
scope_exit_details::at_exit_t< typename std::decay<L>::type >
at_scope_exit( L && l )
	{
		using actual_parameter_t = typename std::decay<L>::type;
		return scope_exit_details::at_exit_t<actual_parameter_t>{
				std::forward<L>(l) };
	}

} /* namespace cpp_util_3 */


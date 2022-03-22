/*!
 * \file
 * \brief Helpers for do rollback actions in the case of exception.
 * \since
 * v.2.7.2
 */

#pragma once

namespace cpp_util_3 {

namespace rollback_on_exception_details {

/*!
 * \brief Helper template class for do rollback actions automatically
 * in the destructor.
 *
 * \tparam L type of lambda with rollback actions.
 *
 * \since
 * v.2.7.2
 */
template< typename L >
class rollbacker_t
	{
		L & m_action;
		bool m_commited = false;

	public :
		inline rollbacker_t( L & action ) : m_action( action ) {}
		inline ~rollbacker_t() { if( !m_commited ) m_action(); }

		inline void commit() { m_commited = true; }
	};

template< typename RESULT, typename MAIN_ACTION, typename ROLLBACK_ACTION >
struct executor
	{
		static RESULT
		exec(
			MAIN_ACTION main_action,
			rollbacker_t< ROLLBACK_ACTION > & rollback )
			{
				auto r = main_action();
				rollback.commit();

				return r;
			}
	};

template< typename MAIN_ACTION, typename ROLLBACK_ACTION >
struct executor< void, MAIN_ACTION, ROLLBACK_ACTION >
	{
		static void
		exec( 
			MAIN_ACTION main_action,
			rollbacker_t< ROLLBACK_ACTION > & rollback )
			{
				main_action();
				rollback.commit();
			}
	};

} /* namespace rollback_on_exception_details */

/*!
 * \brief Helper function for do some action with rollback in the case of
 * an exception.
 *
 * Usage example:
 * \code
	// A reference to file_info must be stored into two containers.
	// Every insert operation can throw. Because of that file_info
	// must be removed from the first container if insertion into
	// the second container fails.
	all_files.insert( file_info );
	cpp_util_3::do_with_rollback_on_exception(
		[&]{ new_files.insert( file_info ); },
		[&]{ all.files.erase( file_info ); } );
 * \endcode
 *
 * Please note that do_with_rollback_on_exception() can return value
 * from \a main_action lambda:
 * \code
	std::size_t store_file_info( const file_info_t & file_info )
	{
		all_files.insert( file_info );
		return cpp_util_3::do_with_rollback_on_exception(
			[&]{
				new_files.insert( file_info );
				// return total count of file infos.
				return all_files.size();
			},
			[&]{ all.files.erase( file_info ); } );
	}
 * \endcode
 *
 * \tparam MAIN_ACTION type of lambda with main action.
 * \tparam ROLLBACK_ACTION type of lambda with rollback action.
 *
 * \since
 * v.2.7.2
 */
template< typename MAIN_ACTION, typename ROLLBACK_ACTION >
auto
do_with_rollback_on_exception(
	MAIN_ACTION main_action,
	ROLLBACK_ACTION rollback_action )
	-> decltype(main_action())
	{
		using result_type = decltype(main_action());

		using namespace rollback_on_exception_details;

		rollbacker_t< ROLLBACK_ACTION > rollbacker{ rollback_action };

		return executor< result_type, MAIN_ACTION, ROLLBACK_ACTION >::exec(
				main_action, rollbacker );
	}

} /* namespace cpp_util_3 */


/*!
 * \file
 * \brief outliving_reference_t and related stuff.
 *
 * \since
 * v.3.0.0
 */

#pragma once

#include <functional>

namespace cpp_util_3 {

/*!
 * \brief Helper class for indication of long-lived reference via its type.
 *
 * Sometimes it is necessary to store a reference to an object that lives
 * longer that a reference holder. For example:
 * \code
	class config {...};
	class config_consumer {
		config & cfg_;
	public :
		config_consumer(config & cfg) : cfg_(cfg) {...}
		...
	};

	void f() {
		config cfg = load_config(); // cfg outlives all other objects in f().
		config_consumer consumer(cfg); // It is safe to store a reference
			// for all lifetime of consumer object.
		...
	}
 * \endcode
 *
 * The problem there is: when we see <tt>consumer::consumer(cfg)</tt> we can
 * say is it safe to pass a reference to short-lived object to it or not.
 * Helper class outliving_reference_t can be used as indicator that
 * <tt>consumer::consumer</tt> expects a reference to long-lived object: 
 * \code
	class config_consumer {
		cpp_util_3::outliving_reference_t<config> cfg_;
	public :
		config_consumer(cpp_util_3::outliving_reference_t<config> cfg)
			: cfg_(cfg)
		{...}
		...
	};

	void f() {
		config cfg = load_config();
		// An explicit sign that cfg will outlive consumer object.
		config_consumer consumer(cpp_util_3::outliving_mutable(cfg));
		...
	}
 * \endcode
 *
 * If it is necessary to store a const reference to long-lived object
 * then outliving_reference_t<const T> and outliving_const() should be used:
 * \code
	class data_processor {
		cpp_util_3::outliving_reference_t<const config> cfg_;
	public :
		data_processor(cpp_util_3::outliving_reference_t<const config> cfg)
			: cfg_(cfg)
		{...}
		...
	};

	void f() {
		config cfg = load_config();
		config_consumer consumer(cpp_util_3::outliving_mutable(cfg));
		data_processor processor(cpp_util_3::outliving_const(cfg));
		...
	}
 * \endcode
 *
 * \note
 * Class outliving_reference_t is derived from std::reference_wrapper.
 * It means that almost all content of std::reference_wrapper is
 * available as part of outliving_reference_t interface.
 *
 * \attention
 * outliving_reference_t has no copy operator! It is CopyConstructible,
 * but not CopyAssingable class.
 *
 * \tparam T type for reference.
 *
 * \since
 * v.3.0.0
 */
template< typename T >
class outliving_reference_t : public std::reference_wrapper<T>
{
	using base_type = std::reference_wrapper<T>;
public :
	explicit outliving_reference_t(T & r) : base_type(r) {}

	template<typename U>
	explicit outliving_reference_t(U && r) : base_type(r) {}

	outliving_reference_t(T &&) = delete;
	outliving_reference_t(outliving_reference_t const & o)
		: base_type(o.get())
		{}

	template<typename U>
	outliving_reference_t(outliving_reference_t<U> const & o)
		: base_type(o.get())
		{}

	outliving_reference_t & operator=(outliving_reference_t const &o) = delete;
};

/*!
 * \brief Make outliving_reference wrapper for mutable reference.
 *
 * \sa outliving_reference_t
 *
 * \since
 * v.3.0.0
 */
template< typename T >
outliving_reference_t< T >
outliving_mutable( T & r ) { return outliving_reference_t<T>(r); }

/*!
 * \brief Make outliving_reference wrapper for const reference.
 *
 * \sa outliving_reference_t
 *
 * \since
 * v.3.0.0
 */
template< typename T >
outliving_reference_t< const T >
outliving_const( T const & r ) { return outliving_reference_t<const T>(r); }

} /* namespace cpp_util_3 */


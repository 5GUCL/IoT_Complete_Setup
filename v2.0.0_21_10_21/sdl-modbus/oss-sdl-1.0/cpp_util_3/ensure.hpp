/*!
 * \file
 * \brief Helper function 'ensure'
 * \since
 * v.2.7.2
 */

#pragma once

namespace cpp_util_3 {

/*!
 * \brief Helper for ensuring validity of some condition.
 *
 * Usage example:
 * \code
	auto r = mosquitto_subscribe( m_mosq.get(),
			&mid, topic_name.c_str(), qos_to_use );
	cpp_util_3::ensure< ex_t >(
			MOSQ_ERR_SUCCESS == r ||
			MOSQ_ERR_NO_CONN == r ||
			MOSQ_ERR_CONN_LOST == r,
			[&]{
				return tfm::format( "mosquitto_subscribe(%s, %d) failed",
					topic_name, qos_to_use );
			} );
 * \endcode
 *
 * \tparam EXCEPTION type of exception to be thrown.
 * \tparam EXPLANATION_LAMBDA lambda or functional object for making
 * explanation message.
 *
 * \since
 * v.2.7.2
 */
template< typename EXCEPTION, typename EXPLANATION_LAMBDA >
void
ensure( bool condition, EXPLANATION_LAMBDA explanation )
	{
		if( !condition )
			throw EXCEPTION{ explanation() };
	}

} /* namespace mosquitto_transport */


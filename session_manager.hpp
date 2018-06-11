//
// Created by xmh on 18-5-7.
//

#ifndef CINATRA_SESSION_UTILS_HPP
#define CINATRA_SESSION_UTILS_HPP

#include "session.hpp"
#include "request.hpp"

namespace cinatra {

namespace session_manager_detail {
static std::map< std::string, std::shared_ptr< session>> map_;
static std::mutex mtx_;
static int max_age_ = 10 * 60;
}

class session_manager {
public:
    session_manager( ) = delete;

    static std::shared_ptr< session > create_session( const std::string &name, std::size_t expire,
                                                      const std::string &path = "/", const std::string &domain = "" ) {
        uuids::uuid_random_generator uid{ };
        std::string uuid_str = uid( ).to_short_str( );
        auto s = std::make_shared< session >( name, uuid_str, expire, path, domain );

        {
            std::unique_lock< std::mutex > lock( session_manager_detail::mtx_ );
            session_manager_detail::map_.emplace( std::move( uuid_str ), s );
        }

        return s;
    }

    static std::shared_ptr< session > create_session( std::string_view host, const std::string &name,
                                                      std::time_t expire = -1, const std::string &path = "/" ) {
        auto pos = host.find( ":" );
        if ( pos != std::string_view::npos ) {
            host = host.substr( 0, pos );
        }

        return create_session( name, expire, path, std::string( host.data( ), host.length( )));
    }

    static std::weak_ptr< session > get_session( const std::string &id ) {
        std::unique_lock< std::mutex > lock( session_manager_detail::mtx_ );
        auto it = session_manager_detail::map_.find( id );
        return ( it != session_manager_detail::map_.end( )) ? it->second : nullptr;
    }

    static void del_session( const std::string &id ) {
        std::unique_lock< std::mutex > lock( session_manager_detail::mtx_ );
        auto it = session_manager_detail::map_.find( id );
        if ( it != session_manager_detail::map_.end( ))
            session_manager_detail::map_.erase( it );
    }

    static void check_expire( ) {
        if ( session_manager_detail::map_.empty( ))
            return;

        auto now = std::time( nullptr );
        std::unique_lock< std::mutex > lock( session_manager_detail::mtx_ );
        for ( auto it = session_manager_detail::map_.begin( ); it != session_manager_detail::map_.end( ); ) {
            if ( now - it->second->time_stamp( ) >= session_manager_detail::max_age_ ) {
                it = session_manager_detail::map_.erase( it );
            } else {
                ++it;
            }
        }
    }

    static void set_max_inactive_interval( int seconds ) {
        session_manager_detail::max_age_ = seconds;
    }

private:
    //static std::map<std::string, std::shared_ptr<session>> map_;
    //static std::mutex mtx_;
    //static int max_age_;
};

}
#endif //CINATRA_SESSION_UTILS_HPP

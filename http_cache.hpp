#pragma once

#include <string>
#include <string_view>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "use_asio.hpp"

namespace cinatra {
constexpr const size_t MAX_CACHE_SIZE = 100000;

namespace cache_detail {
    static std::unordered_map< std::string, std::vector< std::string>> cache_;
    static std::unordered_map< std::string, std::vector< std::string>>::iterator cur_it_ = cache_.begin( );
    static bool need_cache_ = false;
    static std::mutex mtx_;
    static std::unordered_set< std::string_view > skip_cache_;
}

class http_cache {
public:
    static void add( const std::string &key, const std::vector< std::string > &content ) {
        std::unique_lock< std::mutex > lock( cache_detail::mtx_ );

        if ( std::distance( cache_detail::cur_it_, cache_detail::cache_.end( )) > MAX_CACHE_SIZE ) {
            cache_detail::cur_it_ = cache_detail::cache_.begin( );
        }

        cache_detail::cur_it_ = cache_detail::cache_.emplace( key, content ).first;
    }

    static std::vector< std::string > get( const std::string &key ) {
        std::unique_lock< std::mutex > lock( cache_detail::mtx_ );
        auto it = cache_detail::cache_.find( key );
        return it == cache_detail::cache_.end( ) ? std::vector< std::string >{ } : it->second;
    }

    static bool empty( ) {
        return cache_detail::cache_.empty( );
    }

    static void update( const std::string &key ) {
        std::unique_lock< std::mutex > lock( cache_detail::mtx_ );
        auto it = cache_detail::cache_.find( key );
        if ( it != cache_detail::cache_.end( ))
            cache_detail::cache_.erase( it );
    }

    static void add_skip( std::string_view key ) {
        cache_detail::skip_cache_.emplace( key );
    }

    static void enable_cache( bool b ) {
        cache_detail::need_cache_ = b;
    }

    static bool need_cache( ) {
        return cache_detail::need_cache_;
    }

    static bool not_cache( std::string_view key ) {
        return cache_detail::skip_cache_.find( key ) != cache_detail::skip_cache_.end( );
    }


private:
    //static std::mutex mtx_;
    //static bool need_cache_;
    //static std::unordered_map< std::string, std::vector< std::string>> cache_;
    //static std::unordered_map< std::string, std::vector< std::string>>::iterator cur_it_;
    //static std::unordered_set< std::string_view > skip_cache_;
};

}
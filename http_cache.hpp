#pragma once

#include <string>
#include <string_view>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "use_asio.hpp"

namespace cinatra {
	constexpr const std::int32_t MAX_CACHE_SIZE = 100000;

	namespace cache_detail {
	static std::unordered_map< std::string, std::vector< std::string>> cache_;
	static std::unordered_map< std::string, std::vector< std::string>>::iterator cur_it_ = cache_.begin( );
	static bool need_cache_ = false;
	static std::mutex mtx_;
	static std::unordered_set< std::string_view > skip_cache_;
	static std::unordered_set<std::string_view> need_single_cache_;
	static std::time_t max_cache_age_=0;
	static std::unordered_map<std::string, std::time_t > cache_time_;
	}

	class http_cache {
	public:
		static void add(const std::string& key, const std::vector<std::string>& content) {
			std::unique_lock<std::mutex> lock(cache_detail::mtx_);
			
			if (std::distance(cache_detail::cur_it_, cache_detail::cache_.end()) > MAX_CACHE_SIZE) {
				cache_detail::cur_it_ = cache_detail::cache_.begin();
			}

			cache_detail::cur_it_ = cache_detail::cache_.emplace(key, content).first;
			cache_detail::cache_time_[key] = std::time(nullptr) + cache_detail::max_cache_age_;
		}

		static std::vector<std::string> get(const std::string& key) {
			std::unique_lock<std::mutex> lock(cache_detail::mtx_);
			auto time_it = cache_detail::cache_time_.find(key);
			auto it = cache_detail::cache_.find(key);
			auto now_time = std::time(nullptr);
			if(time_it != cache_detail::cache_time_.end() && time_it->second >= now_time){
				return it == cache_detail::cache_.end() ? std::vector<std::string>{} : it->second;
			}else{
				if(time_it != cache_detail::cache_time_.end() && it != cache_detail::cache_.end()){
					cache_detail::cur_it_ = cache_detail::cache_.erase(it);
				}
				return std::vector<std::string>{};
			}
		}

		static bool empty() {
			return cache_detail::cache_.empty();
		}

		static void update(const std::string& key) {
			std::unique_lock<std::mutex> lock(cache_detail::mtx_);
			auto it = cache_detail::cache_.find(key);
			if (it != cache_detail::cache_.end())
				cache_detail::cache_.erase(it);
		}

		static void add_skip(std::string_view key) {
			cache_detail::skip_cache_.emplace(key);
		}

		static void add_single_cache(std::string_view key)
		{
			cache_detail::need_single_cache_.emplace(key);
		}

		static void enable_cache(bool b) {
			cache_detail::need_cache_ = b;
		}

		static bool need_cache(std::string_view key) {
			if(cache_detail::need_cache_){
				return cache_detail::need_cache_;
			}else{
                return cache_detail::need_single_cache_.find(key)!= cache_detail::need_single_cache_.end();
			}
		}

		static bool not_cache(std::string_view key) {
			return cache_detail::skip_cache_.find(key) != cache_detail::skip_cache_.end();
		}

		static void set_cache_max_age(std::time_t seconds)
		{
			cache_detail::max_cache_age_ = seconds;
		}

		static std::time_t get_cache_max_age()
		{
			return cache_detail::max_cache_age_;
		}

	private:
		//static std::mutex mtx_;
		//static bool need_cache_;
		//static std::unordered_map<std::string, std::vector<std::string>> cache_;
		//static std::unordered_map<std::string, std::vector<std::string>>::iterator cur_it_;
		//static std::unordered_set<std::string_view> skip_cache_;
		//static std::unordered_set<std::string_view> need_single_cache_;
		//static std::time_t max_cache_age_;
		//static std::unordered_map<std::string, std::time_t > cache_time_;
	};

	//std::unordered_map<std::string, std::vector<std::string>> http_cache::cache_;
	//std::unordered_map<std::string, std::vector<std::string>>::iterator http_cache::cur_it_= http_cache::cache_.begin();
	//bool http_cache::need_cache_ = false;
	//std::mutex http_cache::mtx_;
	//std::unordered_set<std::string_view> http_cache::skip_cache_;
	//std::time_t http_cache::max_cache_age_ = 0;
	//std::unordered_map<std::string, std::time_t > http_cache::cache_time_;
	//std::unordered_set<std::string_view> http_cache::need_single_cache_;
}

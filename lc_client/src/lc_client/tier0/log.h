#pragma once

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include "spdlog/sinks/base_sink.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "lc_client/tier0/console/i_console.h"

namespace lecore {
    template<typename Mutex>
    class ConsoleSink : public spdlog::sinks::base_sink<Mutex>
    {
        public:
        ConsoleSink() {}
        void setConsole(IConsole* pConsole);
        protected:
            void sink_it_(const spdlog::details::log_msg& msg) override;
            void flush_() override;
            private:
            IConsole* m_pConsole = nullptr;
    };

    class Log {
        using ConsoleSinkMt = ConsoleSink<std::mutex>;
        using ConsoleSinkSt = ConsoleSink<spdlog::details::null_mutex>;
        public:
            static void init();
            static void setConsole(IConsole* pConsole);
            static const std::shared_ptr<spdlog::logger>& getCoreLogger() { return m_coreLogger; }
            static const std::shared_ptr<spdlog::logger>& getGameLogger() { return m_gameLogger; }
        private:
            static std::shared_ptr<spdlog::logger> m_coreLogger;
            static std::shared_ptr<spdlog::logger> m_gameLogger;
            static std::shared_ptr<ConsoleSinkMt> m_consoleSink;
    };
}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

// 0 = no logs, 1 = critical, 2 == error, 3 == warn, 4 == info, 5 == debug, 6 == trace
#define LE_LOG_LEVEL_TRACE (6)
#define LE_LOG_LEVEL_DEBUG (5)
#define LE_LOG_LEVEL_INFO (4)
#define LE_LOG_LEVEL_WARN (3)
#define LE_LOG_LEVEL_ERROR (2)
#define LE_LOG_LEVEL_CRITICAL (1)
#define LE_LOG_LEVEL_NO_LOG (0)

#ifndef LE_CORE_LOG_LEVEL
    #define LE_CORE_LOG_LEVEL LE_LOG_LEVEL_INFO
#endif

#ifndef LE_GAME_LOG_LEVEL
    #define LE_GAME_LOG_LEVEL LE_LOG_LEVEL_INFO
#endif

/*
 * Core log macros
 */
#if LE_CORE_LOG_LEVEL >= LE_LOG_LEVEL_TRACE
    #define LE_CORE_TRACE(...) SPDLOG_LOGGER_TRACE(::lecore::Log::getCoreLogger(), __VA_ARGS__)   //LE_CORE_TRACE(...) ::lecore::Log::getCoreLogger()->trace(__VA_ARGS__)
#else
    #define LE_CORE_TRACE(...) do {} while(0)
#endif

#if LE_CORE_LOG_LEVEL >= LE_LOG_LEVEL_DEBUG
    #define LE_CORE_DEBUG(...) SPDLOG_LOGGER_DEBUG(::lecore::Log::getCoreLogger(), __VA_ARGS__)
#else
    #define LE_CORE_DEBUG(...) do {} while(0)
#endif

#if LE_CORE_LOG_LEVEL >= LE_LOG_LEVEL_INFO
    #define LE_CORE_INFO(...) SPDLOG_LOGGER_INFO(::lecore::Log::getCoreLogger(), __VA_ARGS__)
#else
    #define LE_CORE_INFO(...) do {} while(0)
#endif

#if LE_CORE_LOG_LEVEL >= LE_LOG_LEVEL_WARN
    #define LE_CORE_WARN(...) SPDLOG_LOGGER_WARN(::lecore::Log::getCoreLogger(), __VA_ARGS__)
#else
    #define LE_CORE_WARN(...) do {} while(0)
#endif

#if LE_CORE_LOG_LEVEL >= LE_LOG_LEVEL_ERROR
    #define LE_CORE_ERROR(...) SPDLOG_LOGGER_ERROR(::lecore::Log::getCoreLogger(), __VA_ARGS__)
#else
    #define LE_CORE_ERROR(...) do {} while(0);
#endif

#if LE_CORE_LOG_LEVEL >= LE_LOG_LEVEL_CRITICAL
    #define LE_CORE_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(::lecore::Log::getCoreLogger(), __VA_ARGS__)
#else
    #define LE_CORE_CRITICAL(...) do {} while(0);
#endif

/*
 * Game log macros
 */
#if LE_GAME_LOG_LEVEL >= LE_LOG_LEVEL_TRACE
    #define LE_GAME_TRACE(...) SPDLOG_LOGGER_TRACE(::lecore::Log::getGameLogger(), __VA_ARGS__)
#else
    #define LE_GAME_TRACE(...) do {} while(0);
#endif

#if LE_GAME_LOG_LEVEL >= LE_LOG_LEVEL_DEBUG
    #define LE_GAME_DEBUG(...) SPDLOG_LOGGER_DEBUG(::lecore::Log::getGameLogger(), __VA_ARGS__)
#else
    #define LE_GAME_DEBUG(...) do {} while(0);
#endif

#if LE_GAME_LOG_LEVEL >= LE_LOG_LEVEL_INFO
    #define LE_GAME_INFO(...) SPDLOG_LOGGER_INFO(::lecore::Log::getGameLogger(), __VA_ARGS__)
#else
    #define LE_GAME_INFO(...) do {} while(0);
#endif

#if LE_GAME_LOG_LEVEL >= LE_LOG_LEVEL_WARN
    #define LE_GAME_WARN(...) SPDLOG_LOGGER_WARN(::lecore::Log::getGameLogger(), __VA_ARGS__)
#else
    #define LE_GAME_WARN(...) do {} while(0);
#endif

#if LE_GAME_LOG_LEVEL >= LE_LOG_LEVEL_ERROR
    #define LE_GAME_ERROR(...) SPDLOG_LOGGER_ERROR(::lecore::Log::getGameLogger(), __VA_ARGS__)
#else
    #define LE_GAME_ERROR(...) do {} while(0);
#endif

#if LE_GAME_LOG_LEVEL >= LE_LOG_LEVEL_CRITICAL
    #define LE_GAME_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(::lecore::Log::getGameLogger(), __VA_ARGS__)
#else
    #define LE_GAME_CRITICAL(...) do {} while(0);
#endif

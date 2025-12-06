#include "log.h"
#include "lc_client/tier0/console/i_console.h"


#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>



namespace lecore {
    std::shared_ptr<spdlog::logger> Log::m_coreLogger;
    std::shared_ptr<spdlog::logger> Log::m_gameLogger;
    std::shared_ptr<Log::ConsoleSinkMt> Log::m_consoleSink;

    void Log::init() {
        m_consoleSink = std::make_shared<ConsoleSinkMt>();

        std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		//logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Hazel.log", true));
		logSinks.emplace_back(m_consoleSink);

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		//logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		m_coreLogger = std::make_shared<spdlog::logger>("CORE", begin(logSinks), end(logSinks));
		spdlog::register_logger(m_coreLogger);
		m_coreLogger->set_level(spdlog::level::trace);
		m_coreLogger->flush_on(spdlog::level::trace);

		m_gameLogger = std::make_shared<spdlog::logger>("GAME", begin(logSinks), end(logSinks));
		spdlog::register_logger(m_gameLogger);
		m_gameLogger->set_level(spdlog::level::trace);
		m_gameLogger->flush_on(spdlog::level::trace);
    }

    void Log::setConsole(IConsole *pConsole) {
        m_consoleSink->setConsole(pConsole);
    }

    template<typename Mutex>
    void ConsoleSink<Mutex>::sink_it_(const spdlog::details::log_msg& msg)
    {
        if (m_pConsole == nullptr) {
            return;
        }
        // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
        // msg.payload (before v1.3.0: msg.raw) contains pre formatted log

        // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        auto str = fmt::to_string(formatted);
        switch (msg.level) {
            case SPDLOG_LEVEL_DEBUG:
                m_pConsole->devMessage(str);
                break;
            case SPDLOG_LEVEL_INFO:
                m_pConsole->message(str);
                break;
            case SPDLOG_LEVEL_WARN:
                m_pConsole->warn(str);
                break;
            case SPDLOG_LEVEL_ERROR:
                m_pConsole->warn(str);
                break;
            case SPDLOG_LEVEL_CRITICAL:
                m_pConsole->warn(str);
                break;
            default:
                break;
        }
    }

    template<typename Mutex>
    void ConsoleSink<Mutex>::flush_()
    {
        //std::cout << std::flush;
    }

    template<typename Mutex>
    void ConsoleSink<Mutex>::setConsole(IConsole* pConsole) {
        m_pConsole = pConsole;
    }
}

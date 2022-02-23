#pragma once

#include <iostream>
#include <fstream>
#include <ostream>
#include <filesystem>
#include <ctime>
#include <cstring>
#include <string>

namespace LuM {
    enum LogLevel {
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR
    };
    class Logger {
        public:
            Logger(Logger const&) = delete;
            operator=(Logger const&) = delete;
            static Logger& GetInstance() {
                static Logger instance;
                return instance;
            }
            /**
             * Writes to currently open instance
             * @param message
             */
            void Write(const char* message, LogLevel type) {
                if(!IsStreamOpen()) {
                    throw std::runtime_error("Attempted writing a log but no stream was open.");
                }
                std::string out;
                // Type
                switch(type) {
                    case LOG_DEBUG:
                        out.append("[DEBUG] ");
                        break;
                    case LOG_INFO:
                        out.append("[INFO] ");
                        break;
                    case LOG_WARNING:
                        out.append("[WARNING] ");
                        break;
                    case LOG_ERROR:
                        out.append("[ERROR] ");
                        break;
                    default:
                        out.append("[UNDEFINED] ");
                }
                // Timestamp
                //std::time_t time = std::time(nullptr);
                //out.append(std::asctime(std::localtime(&time)));
                // Actual message
                out.append(message);
                out.append("\n\n");
                // Write
                std::cout << out;
            }
        private:
            Logger() {
                if(!IsStreamOpen()) {
                    CreateInstance();
                }
            }
            ~Logger() {
                if(IsStreamOpen()) {
                    CloseInstance();
                }
            }
            bool IsStreamOpen() {
                return m_stream != nullptr && m_stream->is_open();
            }
            void CloseInstance() {
                if(!IsStreamOpen()) {
                    throw std::runtime_error("Attempted closing a log but no stream was open.");
                }
                m_stream->close();
                delete m_stream;
            }
            void CreateInstance() {
                if(IsStreamOpen()) {
                    std::cerr << "Attempted opening a log but a stream was already open" << std::endl;
                    return;
                }
                m_stream = new std::ofstream (GetPath());
            }
            const char* GetPath() const {
                return "log.txt";
            }
        private:
            std::ofstream* m_stream { nullptr };
    };
}
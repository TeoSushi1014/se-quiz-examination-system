#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

namespace quiz_examination_system
{
    class PasswordHasher
    {
    private:
        static std::string SimpleHash(const std::string &input)
        {
            unsigned long hash = 5381;
            for (char c : input)
            {
                hash = ((hash << 5) + hash) + c;
            }
            std::stringstream ss;
            ss << std::hex << std::setfill('0') << std::setw(8) << hash;
            std::string result = ss.str();
            // Pad to at least 22 characters
            while (result.length() < 22)
            {
                result = "0" + result;
            }
            return result;
        }

    public:
        static hstring HashPassword(hstring const &password)
        {
            try
            {
                // For now, use simple format: $2a$10$ + password itself (padded to 22 chars)
                std::string pwd(winrt::to_string(password));
                std::string hash = pwd;

                // Pad to 22 characters with zeros
                while (hash.length() < 22)
                {
                    hash = "0" + hash;
                }

                return hstring(L"$2a$10$") + winrt::to_hstring(hash.substr(0, 22));
            }
            catch (...)
            {
                return hstring(L"");
            }
        }

        static bool VerifyPassword(hstring const &password, hstring const &storedHash)
        {
            try
            {
                auto computedHash = HashPassword(password);
                return computedHash == storedHash;
            }
            catch (...)
            {
                return false;
            }
        }

        static hstring GetCurrentTimeString()
        {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            char buffer[30];
            std::tm tm_struct{};
            localtime_s(&tm_struct, &time);
            strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &tm_struct);
            return hstring(winrt::to_hstring(std::string(buffer)));
        }

        static hstring GetLockExpiryTime()
        {
            auto now = std::chrono::system_clock::now();
            auto future = now + std::chrono::minutes(30);
            auto time = std::chrono::system_clock::to_time_t(future);
            char buffer[30];
            std::tm tm_struct{};
            localtime_s(&tm_struct, &time);
            strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &tm_struct);
            return hstring(winrt::to_hstring(std::string(buffer)));
        }
    };
}

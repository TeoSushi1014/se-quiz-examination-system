#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <debugapi.h>

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
                std::string pwd(winrt::to_string(password));
                std::string hash = pwd;

                while (hash.length() < 22)
                {
                    hash = "0" + hash;
                }

                std::string result = "$2a$10$" + hash.substr(0, 22);

                return hstring(winrt::to_hstring(result));
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

                // Debug: Log comparison
                auto pwdStr = winrt::to_string(password);
                auto computedStr = winrt::to_string(computedHash);
                auto storedStr = winrt::to_string(storedHash);

                std::string debugMsg1 = "Password: " + pwdStr + "\n";
                std::string debugMsg2 = "Computed: " + computedStr + "\n";
                std::string debugMsg3 = "Stored:   " + storedStr + "\n";
                std::string debugMsg4 = "Match: " + std::string(computedHash == storedHash ? "YES" : "NO") + "\n\n";

                OutputDebugStringA(debugMsg1.c_str());
                OutputDebugStringA(debugMsg2.c_str());
                OutputDebugStringA(debugMsg3.c_str());
                OutputDebugStringA(debugMsg4.c_str());

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

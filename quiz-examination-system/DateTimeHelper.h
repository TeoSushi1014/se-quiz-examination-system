#pragma once

#include <winrt/Windows.Foundation.h>

namespace quiz_examination_system
{
    class DateTimeHelper
    {
    public:
        static winrt::hstring FormatTimeSpent(int seconds)
        {
            int minutes = seconds / 60;
            int secs = seconds % 60;

            wchar_t buffer[32];
            if (minutes > 0)
            {
                swprintf_s(buffer, L"%d mins", minutes);
            }
            else
            {
                swprintf_s(buffer, L"%d secs", secs);
            }
            return winrt::hstring(buffer);
        }

        static winrt::hstring FormatDateTime(winrt::hstring const &timestamp)
        {
            if (timestamp.empty())
            {
                return L"N/A";
            }

            try
            {
                std::wstring ts(timestamp);
                if (ts.length() >= 19)
                {
                    return winrt::hstring(ts.substr(0, 19));
                }
            }
            catch (...)
            {
            }

            return timestamp;
        }

        static winrt::hstring FormatShortDate(winrt::hstring const &timestamp)
        {
            if (timestamp.empty())
            {
                return L"N/A";
            }

            try
            {
                std::wstring ts(timestamp);
                if (ts.length() >= 10)
                {
                    return winrt::hstring(ts.substr(0, 10));
                }
            }
            catch (...)
            {
            }

            return timestamp;
        }
    };
}

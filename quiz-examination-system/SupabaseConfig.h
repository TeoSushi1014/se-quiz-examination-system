#pragma once

#include <winrt/Windows.Foundation.h>

namespace quiz_examination_system
{
    class SupabaseConfig
    {
    public:
        static constexpr wchar_t const *PROJECT_URL = L"https://tuciofxdzzrzwzqsltps.supabase.co";
        static constexpr wchar_t const *ANON_KEY = L"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI";

        static winrt::hstring GetRestUrl() { return winrt::hstring(PROJECT_URL) + L"/rest/v1"; }
        static winrt::hstring GetAuthorizationHeader() { return winrt::hstring(L"Bearer ") + ANON_KEY; }

        static winrt::hstring GetRestEndpoint(winrt::hstring const &path)
        {
            return GetRestUrl() + L"/" + path;
        }

        static winrt::hstring GetRpcEndpoint(winrt::hstring const &functionName)
        {
            return GetRestUrl() + L"/rpc/" + functionName;
        }
    };
}

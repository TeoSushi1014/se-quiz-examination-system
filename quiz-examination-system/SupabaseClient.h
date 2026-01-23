#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Data.Json.h>
#include <functional>
#include <memory>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Data::Json;

namespace quiz_examination_system
{
    class SupabaseClient
    {
    public:
        SupabaseClient();

        void Login(hstring const &username, hstring const &password);
        void ChangePassword(hstring const &username, hstring const &currentPassword, hstring const &newPassword);
        void CreateUser(hstring const &username, hstring const &password, hstring const &role, hstring const &createdBy);
        void ResetPassword(hstring const &userId, hstring const &newPassword);
        void ToggleUserStatus(hstring const &userId, hstring const &newStatus);
        void GetAllUsers();

        bool IsConnected() const { return m_connected; }
        hstring GetLastError() const { return m_lastError; }

        std::function<void(hstring, hstring, hstring)> OnLoginSuccess;
        std::function<void(hstring)> OnLoginFailed;
        std::function<void(hstring)> OnPasswordChanged;
        std::function<void(hstring)> OnPasswordChangeFailed;
        std::function<void(hstring)> OnUserCreated;
        std::function<void(hstring)> OnUserCreationFailed;
        std::function<void(hstring)> OnUsersFetched;
        std::function<void(hstring)> OnUserActionSuccess;
        std::function<void(hstring)> OnUserActionFailed;

    private:
        HttpClient m_httpClient;
        hstring m_projectUrl{L"https://tuciofxdzzrzwzqsltps.supabase.co"};
        hstring m_anonKey{L"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI"};
        bool m_connected{true};
        hstring m_lastError;
    };
}

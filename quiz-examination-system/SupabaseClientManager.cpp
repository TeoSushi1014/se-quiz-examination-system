#include "pch.h"
#include "SupabaseClientManager.h"
#include <Windows.h>
#include <string>

using namespace winrt;

namespace quiz_examination_system
{
    const wchar_t *REG_KEY_PATH = L"Software\\QuizExaminationSystem";

    SupabaseClientManager &SupabaseClientManager::GetInstance()
    {
        static SupabaseClientManager instance;
        return instance;
    }

    SupabaseClientManager::SupabaseClientManager()
    {
        m_client = std::make_unique<SupabaseClient>();
        LoadSessionFromSettings();
    }

    SupabaseClient *SupabaseClientManager::GetClient()
    {
        return m_client.get();
    }

    void SupabaseClientManager::SaveSession(hstring const &userId, hstring const &username, hstring const &role)
    {
        m_userId = userId;
        m_username = username;
        m_role = role;

        HKEY hKey;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY_PATH, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
        {
            std::wstring userIdStr = userId.c_str();
            std::wstring usernameStr = username.c_str();
            std::wstring roleStr = role.c_str();

            RegSetValueExW(hKey, L"userId", 0, REG_SZ, (BYTE *)userIdStr.c_str(), (DWORD)((userIdStr.length() + 1) * sizeof(wchar_t)));
            RegSetValueExW(hKey, L"username", 0, REG_SZ, (BYTE *)usernameStr.c_str(), (DWORD)((usernameStr.length() + 1) * sizeof(wchar_t)));
            RegSetValueExW(hKey, L"role", 0, REG_SZ, (BYTE *)roleStr.c_str(), (DWORD)((roleStr.length() + 1) * sizeof(wchar_t)));

            RegCloseKey(hKey);
        }
    }

    void SupabaseClientManager::ClearSession()
    {
        m_userId = L"";
        m_username = L"";
        m_role = L"";

        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY_PATH, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
        {
            RegDeleteValueW(hKey, L"userId");
            RegDeleteValueW(hKey, L"username");
            RegDeleteValueW(hKey, L"role");
            RegCloseKey(hKey);
        }
    }

    void SupabaseClientManager::LoadSessionFromSettings()
    {
        try
        {
            HKEY hKey;
            if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                wchar_t buffer[256];
                DWORD bufferSize = sizeof(buffer);

                if (RegQueryValueExW(hKey, L"userId", NULL, NULL, (BYTE *)buffer, &bufferSize) == ERROR_SUCCESS)
                {
                    m_userId = hstring(buffer);
                }

                bufferSize = sizeof(buffer);
                if (RegQueryValueExW(hKey, L"username", NULL, NULL, (BYTE *)buffer, &bufferSize) == ERROR_SUCCESS)
                {
                    m_username = hstring(buffer);
                }

                bufferSize = sizeof(buffer);
                if (RegQueryValueExW(hKey, L"role", NULL, NULL, (BYTE *)buffer, &bufferSize) == ERROR_SUCCESS)
                {
                    m_role = hstring(buffer);
                }

                RegCloseKey(hKey);
            }
        }
        catch (...)
        {
        }
    }

    bool SupabaseClientManager::HasActiveSession() const
    {
        return !m_userId.empty();
    }

    hstring SupabaseClientManager::GetUserId() const
    {
        return m_userId;
    }

    hstring SupabaseClientManager::GetUsername() const
    {
        return m_username;
    }

    hstring SupabaseClientManager::GetRole() const
    {
        return m_role;
    }
}

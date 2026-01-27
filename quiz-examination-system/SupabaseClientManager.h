#pragma once

#include "SupabaseClient.h"
#include <memory>

namespace quiz_examination_system
{
    class SupabaseClientManager
    {
    public:
        static SupabaseClientManager &GetInstance();

        SupabaseClient *GetClient();

        void SaveSession(hstring const &userId, hstring const &username, hstring const &role);
        void ClearSession();

        bool HasActiveSession() const;
        hstring GetUserId() const;
        hstring GetUsername() const;
        hstring GetRole() const;

        SupabaseClientManager(SupabaseClientManager const &) = delete;
        SupabaseClientManager &operator=(SupabaseClientManager const &) = delete;

    private:
        SupabaseClientManager();
        void LoadSessionFromSettings();

        std::unique_ptr<SupabaseClient> m_client;
        hstring m_userId;
        hstring m_username;
        hstring m_role;
    };
}

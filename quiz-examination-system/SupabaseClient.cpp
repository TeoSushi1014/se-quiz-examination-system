#include "pch.h"
#include "SupabaseClient.h"
#include <winrt/Windows.Web.Http.Filters.h>
#include <winrt/Microsoft.UI.Dispatching.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Filters;
using namespace Windows::Data::Json;
using namespace Microsoft::UI::Dispatching;

namespace quiz_examination_system
{
    SupabaseClient::SupabaseClient()
    {
        HttpBaseProtocolFilter filter;
        m_httpClient = HttpClient(filter);
    }

    void SupabaseClient::Login(hstring const &username, hstring const &password)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            Uri uri(m_projectUrl + L"/rest/v1/users?select=id,username,password_hash,role&username=eq." + username);
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            m_httpClient.SendRequestAsync(request).Completed([this, username, password, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, username, password, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([=]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    JsonArray users = JsonArray::Parse(content);
                                    
                                    if (users.Size() > 0)
                                    {
                                        auto user = users.GetObjectAt(0);
                                        auto storedHash = user.GetNamedString(L"password_hash");
                                        auto role = user.GetNamedString(L"role", L"STUDENT");
                                        
                                        if (storedHash == password)
                                        {
                                            hstring displayRole = (role == L"ADMIN") ? L"Administrator" : 
                                                                 (role == L"TEACHER") ? L"Lecturer" : L"Student";
                                            if (OnLoginSuccess)
                                            {
                                                OnLoginSuccess(username, displayRole, role);
                                            }
                                        }
                                        else
                                        {
                                            if (OnLoginFailed)
                                            {
                                                OnLoginFailed(L"Invalid credentials");
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (OnLoginFailed)
                                        {
                                            OnLoginFailed(L"User not found");
                                        }
                                    }
                                }
                                catch (...)
                                {
                                    if (OnLoginFailed)
                                    {
                                        OnLoginFailed(L"Parse error");
                                    }
                                }
                            });
                        });
                    }
                    else
                    {
                        auto statusCode = response.StatusCode();
                        response.Content().ReadAsStringAsync().Completed([=](auto const &errOp, auto)
                        {
                            dispatcher.TryEnqueue([=]()
                            {
                                if (OnLoginFailed)
                                {
                                    hstring body;
                                    try { body = errOp.GetResults(); } catch (...) { body = L""; }
                                    OnLoginFailed(hstring(L"API error: ") + to_hstring(static_cast<int>(statusCode)) + hstring(L" ") + body);
                                }
                            });
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([=]()
                    {
                        if (OnLoginFailed)
                        {
                            OnLoginFailed(L"Request error");
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnLoginFailed)
            {
                OnLoginFailed(L"Connection error");
            }
        }
    }

    void SupabaseClient::ChangePassword(hstring const &username, hstring const &currentPassword, hstring const &newPassword)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            Uri uri(m_projectUrl + L"/rest/v1/users?select=id,username,password_hash,role&username=eq." + username);
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            m_httpClient.SendRequestAsync(request).Completed([this, currentPassword, newPassword, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, currentPassword, newPassword, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([=]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    JsonArray users = JsonArray::Parse(content);
                                    
                                    if (users.Size() > 0)
                                    {
                                        auto user = users.GetObjectAt(0);
                                        auto storedHash = user.GetNamedString(L"password_hash");
                                        auto userId = user.GetNamedString(L"id");
                                        
                                        if (storedHash == currentPassword)
                                        {
                                            JsonObject updateData;
                                            updateData.Insert(L"password_hash", JsonValue::CreateStringValue(newPassword));
                                            
                                            Uri updateUri(m_projectUrl + L"/rest/v1/users?id=eq." + userId);
                                            HttpRequestMessage updateRequest(HttpMethod::Patch(), updateUri);
                                            updateRequest.Headers().Insert(L"apikey", m_anonKey);
                                            updateRequest.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
                                            updateRequest.Headers().Insert(L"Content-Type", L"application/json");
                                            updateRequest.Content(HttpStringContent(updateData.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

                                            m_httpClient.SendRequestAsync(updateRequest).Completed([=](auto const &updateOp, auto)
                                            {
                                                dispatcher.TryEnqueue([=]()
                                                {
                                                    try
                                                    {
                                                        auto updateResponse = updateOp.GetResults();
                                                        if (updateResponse.StatusCode() == HttpStatusCode::NoContent || updateResponse.StatusCode() == HttpStatusCode::Ok)
                                                        {
                                                            if (OnPasswordChanged)
                                                            {
                                                                OnPasswordChanged(L"Password updated");
                                                            }
                                                        }
                                                        else
                                                        {
                                                            if (OnPasswordChangeFailed)
                                                            {
                                                                OnPasswordChangeFailed(L"Update failed");
                                                            }
                                                        }
                                                    }
                                                    catch (...)
                                                    {
                                                        if (OnPasswordChangeFailed)
                                                        {
                                                            OnPasswordChangeFailed(L"Update error");
                                                        }
                                                    }
                                                });
                                            });
                                        }
                                        else
                                        {
                                            if (OnPasswordChangeFailed)
                                            {
                                                OnPasswordChangeFailed(L"Current password is incorrect");
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (OnPasswordChangeFailed)
                                        {
                                            OnPasswordChangeFailed(L"User not found");
                                        }
                                    }
                                }
                                catch (...)
                                {
                                    if (OnPasswordChangeFailed)
                                    {
                                        OnPasswordChangeFailed(L"Parse error");
                                    }
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([=]()
                        {
                            if (OnPasswordChangeFailed)
                            {
                                OnPasswordChangeFailed(L"Database error");
                            }
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([=]()
                    {
                        if (OnPasswordChangeFailed)
                        {
                            OnPasswordChangeFailed(L"Request error");
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnPasswordChangeFailed)
            {
                OnPasswordChangeFailed(L"Connection error");
            }
        }
    }

    void SupabaseClient::CreateUser(hstring const &username, hstring const &password, hstring const &role, hstring const &createdBy)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            JsonObject newUser;
            newUser.Insert(L"username", JsonValue::CreateStringValue(username));
            newUser.Insert(L"password_hash", JsonValue::CreateStringValue(password));
            newUser.Insert(L"role", JsonValue::CreateStringValue(role));
            newUser.Insert(L"status", JsonValue::CreateStringValue(L"active"));
            newUser.Insert(L"created_by", JsonValue::CreateStringValue(createdBy));

            Uri uri(m_projectUrl + L"/rest/v1/users");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Content-Type", L"application/json");
            request.Content(HttpStringContent(newUser.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    dispatcher.TryEnqueue([=]()
                    {
                        if (response.StatusCode() == HttpStatusCode::Created || response.StatusCode() == HttpStatusCode::Ok)
                        {
                            if (OnUserCreated)
                                OnUserCreated(L"User created successfully");
                        }
                        else
                        {
                            if (OnUserCreationFailed)
                                OnUserCreationFailed(L"Failed to create user");
                        }
                    });
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([=]()
                    {
                        if (OnUserCreationFailed)
                            OnUserCreationFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnUserCreationFailed)
                OnUserCreationFailed(L"Connection error");
        }
    }

    void SupabaseClient::ResetPassword(hstring const &userId, hstring const &newPassword)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            JsonObject updateData;
            updateData.Insert(L"password_hash", JsonValue::CreateStringValue(newPassword));

            Uri uri(m_projectUrl + L"/rest/v1/users?id=eq." + userId);
            HttpRequestMessage request(HttpMethod::Patch(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Content-Type", L"application/json");
            request.Content(HttpStringContent(updateData.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    dispatcher.TryEnqueue([=]()
                    {
                        if (response.StatusCode() == HttpStatusCode::NoContent || response.StatusCode() == HttpStatusCode::Ok)
                        {
                            if (OnUserActionSuccess)
                                OnUserActionSuccess(L"Password reset successfully");
                        }
                        else
                        {
                            if (OnUserActionFailed)
                                OnUserActionFailed(L"Failed to reset password");
                        }
                    });
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([=]()
                    {
                        if (OnUserActionFailed)
                            OnUserActionFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnUserActionFailed)
                OnUserActionFailed(L"Connection error");
        }
    }

    void SupabaseClient::ToggleUserStatus(hstring const &userId, hstring const &newStatus)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            JsonObject updateData;
            updateData.Insert(L"status", JsonValue::CreateStringValue(newStatus));

            Uri uri(m_projectUrl + L"/rest/v1/users?id=eq." + userId);
            HttpRequestMessage request(HttpMethod::Patch(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Content-Type", L"application/json");
            request.Content(HttpStringContent(updateData.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    dispatcher.TryEnqueue([=]()
                    {
                        if (response.StatusCode() == HttpStatusCode::NoContent || response.StatusCode() == HttpStatusCode::Ok)
                        {
                            if (OnUserActionSuccess)
                                OnUserActionSuccess(L"User status updated");
                        }
                        else
                        {
                            if (OnUserActionFailed)
                                OnUserActionFailed(L"Failed to update user status");
                        }
                    });
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([=]()
                    {
                        if (OnUserActionFailed)
                            OnUserActionFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnUserActionFailed)
                OnUserActionFailed(L"Connection error");
        }
    }

    void SupabaseClient::GetAllUsers()
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            Uri uri(m_projectUrl + L"/rest/v1/users?select=id,username,role,status");
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([=]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    if (OnUsersFetched)
                                        OnUsersFetched(content);
                                }
                                catch (...)
                                {
                                    if (OnUserActionFailed)
                                        OnUserActionFailed(L"Parse error");
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([=]()
                        {
                            if (OnUserActionFailed)
                                OnUserActionFailed(L"Failed to fetch users");
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([=]()
                    {
                        if (OnUserActionFailed)
                            OnUserActionFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnUserActionFailed)
                OnUserActionFailed(L"Connection error");
        }
    }
}

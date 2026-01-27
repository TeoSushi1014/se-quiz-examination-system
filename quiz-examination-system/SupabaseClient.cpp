#include "pch.h"
#include "SupabaseClient.h"
#include "PasswordHasher.h"
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

            // Step 1: Check account status (locked, inactive, or active)
            JsonObject statusParams;
            statusParams.Insert(L"input_username", JsonValue::CreateStringValue(username));
            Uri statusUri(m_projectUrl + L"/rest/v1/rpc/check_account_status");
            HttpRequestMessage statusReq(HttpMethod::Post(), statusUri);
            statusReq.Headers().Insert(L"apikey", m_anonKey);
            statusReq.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            statusReq.Content(HttpStringContent(statusParams.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto statusOp = m_httpClient.SendRequestAsync(statusReq);
            statusOp.Completed([this, username, password, dispatcher](auto op, auto status)
                               {
                if (status == AsyncStatus::Completed)
                {
                    try
                    {
                        auto statusResponse = op.GetResults();
                        statusResponse.Content().ReadAsStringAsync().Completed([this, username, password, dispatcher](auto readOp, auto readStatus)
                                                                               {
                            UNREFERENCED_PARAMETER(readStatus);
                            dispatcher.TryEnqueue([this, username, password, dispatcher, readOp]()
                                                                                                       {
                                try
                                {
                                    auto statusContent = readOp.GetResults();
                                    auto statusObj = JsonObject::Parse(statusContent);
                                    auto checkStatus = statusObj.GetNamedString(L"status", L"");
                                        
                                    if (checkStatus == L"user_not_found")
                                    {
                                        if (OnLoginFailed)
                                        {
                                            OnLoginFailed(L"User not found");
                                        }
                                        return;
                                    }
                                    else if (checkStatus == L"locked")
                                    {
                                        auto minutesRemaining = static_cast<int>(statusObj.GetNamedNumber(L"minutes_remaining", 30));
                                        hstring message = hstring(L"Account is locked. Try again in ") + to_hstring(minutesRemaining) + L" minutes.";
                                        if (OnLoginFailed)
                                        {
                                            OnLoginFailed(message);
                                        }
                                        return;
                                    }
                                    else if (checkStatus == L"inactive")
                                    {
                                        if (OnLoginFailed)
                                        {
                                            OnLoginFailed(L"Account is inactive");
                                        }
                                        return;
                                    }
                                    
                                    // Step 2: Account is active, proceed with login
                                    Uri uri(m_projectUrl + L"/rest/v1/users?select=id,username,password_hash,role,failed_login_count,locked_until&username=eq." + username);
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
                                                    dispatcher.TryEnqueue([this, username, password, dispatcher, readOp]()
                                                    {
                                                        try
                                                        {
                                                            auto content = readOp.GetResults();
                                                            JsonArray users = JsonArray::Parse(content);
                                                            
                                                            if (users.Size() > 0)
                                                            {
                                                                auto user = users.GetObjectAt(0);
                                                                auto userId = user.GetNamedString(L"id");
                                                                auto storedHash = user.GetNamedString(L"password_hash");
                                                                auto role = user.GetNamedString(L"role", L"STUDENT");

                                                                if (PasswordHasher::VerifyPassword(password, storedHash))
                                                                {
                                                                    // Password correct - reset failed login count via RPC
                                                                    JsonObject resetParams;
                                                                    resetParams.Insert(L"input_username", JsonValue::CreateStringValue(username));
                                                                    Uri resetUri(m_projectUrl + L"/rest/v1/rpc/handle_login_success");
                                                                    HttpRequestMessage resetReq(HttpMethod::Post(), resetUri);
                                                                    resetReq.Headers().Insert(L"apikey", m_anonKey);
                                                                    resetReq.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
                                                                    resetReq.Content(HttpStringContent(resetParams.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));
                                                                    
                                                                    m_httpClient.SendRequestAsync(resetReq);
                                                                    
                                                                    hstring displayRole = (role == L"ADMIN") ? L"Administrator" : 
                                                                                         (role == L"TEACHER") ? L"Lecturer" : L"Student";
                                                                    if (OnLoginSuccess)
                                                                    {
                                                                        OnLoginSuccess(username, displayRole, role, userId);
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    // Password wrong - increment failed login count via RPC
                                                                    JsonObject rpcParams;
                                                                    rpcParams.Insert(L"input_username", JsonValue::CreateStringValue(username));
                                                                    Uri rpcUri(m_projectUrl + L"/rest/v1/rpc/handle_login_failure");
                                                                    HttpRequestMessage rpcReq(HttpMethod::Post(), rpcUri);
                                                                    rpcReq.Headers().Insert(L"apikey", m_anonKey);
                                                                    rpcReq.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
                                                                    rpcReq.Content(HttpStringContent(rpcParams.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));
                                                                    
                                                                    OutputDebugStringW(L"[DEBUG] Calling RPC: handle_login_failure\n");
                                                                    
                                                                    auto rpcOp = m_httpClient.SendRequestAsync(rpcReq);
                                                                    rpcOp.Completed([this, dispatcher](auto op, auto status)
                                                                    {
                                                                        if (status == AsyncStatus::Completed)
                                                                        {
                                                                            OutputDebugStringW(L"[DEBUG] RPC response received\n");
                                                                            try
                                                                            {
                                                                                auto rpcResponse = op.GetResults();
                                                                                OutputDebugStringW((hstring(L"[DEBUG] RPC Status Code: ") + to_hstring(static_cast<int>(rpcResponse.StatusCode())) + L"\n").c_str());
                                                                                rpcResponse.Content().ReadAsStringAsync().Completed([this, dispatcher](auto readOp, auto readStatus)
                                                                                {
                                                                                    UNREFERENCED_PARAMETER(readStatus);
                                                                                    dispatcher.TryEnqueue([this, dispatcher, readOp]()
                                                                                    {
                                                                                        try
                                                                                        {
                                                                                            auto rpcContent = readOp.GetResults();
                                                                                            OutputDebugStringW((hstring(L"[DEBUG] RPC response body: ") + rpcContent + L"\n").c_str());
                                                                                            auto resultObj = JsonObject::Parse(rpcContent);
                                                                                            auto rpcStatus = resultObj.GetNamedString(L"status", L"");
                                                                                            OutputDebugStringW((hstring(L"[DEBUG] RPC status field: ") + rpcStatus + L"\n").c_str());
                                                                                            
                                                                                            if (rpcStatus == L"locked")
                                                                                            {
                                                                                                auto message = resultObj.GetNamedString(L"message", L"Account locked");
                                                                                                if (OnLoginFailed)
                                                                                                {
                                                                                                    OnLoginFailed(message);
                                                                                                }
                                                                                            }
                                                                                            else if (rpcStatus == L"failed")
                                                                                            {
                                                                                                int attemptsLeft = static_cast<int>(resultObj.GetNamedNumber(L"attempts_left", 4));
                                                                                                hstring message = hstring(L"Invalid credentials. Attempts left: ") + to_hstring(attemptsLeft);
                                                                                                if (OnLoginFailed)
                                                                                                {
                                                                                                    OnLoginFailed(message);
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            {
                                                                                                if (OnLoginFailed)
                                                                                                {
                                                                                                    OnLoginFailed(L"Invalid credentials. Please check your username and password.");
                                                                                                }
                                                                                            }
                                                                                        }
                                                                                        catch (...)
                                                                                        {
                                                                                            if (OnLoginFailed)
                                                                                            {
                                                                                                OnLoginFailed(L"Invalid credentials");
                                                                                            }
                                                                                        }
                                                                                    });
                                                                                });
                                                                            }
                                                                            catch (...)
                                                                            {
                                                                                OutputDebugStringW(L"[DEBUG] RPC response parsing error\n");
                                                                                dispatcher.TryEnqueue([this]()
                                                                                {
                                                                                    if (OnLoginFailed)
                                                                                    {
                                                                                        OnLoginFailed(L"Invalid credentials");
                                                                                    }
                                                                                });
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            OutputDebugStringW((hstring(L"[DEBUG] RPC async failed with status: ") + to_hstring(static_cast<int>(status)) + L"\n").c_str());
                                                                            dispatcher.TryEnqueue([this]()
                                                                            {
                                                                                if (OnLoginFailed)
                                                                                {
                                                                                    OnLoginFailed(L"Login failed");
                                                                                }
                                                                            });
                                                                        }
                                                                    });
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
                                                dispatcher.TryEnqueue([this, response]()
                                                {
                                                    if (OnLoginFailed)
                                                    {
                                                        OnLoginFailed(hstring(L"API error: ") + to_hstring(static_cast<int>(response.StatusCode())));
                                                    }
                                                });
                                            }
                                        }
                                        catch (...)
                                        {
                                            dispatcher.TryEnqueue([this]()
                                            {
                                                if (OnLoginFailed)
                                                {
                                                    OnLoginFailed(L"Request error");
                                                }
                                            });
                                        }
                                    });
                }  
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnLoginFailed)
                        {
                            OnLoginFailed(L"Status check error");
                        }
                    });
                }
            });
                    });
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                                          {
                            if (OnLoginFailed)
                            {
                                OnLoginFailed(L"Status check failed");
                            } });
                }
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
            if (newPassword.size() > 72)
            {
                auto dispatcher = DispatcherQueue::GetForCurrentThread();
                dispatcher.TryEnqueue([this]()
                                      {
                    if (OnPasswordChangeFailed)
                    {
                        OnPasswordChangeFailed(L"Password must not exceed 72 bytes");
                    } });
                return;
            }

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
            if (password.size() > 72)
            {
                auto dispatcher = DispatcherQueue::GetForCurrentThread();
                dispatcher.TryEnqueue([this]()
                                      {
                    if (OnUserCreationFailed)
                        OnUserCreationFailed(L"Password must not exceed 72 bytes"); });
                return;
            }

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
            if (m_currentUserRole != L"ADMIN")
            {
                auto dispatcher = DispatcherQueue::GetForCurrentThread();
                dispatcher.TryEnqueue([this]()
                                      {
                    if (OnUserActionFailed)
                        OnUserActionFailed(L"Permission denied: Only administrators can reset passwords"); });
                return;
            }

            if (newPassword.size() > 72)
            {
                auto dispatcher = DispatcherQueue::GetForCurrentThread();
                dispatcher.TryEnqueue([this]()
                                      {
                    if (OnUserActionFailed)
                        OnUserActionFailed(L"Password must not exceed 72 bytes"); });
                return;
            }

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
            if (m_currentUserRole != L"ADMIN")
            {
                auto dispatcher = DispatcherQueue::GetForCurrentThread();
                dispatcher.TryEnqueue([this]()
                                      {
                    if (OnUserActionFailed)
                        OnUserActionFailed(L"Permission denied: Only administrators can change user status"); });
                return;
            }

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

    void SupabaseClient::GetQuizzes(hstring const &createdBy)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            Uri uri(m_projectUrl + L"/rest/v1/quizzes?select=id,title,time_limit_minutes,total_points,created_by&created_by=eq." + createdBy);
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
                                    if (OnQuizzesFetched)
                                        OnQuizzesFetched(content);
                                }
                                catch (...)
                                {
                                    if (OnQuizCreationFailed)
                                        OnQuizCreationFailed(L"Parse error");
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([=]()
                        {
                            if (OnQuizCreationFailed)
                                OnQuizCreationFailed(L"Failed to fetch quizzes");
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([=]()
                    {
                        if (OnQuizCreationFailed)
                            OnQuizCreationFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizCreationFailed)
                OnQuizCreationFailed(L"Connection error");
        }
    }

    void SupabaseClient::GetQuestions(hstring const &createdBy)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            Uri uri(m_projectUrl + L"/rest/v1/questions?select=id,question_text,difficulty_level,created_by&created_by=eq." + createdBy);
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
                                    if (OnQuestionsFetched)
                                        OnQuestionsFetched(content);
                                }
                                catch (...)
                                {
                                    if (OnQuizCreationFailed)
                                        OnQuizCreationFailed(L"Parse error");
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([=]()
                        {
                            if (OnQuizCreationFailed)
                                OnQuizCreationFailed(L"Failed to fetch questions");
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([=]()
                    {
                        if (OnQuizCreationFailed)
                            OnQuizCreationFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizCreationFailed)
                OnQuizCreationFailed(L"Connection error");
        }
    }

    void SupabaseClient::CreateQuiz(hstring const &quizId, hstring const &title, int timeLimit, int totalPoints, hstring const &createdBy)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            JsonObject quizData;
            quizData.Insert(L"id", JsonValue::CreateStringValue(quizId));
            quizData.Insert(L"title", JsonValue::CreateStringValue(title));
            quizData.Insert(L"time_limit_minutes", JsonValue::CreateNumberValue(timeLimit));
            quizData.Insert(L"total_points", JsonValue::CreateNumberValue(totalPoints));
            quizData.Insert(L"created_by", JsonValue::CreateStringValue(createdBy));
            quizData.Insert(L"shuffle_questions", JsonValue::CreateBooleanValue(false));
            quizData.Insert(L"shuffle_answers", JsonValue::CreateBooleanValue(false));

            Uri uri(m_projectUrl + L"/rest/v1/quizzes");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Content-Type", L"application/json");
            request.Headers().Insert(L"Prefer", L"return=representation");
            request.Content(HttpStringContent(quizData.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    dispatcher.TryEnqueue([this, response]()
                    {
                        if (response.StatusCode() == HttpStatusCode::Created || response.StatusCode() == HttpStatusCode::Ok)
                        {
                            if (OnQuizCreated)
                                OnQuizCreated(L"Quiz created successfully");
                        }
                        else
                        {
                            if (OnQuizCreationFailed)
                                OnQuizCreationFailed(L"Failed to create quiz");
                        }
                    });
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuizCreationFailed)
                            OnQuizCreationFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizCreationFailed)
                OnQuizCreationFailed(L"Connection error");
        }
    }

    void SupabaseClient::AddQuestionsToQuiz(hstring const &quizId, hstring const &questionId, int points)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            JsonObject questionData;
            questionData.Insert(L"quiz_id", JsonValue::CreateStringValue(quizId));
            questionData.Insert(L"question_id", JsonValue::CreateStringValue(questionId));
            questionData.Insert(L"points", JsonValue::CreateNumberValue(points));

            Uri uri(m_projectUrl + L"/rest/v1/quiz_questions");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Content-Type", L"application/json");
            request.Content(HttpStringContent(questionData.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    dispatcher.TryEnqueue([this, response]()
                    {
                        if (response.StatusCode() == HttpStatusCode::Created || response.StatusCode() == HttpStatusCode::Ok)
                        {
                            if (OnQuizCreated)
                                OnQuizCreated(L"Question added to quiz");
                        }
                        else
                        {
                            if (OnQuizCreationFailed)
                                OnQuizCreationFailed(L"Failed to add question");
                        }
                    });
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuizCreationFailed)
                            OnQuizCreationFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizCreationFailed)
                OnQuizCreationFailed(L"Connection error");
        }
    }

    void SupabaseClient::CreateQuestion(hstring const &questionId, hstring const &questionText, hstring const &optionA, hstring const &optionB, hstring const &optionC, hstring const &optionD, hstring const &correctOption, hstring const &difficulty, hstring const &createdBy)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            JsonObject questionData;
            questionData.Insert(L"id", JsonValue::CreateStringValue(questionId));
            questionData.Insert(L"question_text", JsonValue::CreateStringValue(questionText));
            questionData.Insert(L"option_a", JsonValue::CreateStringValue(optionA));
            questionData.Insert(L"option_b", JsonValue::CreateStringValue(optionB));
            questionData.Insert(L"option_c", JsonValue::CreateStringValue(optionC));
            questionData.Insert(L"option_d", JsonValue::CreateStringValue(optionD));
            questionData.Insert(L"correct_option", JsonValue::CreateStringValue(correctOption));
            questionData.Insert(L"difficulty_level", JsonValue::CreateStringValue(difficulty));
            questionData.Insert(L"created_by", JsonValue::CreateStringValue(createdBy));

            Uri uri(m_projectUrl + L"/rest/v1/questions");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Content-Type", L"application/json");
            request.Content(HttpStringContent(questionData.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    dispatcher.TryEnqueue([this, response]()
                    {
                        if (response.StatusCode() == HttpStatusCode::Created || response.StatusCode() == HttpStatusCode::Ok)
                        {
                            if (OnQuizCreated)
                                OnQuizCreated(L"Question created successfully");
                        }
                        else
                        {
                            if (OnQuizCreationFailed)
                                OnQuizCreationFailed(L"Failed to create question");
                        }
                    });
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuizCreationFailed)
                            OnQuizCreationFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizCreationFailed)
                OnQuizCreationFailed(L"Connection error");
        }
    }

    void SupabaseClient::GetQuestionsByTeacher(hstring const &createdBy)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            Uri uri(m_projectUrl + L"/rest/v1/questions?select=id,question_text,option_a,option_b,option_c,option_d,correct_option,difficulty_level,created_by&created_by=eq." + createdBy);
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
                                    if (OnQuestionsFetched)
                                        OnQuestionsFetched(content);
                                }
                                catch (...)
                                {
                                    if (OnQuizCreationFailed)
                                        OnQuizCreationFailed(L"Parse error");
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([=]()
                        {
                            if (OnQuizCreationFailed)
                                OnQuizCreationFailed(L"Failed to fetch questions");
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([=]()
                    {
                        if (OnQuizCreationFailed)
                            OnQuizCreationFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizCreationFailed)
                OnQuizCreationFailed(L"Connection error");
        }
    }

    void SupabaseClient::DeleteQuestion(hstring const &questionId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();
            Uri uri(m_projectUrl + L"/rest/v1/questions?id=eq." + questionId);
            HttpRequestMessage request(HttpMethod::Delete(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    dispatcher.TryEnqueue([this, response]()
                    {
                        if (response.StatusCode() == HttpStatusCode::NoContent || response.StatusCode() == HttpStatusCode::Ok)
                        {
                            if (OnQuizCreated)
                                OnQuizCreated(L"Question deleted successfully");
                        }
                        else
                        {
                            if (OnQuizCreationFailed)
                                OnQuizCreationFailed(L"Failed to delete question");
                        }
                    });
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuizCreationFailed)
                            OnQuizCreationFailed(L"Request error");
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizCreationFailed)
                OnQuizCreationFailed(L"Connection error");
        }
    }

    // =====================================================
    // UC02: Question Bank Management - Create with Validation
    // =====================================================
    void SupabaseClient::CreateQuestionValidated(
        hstring const &id,
        hstring const &teacherId,
        hstring const &text,
        hstring const &optA, hstring const &optB, hstring const &optC, hstring const &optD,
        hstring const &correctOpt,
        hstring const &difficulty,
        hstring const &topic)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            // Build JSON parameters matching RPC signature
            JsonObject params;
            params.Insert(L"p_id", JsonValue::CreateStringValue(id));
            params.Insert(L"p_created_by", JsonValue::CreateStringValue(teacherId));
            params.Insert(L"p_question_text", JsonValue::CreateStringValue(text));
            params.Insert(L"p_option_a", JsonValue::CreateStringValue(optA));
            params.Insert(L"p_option_b", JsonValue::CreateStringValue(optB));
            params.Insert(L"p_option_c", JsonValue::CreateStringValue(optC));
            params.Insert(L"p_option_d", JsonValue::CreateStringValue(optD));
            params.Insert(L"p_correct_option", JsonValue::CreateStringValue(correctOpt));
            params.Insert(L"p_difficulty_level", JsonValue::CreateStringValue(difficulty));
            params.Insert(L"p_topic", JsonValue::CreateStringValue(topic));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/create_question_validated");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                if (asyncOp.Status() == AsyncStatus::Completed)
                {
                    try
                    {
                        auto response = asyncOp.GetResults();
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto resultObj = JsonObject::Parse(content);
                                    auto status = resultObj.GetNamedString(L"status", L"error");
                                    auto message = resultObj.GetNamedString(L"message", L"Unknown error");

                                    if (OnQuestionValidatedCreated)
                                    {
                                        OnQuestionValidatedCreated(status == L"success", message);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnQuestionValidatedCreated)
                                    {
                                        OnQuestionValidatedCreated(false, L"Failed to parse response");
                                    }
                                }
                            });
                        });
                    }
                    catch (...)
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnQuestionValidatedCreated)
                            {
                                OnQuestionValidatedCreated(false, L"Request failed");
                            }
                        });
                    }
                }
                else
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuestionValidatedCreated)
                        {
                            OnQuestionValidatedCreated(false, L"Network error");
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnQuestionValidatedCreated)
            {
                OnQuestionValidatedCreated(false, L"Connection error");
            }
        }
    }

    // =====================================================
    // UC02: Question Bank Management - Safe Delete
    // =====================================================
    void SupabaseClient::DeleteQuestionSafe(hstring const &questionId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            JsonObject params;
            params.Insert(L"question_id_input", JsonValue::CreateStringValue(questionId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/delete_question_safe");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                if (asyncOp.Status() == AsyncStatus::Completed)
                {
                    try
                    {
                        auto response = asyncOp.GetResults();
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto resultObj = JsonObject::Parse(content);
                                    auto status = resultObj.GetNamedString(L"status", L"error");
                                    auto message = resultObj.GetNamedString(L"message", L"Unknown error");
                                    int quizCount = static_cast<int>(resultObj.GetNamedNumber(L"quiz_count", 0));

                                    if (OnQuestionDeleteResult)
                                    {
                                        OnQuestionDeleteResult(status, message, quizCount);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnQuestionDeleteResult)
                                    {
                                        OnQuestionDeleteResult(L"error", L"Failed to parse response", 0);
                                    }
                                }
                            });
                        });
                    }
                    catch (...)
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnQuestionDeleteResult)
                            {
                                OnQuestionDeleteResult(L"error", L"Request failed", 0);
                            }
                        });
                    }
                }
                else
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuestionDeleteResult)
                        {
                            OnQuestionDeleteResult(L"error", L"Network error", 0);
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnQuestionDeleteResult)
            {
                OnQuestionDeleteResult(L"error", L"Connection error", 0);
            }
        }
    }

    // =====================================================
    // UC08: Quiz Management - Teacher Delete
    // =====================================================
    void SupabaseClient::DeleteQuizAsTeacher(hstring const &quizId, hstring const &teacherId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            JsonObject params;
            params.Insert(L"quiz_id_input", JsonValue::CreateStringValue(quizId));
            params.Insert(L"teacher_id", JsonValue::CreateStringValue(teacherId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/delete_quiz_teacher");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                if (asyncOp.Status() == AsyncStatus::Completed)
                {
                    try
                    {
                        auto response = asyncOp.GetResults();
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto resultObj = JsonObject::Parse(content);
                                    auto status = resultObj.GetNamedString(L"status", L"error");
                                    auto message = resultObj.GetNamedString(L"message", L"Unknown error");
                                    int attemptCount = static_cast<int>(resultObj.GetNamedNumber(L"attempt_count", 0));

                                    bool success = (status == L"success");

                                    if (OnQuizDeleteResult)
                                    {
                                        OnQuizDeleteResult(success, message, attemptCount);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnQuizDeleteResult)
                                    {
                                        OnQuizDeleteResult(false, L"Failed to parse response", 0);
                                    }
                                }
                            });
                        });
                    }
                    catch (...)
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnQuizDeleteResult)
                            {
                                OnQuizDeleteResult(false, L"Request failed", 0);
                            }
                        });
                    }
                }
                else
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuizDeleteResult)
                        {
                            OnQuizDeleteResult(false, L"Network error", 0);
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizDeleteResult)
            {
                OnQuizDeleteResult(false, L"Connection error", 0);
            }
        }
    }

    // =====================================================
    // UC08: Quiz Management - Admin Purge
    // =====================================================
    void SupabaseClient::PurgeQuizAsAdmin(hstring const &quizId, hstring const &adminId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            JsonObject params;
            params.Insert(L"quiz_id_input", JsonValue::CreateStringValue(quizId));
            params.Insert(L"admin_id", JsonValue::CreateStringValue(adminId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/purge_quiz_admin");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                if (asyncOp.Status() == AsyncStatus::Completed)
                {
                    try
                    {
                        auto response = asyncOp.GetResults();
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto resultObj = JsonObject::Parse(content);
                                    auto status = resultObj.GetNamedString(L"status", L"error");
                                    auto message = resultObj.GetNamedString(L"message", L"Unknown error");
                                    int attemptsDeleted = static_cast<int>(resultObj.GetNamedNumber(L"attempts_deleted", 0));

                                    bool success = (status == L"success");

                                    if (OnQuizPurgeResult)
                                    {
                                        OnQuizPurgeResult(success, attemptsDeleted, message);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnQuizPurgeResult)
                                    {
                                        OnQuizPurgeResult(false, 0, L"Failed to parse response");
                                    }
                                }
                            });
                        });
                    }
                    catch (...)
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnQuizPurgeResult)
                            {
                                OnQuizPurgeResult(false, 0, L"Request failed");
                            }
                        });
                    }
                }
                else
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuizPurgeResult)
                        {
                            OnQuizPurgeResult(false, 0, L"Network error");
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizPurgeResult)
            {
                OnQuizPurgeResult(false, 0, L"Connection error");
            }
        }
    }

    void SupabaseClient::GetQuizQuestions(hstring const &quizId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            JsonObject params;
            params.Insert(L"input_quiz_id", JsonValue::CreateStringValue(quizId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/get_quiz_questions");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto questionsArray = JsonArray::Parse(content);

                                    std::vector<QuestionData> questions;
                                    for (uint32_t i = 0; i < questionsArray.Size(); ++i)
                                    {
                                        auto questionObj = questionsArray.GetObjectAt(i);
                                        QuestionData q;
                                        q.quiz_question_id = questionObj.GetNamedString(L"quiz_question_id");
                                        q.question_id = questionObj.GetNamedString(L"question_id");
                                        q.question_text = questionObj.GetNamedString(L"question_text");
                                        q.option_a = questionObj.GetNamedString(L"option_a");
                                        q.option_b = questionObj.GetNamedString(L"option_b");
                                        q.option_c = questionObj.GetNamedString(L"option_c");
                                        q.option_d = questionObj.GetNamedString(L"option_d");
                                        q.difficulty_level = questionObj.GetNamedString(L"difficulty_level");
                                        q.points = static_cast<int>(questionObj.GetNamedNumber(L"points"));
                                        q.order_num = static_cast<int>(questionObj.GetNamedNumber(L"order_num"));

                                        questions.push_back(q);
                                    }

                                    if (OnQuizQuestionsLoaded)
                                    {
                                        OnQuizQuestionsLoaded(true, questions);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnQuizQuestionsLoaded)
                                    {
                                        OnQuizQuestionsLoaded(false, std::vector<QuestionData>());
                                    }
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnQuizQuestionsLoaded)
                            {
                                OnQuizQuestionsLoaded(false, std::vector<QuestionData>());
                            }
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuizQuestionsLoaded)
                        {
                            OnQuizQuestionsLoaded(false, std::vector<QuestionData>());
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizQuestionsLoaded)
            {
                OnQuizQuestionsLoaded(false, std::vector<QuestionData>());
            }
        }
    }

    void SupabaseClient::SubmitQuizAttempt(
        hstring const &studentId,
        hstring const &quizId,
        hstring const &answersJson,
        int timeSpentSeconds)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            // Parse answersJson and convert to JSONB
            auto answersArray = JsonArray::Parse(answersJson);

            JsonObject params;
            params.Insert(L"input_student_id", JsonValue::CreateStringValue(studentId));
            params.Insert(L"input_quiz_id", JsonValue::CreateStringValue(quizId));
            params.Insert(L"input_answers", answersArray);
            params.Insert(L"input_time_spent_seconds", JsonValue::CreateNumberValue(static_cast<double>(timeSpentSeconds)));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/submit_quiz_attempt");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto resultArray = JsonArray::Parse(content);

                                    if (resultArray.Size() > 0)
                                    {
                                        auto resultObj = resultArray.GetObjectAt(0);
                                        AttemptResult result;
                                        result.success = resultObj.GetNamedBoolean(L"success", false);
                                        result.attempt_id = resultObj.GetNamedString(L"attempt_id", L"");
                                        result.score = static_cast<int>(resultObj.GetNamedNumber(L"score", 0.0));
                                        result.total_points = static_cast<int>(resultObj.GetNamedNumber(L"total_points", 0.0));
                                        result.correct_count = static_cast<int>(resultObj.GetNamedNumber(L"correct_count", 0.0));
                                        result.incorrect_count = static_cast<int>(resultObj.GetNamedNumber(L"incorrect_count", 0.0));
                                        result.message = resultObj.GetNamedString(L"message", L"");

                                        if (OnAttemptSubmitted)
                                        {
                                            OnAttemptSubmitted(result);
                                        }
                                    }
                                }
                                catch (...)
                                {
                                    if (OnAttemptSubmitted)
                                    {
                                        AttemptResult result;
                                        result.success = false;
                                        result.message = L"Failed to parse response";
                                        OnAttemptSubmitted(result);
                                    }
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnAttemptSubmitted)
                            {
                                AttemptResult result;
                                result.success = false;
                                result.message = L"HTTP error";
                                OnAttemptSubmitted(result);
                            }
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnAttemptSubmitted)
                        {
                            AttemptResult result;
                            result.success = false;
                            result.message = L"Request failed";
                            OnAttemptSubmitted(result);
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnAttemptSubmitted)
            {
                AttemptResult result;
                result.success = false;
                result.message = L"Connection error";
                OnAttemptSubmitted(result);
            }
        }
    }

    void SupabaseClient::GetStudentQuizzes(hstring const &studentId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            JsonObject params;
            params.Insert(L"input_student_id", JsonValue::CreateStringValue(studentId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/get_student_quizzes");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto quizzesArray = JsonArray::Parse(content);

                                    std::vector<QuizData> quizzes;
                                    for (uint32_t i = 0; i < quizzesArray.Size(); ++i)
                                    {
                                        auto quizObj = quizzesArray.GetObjectAt(i);
                                        QuizData q;
                                        q.quiz_id = quizObj.GetNamedString(L"quiz_id");
                                        q.quiz_title = quizObj.GetNamedString(L"quiz_title");
                                        q.time_limit_minutes = static_cast<int>(quizObj.GetNamedNumber(L"time_limit_minutes"));
                                        q.total_points = static_cast<int>(quizObj.GetNamedNumber(L"total_points"));
                                        q.max_attempts = quizObj.GetNamedString(L"max_attempts");
                                        q.attempts_used = static_cast<int>(quizObj.GetNamedNumber(L"attempts_used"));
                                        q.result_visibility = quizObj.GetNamedString(L"result_visibility");

                                        quizzes.push_back(q);
                                    }

                                    if (OnStudentQuizzesLoaded)
                                    {
                                        OnStudentQuizzesLoaded(true, quizzes);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnStudentQuizzesLoaded)
                                    {
                                        OnStudentQuizzesLoaded(false, std::vector<QuizData>());
                                    }
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnStudentQuizzesLoaded)
                            {
                                OnStudentQuizzesLoaded(false, std::vector<QuizData>());
                            }
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnStudentQuizzesLoaded)
                        {
                            OnStudentQuizzesLoaded(false, std::vector<QuizData>());
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnStudentQuizzesLoaded)
            {
                OnStudentQuizzesLoaded(false, std::vector<QuizData>());
            }
        }
    }

    void SupabaseClient::GetAttemptResults(hstring const &studentId, hstring const &quizId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            JsonObject params;
            params.Insert(L"input_student_id", JsonValue::CreateStringValue(studentId));
            params.Insert(L"input_quiz_id", JsonValue::CreateStringValue(quizId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/get_attempt_results");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto attemptsArray = JsonArray::Parse(content);

                                    std::vector<AttemptData> attempts;
                                    for (uint32_t i = 0; i < attemptsArray.Size(); ++i)
                                    {
                                        auto attemptObj = attemptsArray.GetObjectAt(i);
                                        AttemptData a;
                                        a.attempt_id = attemptObj.GetNamedString(L"attempt_id");
                                        a.attempt_number = static_cast<int>(attemptObj.GetNamedNumber(L"attempt_number"));
                                        a.score = static_cast<int>(attemptObj.GetNamedNumber(L"score"));
                                        a.total_points = static_cast<int>(attemptObj.GetNamedNumber(L"total_points"));
                                        a.correct_count = static_cast<int>(attemptObj.GetNamedNumber(L"correct_count"));
                                        a.incorrect_count = static_cast<int>(attemptObj.GetNamedNumber(L"incorrect_count"));

                                        attempts.push_back(a);
                                    }

                                    if (OnAttemptResultsLoaded)
                                    {
                                        OnAttemptResultsLoaded(true, attempts);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnAttemptResultsLoaded)
                                    {
                                        OnAttemptResultsLoaded(false, std::vector<AttemptData>());
                                    }
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnAttemptResultsLoaded)
                            {
                                OnAttemptResultsLoaded(false, std::vector<AttemptData>());
                            }
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnAttemptResultsLoaded)
                        {
                            OnAttemptResultsLoaded(false, std::vector<AttemptData>());
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnAttemptResultsLoaded)
            {
                OnAttemptResultsLoaded(false, std::vector<AttemptData>());
            }
        }
    }

    void SupabaseClient::GetAttemptDetailsWithAnswers(hstring const &attemptId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            JsonObject params;
            params.Insert(L"input_attempt_id", JsonValue::CreateStringValue(attemptId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/get_attempt_details_with_answers");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto answersArray = JsonArray::Parse(content);

                                    std::vector<AnswerDetail> answers;
                                    for (uint32_t i = 0; i < answersArray.Size(); ++i)
                                    {
                                        auto answerObj = answersArray.GetObjectAt(i);
                                        AnswerDetail a;
                                        a.question_id = answerObj.GetNamedString(L"question_id");
                                        a.question_text = answerObj.GetNamedString(L"question_text");
                                        a.selected_option = answerObj.GetNamedString(L"selected_option", L"");
                                        a.correct_option = answerObj.GetNamedString(L"correct_option");
                                        a.is_correct = answerObj.GetNamedBoolean(L"is_correct", false);
                                        a.points_earned = static_cast<int>(answerObj.GetNamedNumber(L"points_earned", 0.0));
                                        a.total_possible_points = static_cast<int>(answerObj.GetNamedNumber(L"total_possible_points", 0.0));

                                        answers.push_back(a);
                                    }

                                    if (OnAttemptDetailsLoaded)
                                    {
                                        OnAttemptDetailsLoaded(true, answers);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnAttemptDetailsLoaded)
                                    {
                                        OnAttemptDetailsLoaded(false, std::vector<AnswerDetail>());
                                    }
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnAttemptDetailsLoaded)
                            {
                                OnAttemptDetailsLoaded(false, std::vector<AnswerDetail>());
                            }
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnAttemptDetailsLoaded)
                        {
                            OnAttemptDetailsLoaded(false, std::vector<AnswerDetail>());
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnAttemptDetailsLoaded)
            {
                OnAttemptDetailsLoaded(false, std::vector<AnswerDetail>());
            }
        }
    }

    void SupabaseClient::GetQuizAttemptsReport(hstring const &quizId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            JsonObject params;
            params.Insert(L"input_quiz_id", JsonValue::CreateStringValue(quizId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/get_quiz_attempts_report");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto content = readOp.GetResults();
                                    auto reportsArray = JsonArray::Parse(content);

                                    std::vector<AttemptReportRow> reports;
                                    for (uint32_t i = 0; i < reportsArray.Size(); ++i)
                                    {
                                        auto reportObj = reportsArray.GetObjectAt(i);
                                        AttemptReportRow r;
                                        r.student_id = reportObj.GetNamedString(L"student_id");
                                        r.username = reportObj.GetNamedString(L"username");
                                        r.attempt_number = static_cast<int>(reportObj.GetNamedNumber(L"attempt_number"));
                                        r.score = static_cast<int>(reportObj.GetNamedNumber(L"score"));
                                        r.total_points = static_cast<int>(reportObj.GetNamedNumber(L"total_points"));
                                        r.correct_count = static_cast<int>(reportObj.GetNamedNumber(L"correct_count"));
                                        r.incorrect_count = static_cast<int>(reportObj.GetNamedNumber(L"incorrect_count"));
                                        r.time_spent_seconds = static_cast<int>(reportObj.GetNamedNumber(L"time_spent_seconds", 0.0));

                                        reports.push_back(r);
                                    }

                                    if (OnQuizReportLoaded)
                                    {
                                        OnQuizReportLoaded(true, reports);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnQuizReportLoaded)
                                    {
                                        OnQuizReportLoaded(false, std::vector<AttemptReportRow>());
                                    }
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnQuizReportLoaded)
                            {
                                OnQuizReportLoaded(false, std::vector<AttemptReportRow>());
                            }
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnQuizReportLoaded)
                        {
                            OnQuizReportLoaded(false, std::vector<AttemptReportRow>());
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnQuizReportLoaded)
            {
                OnQuizReportLoaded(false, std::vector<AttemptReportRow>());
            }
        }
    }

    void SupabaseClient::GetQuizReportCsv(hstring const &quizId)
    {
        try
        {
            auto dispatcher = DispatcherQueue::GetForCurrentThread();

            JsonObject params;
            params.Insert(L"input_quiz_id", JsonValue::CreateStringValue(quizId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/get_quiz_attempts_report");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Accept", L"text/csv");
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            m_httpClient.SendRequestAsync(request).Completed([this, dispatcher](auto const &asyncOp, auto)
                                                             {
                try
                {
                    auto response = asyncOp.GetResults();
                    if (response.StatusCode() == HttpStatusCode::Ok)
                    {
                        response.Content().ReadAsStringAsync().Completed([this, dispatcher](auto const &readOp, auto)
                        {
                            dispatcher.TryEnqueue([this, readOp]()
                            {
                                try
                                {
                                    auto csvContent = readOp.GetResults();
                                    if (OnCsvReportLoaded)
                                    {
                                        OnCsvReportLoaded(true, csvContent);
                                    }
                                }
                                catch (...)
                                {
                                    if (OnCsvReportLoaded)
                                    {
                                        OnCsvReportLoaded(false, L"");
                                    }
                                }
                            });
                        });
                    }
                    else
                    {
                        dispatcher.TryEnqueue([this]()
                        {
                            if (OnCsvReportLoaded)
                            {
                                OnCsvReportLoaded(false, L"");
                            }
                        });
                    }
                }
                catch (...)
                {
                    dispatcher.TryEnqueue([this]()
                    {
                        if (OnCsvReportLoaded)
                        {
                            OnCsvReportLoaded(false, L"");
                        }
                    });
                } });
        }
        catch (...)
        {
            if (OnCsvReportLoaded)
            {
                OnCsvReportLoaded(false, L"");
            }
        }
    }
}
#include "pch.h"
#include "SupabaseClientAsync.h"
#include "BCryptPasswordHasher.h"
#include <winrt/Windows.Web.Http.Filters.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Filters;
using namespace Windows::Data::Json;

namespace
{
    hstring TrimUserId(hstring const &userId)
    {
        std::wstring str(userId);
        auto pos = str.find_last_not_of(L' ');
        if (pos != std::wstring::npos)
        {
            str.erase(pos + 1);
        }
        else
        {
            str.clear();
        }
        return hstring(str);
    }
}

namespace quiz_examination_system
{
    SupabaseClientAsync::SupabaseClientAsync()
        : m_connected(true)
    {
        HttpBaseProtocolFilter filter;
        m_httpClient = HttpClient(filter);
    }

    IAsyncOperation<hstring> SupabaseClientAsync::LoginAsync(hstring const &username, hstring const &password)
    {
        JsonObject resultJson;
        resultJson.Insert(L"success", JsonValue::CreateBooleanValue(false));

        try
        {
            JsonObject statusParams;
            statusParams.Insert(L"input_username", JsonValue::CreateStringValue(username));
            Uri statusUri(m_projectUrl + L"/rest/v1/rpc/check_account_status");
            HttpRequestMessage statusReq(HttpMethod::Post(), statusUri);
            statusReq.Headers().Insert(L"apikey", m_anonKey);
            statusReq.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            statusReq.Content(HttpStringContent(statusParams.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto statusResponse = co_await m_httpClient.SendRequestAsync(statusReq);
            auto statusContent = co_await statusResponse.Content().ReadAsStringAsync();
            auto statusObj = JsonObject::Parse(statusContent);
            auto checkStatus = statusObj.GetNamedString(L"status", L"");

            if (checkStatus == L"user_not_found")
            {
                resultJson.Insert(L"errorMessage", JsonValue::CreateStringValue(L"User not found"));
                co_return resultJson.Stringify();
            }
            else if (checkStatus == L"locked")
            {
                auto minutesRemaining = static_cast<int>(statusObj.GetNamedNumber(L"minutes_remaining", 30));
                resultJson.Insert(L"errorMessage", JsonValue::CreateStringValue(hstring(L"Account is locked. Try again in ") + to_hstring(minutesRemaining) + L" minutes."));
                co_return resultJson.Stringify();
            }
            else if (checkStatus == L"inactive")
            {
                resultJson.Insert(L"errorMessage", JsonValue::CreateStringValue(L"Account is inactive"));
                co_return resultJson.Stringify();
            }

            Uri uri(m_projectUrl + L"/rest/v1/users?select=id,username,password_hash,role&username=eq." + username);
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            OutputDebugStringW((L"API URL: " + uri.ToString() + L"\n").c_str());

            auto response = co_await m_httpClient.SendRequestAsync(request);
            auto statusCode = static_cast<int>(response.StatusCode());
            OutputDebugStringW((L"HTTP Status Code: " + std::to_wstring(statusCode) + L"\n").c_str());

            if (response.StatusCode() != HttpStatusCode::Ok)
            {
                auto errorContent = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"Error Response: " + errorContent + L"\n").c_str());
                resultJson.Insert(L"errorMessage", JsonValue::CreateStringValue(L"Login failed"));
                co_return resultJson.Stringify();
            }

            auto content = co_await response.Content().ReadAsStringAsync();
            JsonArray users = JsonArray::Parse(content);

            if (users.Size() > 0)
            {
                auto user = users.GetObjectAt(0);
                auto userId = user.GetNamedString(L"id");
                auto storedHash = user.GetNamedString(L"password_hash");
                auto role = user.GetNamedString(L"role", L"Student");

                OutputDebugStringW((L"Stored hash from DB: " + storedHash + L"\n").c_str());

                // Generate hash for "123456" for testing
                auto testHash = BCryptPasswordHasher::HashPassword(L"123456");
                OutputDebugStringW((L"Generated hash for '123456': " + testHash + L"\n").c_str());

                bool passwordVerified = BCryptPasswordHasher::VerifyPassword(password, storedHash);
                OutputDebugStringW((L"Password verification result: " + std::to_wstring(passwordVerified) + L"\n").c_str());

                if (passwordVerified)
                {
                    JsonObject resetParams;
                    resetParams.Insert(L"input_username", JsonValue::CreateStringValue(username));
                    Uri resetUri(m_projectUrl + L"/rest/v1/rpc/handle_login_success");
                    HttpRequestMessage resetReq(HttpMethod::Post(), resetUri);
                    resetReq.Headers().Insert(L"apikey", m_anonKey);
                    resetReq.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
                    resetReq.Content(HttpStringContent(resetParams.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

                    co_await m_httpClient.SendRequestAsync(resetReq);

                    resultJson.SetNamedValue(L"success", JsonValue::CreateBooleanValue(true));
                    resultJson.Insert(L"username", JsonValue::CreateStringValue(username));
                    resultJson.Insert(L"role", JsonValue::CreateStringValue(role));
                    resultJson.Insert(L"userId", JsonValue::CreateStringValue(userId));
                    auto displayRole = (role == L"Admin") ? L"Administrator" : (role == L"Teacher") ? L"Lecturer"
                                                                                                    : L"Student";
                    resultJson.Insert(L"displayRole", JsonValue::CreateStringValue(displayRole));
                }
                else
                {
                    JsonObject rpcParams;
                    rpcParams.Insert(L"input_username", JsonValue::CreateStringValue(username));
                    Uri rpcUri(m_projectUrl + L"/rest/v1/rpc/handle_login_failure");
                    HttpRequestMessage rpcReq(HttpMethod::Post(), rpcUri);
                    rpcReq.Headers().Insert(L"apikey", m_anonKey);
                    rpcReq.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
                    rpcReq.Content(HttpStringContent(rpcParams.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

                    co_await m_httpClient.SendRequestAsync(rpcReq);

                    resultJson.Insert(L"errorMessage", JsonValue::CreateStringValue(L"Incorrect password"));
                }
            }
            else
            {
                resultJson.Insert(L"errorMessage", JsonValue::CreateStringValue(L"User not found"));
            }
        }
        catch (hresult_error const &ex)
        {
            resultJson.Insert(L"errorMessage", JsonValue::CreateStringValue(ex.message()));
        }

        co_return resultJson.Stringify();
    }

    IAsyncOperation<bool> SupabaseClientAsync::ChangePasswordAsync(hstring const &username, hstring const &currentPassword, hstring const &newPassword)
    {
        try
        {
            Uri uri(m_projectUrl + L"/rest/v1/users?select=id,password_hash&username=eq." + username);
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            auto response = co_await m_httpClient.SendRequestAsync(request);
            if (response.StatusCode() != HttpStatusCode::Ok)
            {
                co_return false;
            }

            auto content = co_await response.Content().ReadAsStringAsync();
            JsonArray users = JsonArray::Parse(content);

            if (users.Size() == 0)
            {
                co_return false;
            }

            auto user = users.GetObjectAt(0);
            auto userId = user.GetNamedString(L"id");
            auto storedHash = user.GetNamedString(L"password_hash");

            if (!BCryptPasswordHasher::VerifyPassword(currentPassword, storedHash))
            {
                co_return false;
            }

            auto trimmedUserId = TrimUserId(userId);

            auto newHash = BCryptPasswordHasher::HashPassword(newPassword);
            JsonObject updateData;
            updateData.Insert(L"password_hash", JsonValue::CreateStringValue(newHash));

            Uri updateUri(m_projectUrl + L"/rest/v1/users?id=eq." + trimmedUserId);
            HttpRequestMessage updateReq(HttpMethod::Patch(), updateUri);
            updateReq.Headers().Insert(L"apikey", m_anonKey);
            updateReq.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            updateReq.Headers().Insert(L"Prefer", L"return=minimal");
            updateReq.Content(HttpStringContent(updateData.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto updateResponse = co_await m_httpClient.SendRequestAsync(updateReq);
            co_return updateResponse.StatusCode() == HttpStatusCode::NoContent;
        }
        catch (...)
        {
            co_return false;
        }
    }

    IAsyncOperation<hstring> SupabaseClientAsync::GetQuestionsJsonAsync(hstring const &createdBy)
    {
        try
        {
            Uri uri(m_projectUrl + L"/rest/v1/questions?select=*&created_by=eq." + createdBy + L"&order=created_at.desc");
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            auto response = co_await m_httpClient.SendRequestAsync(request);
            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                auto content = co_await response.Content().ReadAsStringAsync();
                co_return content;
            }
        }
        catch (...)
        {
        }
        co_return L"[]";
    }

    IAsyncOperation<hstring> SupabaseClientAsync::CreateQuestionValidatedAsync(
        hstring const &id, hstring const &teacherId, hstring const &text,
        hstring const &optA, hstring const &optB, hstring const &optC, hstring const &optD,
        hstring const &correctOpt, hstring const &difficulty, hstring const &topic)
    {
        JsonObject resultJson;
        resultJson.Insert(L"success", JsonValue::CreateBooleanValue(false));

        try
        {
            JsonObject questionData;
            questionData.Insert(L"id", JsonValue::CreateStringValue(id));
            questionData.Insert(L"created_by", JsonValue::CreateStringValue(teacherId));
            questionData.Insert(L"question_text", JsonValue::CreateStringValue(text));
            questionData.Insert(L"option_a", JsonValue::CreateStringValue(optA));
            questionData.Insert(L"option_b", JsonValue::CreateStringValue(optB));
            questionData.Insert(L"option_c", JsonValue::CreateStringValue(optC));
            questionData.Insert(L"option_d", JsonValue::CreateStringValue(optD));
            questionData.Insert(L"correct_option", JsonValue::CreateStringValue(correctOpt));
            questionData.Insert(L"difficulty_level", JsonValue::CreateStringValue(difficulty));
            questionData.Insert(L"topic", JsonValue::CreateStringValue(topic));

            Uri uri(m_projectUrl + L"/rest/v1/questions");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Prefer", L"return=minimal");
            request.Content(HttpStringContent(questionData.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await m_httpClient.SendRequestAsync(request);

            if (response.StatusCode() == HttpStatusCode::Created)
            {
                resultJson.SetNamedValue(L"success", JsonValue::CreateBooleanValue(true));
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Question created successfully"));
            }
            else
            {
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Failed to create question"));
            }
        }
        catch (hresult_error const &ex)
        {
            resultJson.Insert(L"message", JsonValue::CreateStringValue(ex.message()));
        }

        co_return resultJson.Stringify();
    }

    IAsyncOperation<bool> SupabaseClientAsync::DeleteQuestionSafeAsync(hstring const &questionId)
    {
        try
        {
            Uri uri(m_projectUrl + L"/rest/v1/questions?id=eq." + questionId);
            HttpRequestMessage request(HttpMethod::Delete(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Prefer", L"return=minimal");

            auto response = co_await m_httpClient.SendRequestAsync(request);
            co_return response.StatusCode() == HttpStatusCode::NoContent;
        }
        catch (...)
        {
            co_return false;
        }
    }

    IAsyncOperation<hstring> SupabaseClientAsync::GetQuizQuestionsJsonAsync(hstring const &quizId)
    {
        hstring result = L"[]";

        try
        {
            JsonObject params;
            params.Insert(L"p_quiz_id", JsonValue::CreateStringValue(quizId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/get_quiz_questions_v2");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await m_httpClient.SendRequestAsync(request);
            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                result = co_await response.Content().ReadAsStringAsync();
            }
        }
        catch (...)
        {
        }

        co_return result;
    }

    IAsyncOperation<hstring> SupabaseClientAsync::GetStudentQuizzesJsonAsync(hstring const &studentId)
    {
        hstring result = L"[]";

        try
        {
            JsonObject params;
            params.Insert(L"p_student_id", JsonValue::CreateStringValue(studentId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/get_student_quizzes_v2");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await m_httpClient.SendRequestAsync(request);
            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                result = co_await response.Content().ReadAsStringAsync();
            }
        }
        catch (...)
        {
        }

        co_return result;
    }

    IAsyncOperation<hstring> SupabaseClientAsync::GetAllUsersAsync()
    {
        hstring result = L"[]";

        try
        {
            OutputDebugStringW(L"[GetAllUsers] START - Fetching from Supabase\n");

            HttpBaseProtocolFilter filter;
            filter.CacheControl().ReadBehavior(HttpCacheReadBehavior::MostRecent);
            filter.CacheControl().WriteBehavior(HttpCacheWriteBehavior::NoCache);
            HttpClient freshClient(filter);

            Uri uri(m_projectUrl + L"/rest/v1/users?select=id,username,role,status&order=username.asc");
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            auto response = co_await freshClient.SendRequestAsync(request);
            OutputDebugStringW((L"[GetAllUsers] HTTP Status: " + std::to_wstring((int)response.StatusCode()) + L"\n").c_str());

            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                result = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[GetAllUsers] Received data length: " + std::to_wstring(result.size()) + L" chars\n").c_str());

                // Parse to count users
                try
                {
                    auto usersArray = JsonArray::Parse(result);
                    OutputDebugStringW((L"[GetAllUsers] SUCCESS - Fetched " + std::to_wstring(usersArray.Size()) + L" users from Supabase\n").c_str());
                }
                catch (...)
                {
                    OutputDebugStringW(L"[GetAllUsers] WARNING - Failed to parse JSON\n");
                }
            }
            else
            {
                OutputDebugStringW(L"[GetAllUsers] ERROR - Non-OK status code\n");
            }
        }
        catch (...)
        {
            OutputDebugStringW(L"[GetAllUsers] EXCEPTION caught\n");
        }

        co_return result;
    }

    IAsyncOperation<bool> SupabaseClientAsync::CreateUserAsync(hstring const &adminUsername, hstring const &newUsername, hstring const &hashedPassword, hstring const &role)
    {
        try
        {
            JsonObject params;
            params.Insert(L"p_admin_username", JsonValue::CreateStringValue(adminUsername));
            params.Insert(L"p_new_username", JsonValue::CreateStringValue(newUsername));
            params.Insert(L"p_password_hash", JsonValue::CreateStringValue(hashedPassword));
            params.Insert(L"p_role", JsonValue::CreateStringValue(role));

            OutputDebugStringW((L"CreateUser RPC JSON: " + params.Stringify() + L"\n").c_str());

            Uri uri(m_projectUrl + L"/rest/v1/rpc/create_user_by_admin");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await m_httpClient.SendRequestAsync(request);
            auto statusCode = static_cast<int>(response.StatusCode());
            OutputDebugStringW((L"CreateUser HTTP Status: " + std::to_wstring(statusCode) + L"\n").c_str());

            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                auto content = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"CreateUser Response: " + content + L"\n").c_str());

                auto resultObj = JsonObject::Parse(content);
                if (resultObj.HasKey(L"success"))
                {
                    co_return resultObj.GetNamedBoolean(L"success");
                }
            }
            else
            {
                auto errorContent = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"CreateUser Error: " + errorContent + L"\n").c_str());
            }

            co_return false;
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"CreateUser Exception: " + ex.message() + L"\n").c_str());
            co_return false;
        }
    }

    IAsyncOperation<bool> SupabaseClientAsync::ResetUserPasswordAsync(hstring const &userId, hstring const &hashedPassword)
    {
        try
        {
            OutputDebugStringW(L"[ResetUserPasswordAsync] START\n");
            OutputDebugStringW((L"[ResetUserPasswordAsync] Input UserId: '" + userId + L"'\n").c_str());
            OutputDebugStringW((L"[ResetUserPasswordAsync] Input UserId length: " + std::to_wstring(userId.size()) + L"\n").c_str());

            auto trimmedUserId = TrimUserId(userId);
            OutputDebugStringW((L"[ResetUserPasswordAsync] Trimmed UserId: '" + trimmedUserId + L"'\n").c_str());
            OutputDebugStringW((L"[ResetUserPasswordAsync] Trimmed UserId length: " + std::to_wstring(trimmedUserId.size()) + L"\n").c_str());

            JsonObject params;
            params.Insert(L"password_hash", JsonValue::CreateStringValue(hashedPassword));

            auto uriString = m_projectUrl + L"/rest/v1/users?id=eq." + trimmedUserId;
            OutputDebugStringW((L"[ResetUserPasswordAsync] URI: " + uriString + L"\n").c_str());
            OutputDebugStringW((L"[ResetUserPasswordAsync] Body: " + params.Stringify() + L"\n").c_str());

            Uri uri(uriString);
            HttpRequestMessage request(HttpMethod::Patch(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Prefer", L"return=minimal");
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            OutputDebugStringW(L"[ResetUserPasswordAsync] Sending PATCH request...\n");
            auto response = co_await m_httpClient.SendRequestAsync(request);
            auto statusCode = response.StatusCode();
            OutputDebugStringW((L"[ResetUserPasswordAsync] HTTP Status: " + std::to_wstring(static_cast<int>(statusCode)) + L"\n").c_str());

            bool success = statusCode == HttpStatusCode::NoContent;
            OutputDebugStringW((L"[ResetUserPasswordAsync] Result: " + std::wstring(success ? L"SUCCESS" : L"FAILED") + L"\n").c_str());
            co_return success;
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[ResetUserPasswordAsync] Exception: " + ex.message() + L"\n").c_str());
            co_return false;
        }
        catch (...)
        {
            OutputDebugStringW(L"[ResetUserPasswordAsync] Unknown exception\n");
            co_return false;
        }
    }

    IAsyncOperation<bool> SupabaseClientAsync::UpdateUserStatusAsync(hstring const &userId, bool isActive)
    {
        try
        {
            OutputDebugStringW(L"[UpdateUserStatus] START\n");
            auto trimmedUserId = TrimUserId(userId);
            OutputDebugStringW((L"[UpdateUserStatus] UserId: " + trimmedUserId + L"\n").c_str());
            OutputDebugStringW((L"[UpdateUserStatus] New status: " + std::wstring(isActive ? L"Active" : L"Inactive") + L"\n").c_str());

            JsonObject params;
            params.Insert(L"status", JsonValue::CreateStringValue(isActive ? L"Active" : L"Inactive"));

            auto uriString = m_projectUrl + L"/rest/v1/users?id=eq." + trimmedUserId;
            OutputDebugStringW((L"[UpdateUserStatus] URI: " + uriString + L"\n").c_str());
            OutputDebugStringW((L"[UpdateUserStatus] Body: " + params.Stringify() + L"\n").c_str());

            Uri uri(uriString);
            HttpRequestMessage request(HttpMethod::Patch(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Prefer", L"return=minimal");
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            OutputDebugStringW(L"[UpdateUserStatus] Sending PATCH request...\n");
            auto response = co_await m_httpClient.SendRequestAsync(request);
            auto statusCode = response.StatusCode();
            OutputDebugStringW((L"[UpdateUserStatus] HTTP Status: " + std::to_wstring(static_cast<int>(statusCode)) + L"\n").c_str());

            auto responseBody = co_await response.Content().ReadAsStringAsync();
            OutputDebugStringW((L"[UpdateUserStatus] Response: " + responseBody + L"\n").c_str());

            bool success = statusCode == HttpStatusCode::NoContent || statusCode == HttpStatusCode::Ok;
            OutputDebugStringW((L"[UpdateUserStatus] Result: " + std::wstring(success ? L"SUCCESS" : L"FAILED") + L"\n").c_str());
            co_return success;
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[UpdateUserStatus] Exception: " + ex.message() + L"\n").c_str());
            co_return false;
        }
        catch (...)
        {
            OutputDebugStringW(L"[UpdateUserStatus] Unknown exception\n");
            co_return false;
        }
    }
}

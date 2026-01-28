#include "pch.h"
#include "SupabaseClientAsync.h"
#include "BCryptPasswordHasher.h"
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Filters.h>
#include <winrt/Windows.Web.Http.Headers.h>

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

                    JsonObject loginParams;
                    loginParams.Insert(L"p_username", JsonValue::CreateStringValue(username));
                    loginParams.Insert(L"p_password", JsonValue::CreateStringValue(storedHash));
                    Uri loginUri(m_projectUrl + L"/rest/v1/rpc/login_user");
                    HttpRequestMessage loginReq(HttpMethod::Post(), loginUri);
                    loginReq.Headers().Insert(L"apikey", m_anonKey);
                    loginReq.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
                    loginReq.Content(HttpStringContent(loginParams.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

                    auto loginResponse = co_await m_httpClient.SendRequestAsync(loginReq);
                    auto loginContent = co_await loginResponse.Content().ReadAsStringAsync();
                    auto loginResult = JsonObject::Parse(loginContent);

                    OutputDebugStringW((L"Login RPC Response: " + loginContent + L"\n").c_str());

                    auto jwtToken = loginResult.GetNamedString(L"jwt_token", L"");

                    resultJson.SetNamedValue(L"success", JsonValue::CreateBooleanValue(true));
                    resultJson.Insert(L"username", JsonValue::CreateStringValue(username));
                    resultJson.Insert(L"role", JsonValue::CreateStringValue(role));
                    resultJson.Insert(L"userId", JsonValue::CreateStringValue(userId));
                    resultJson.Insert(L"jwtToken", JsonValue::CreateStringValue(jwtToken));
                    auto displayRole = (role == L"Admin") ? L"Administrator" : (role == L"Teacher") ? L"Teacher"
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
        hstring result = L"[]";

        try
        {
            OutputDebugStringW(L"[GetQuestions] START - Fetching from Supabase\n");

            HttpBaseProtocolFilter filter;
            filter.CacheControl().ReadBehavior(HttpCacheReadBehavior::MostRecent);
            filter.CacheControl().WriteBehavior(HttpCacheWriteBehavior::NoCache);
            HttpClient freshClient(filter);

            Uri uri(m_projectUrl + L"/rest/v1/questions?select=*&created_by=eq." + createdBy + L"&order=created_at.desc");
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            auto response = co_await freshClient.SendRequestAsync(request);
            OutputDebugStringW((L"[GetQuestions] HTTP Status: " + std::to_wstring((int)response.StatusCode()) + L"\n").c_str());

            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                auto content = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[GetQuestions] SUCCESS - Received " + std::to_wstring(content.size()) + L" chars\n").c_str());
                co_return content;
            }
            else
            {
                OutputDebugStringW((L"[GetQuestions] ERROR - Status: " + std::to_wstring((int)response.StatusCode()) + L"\n").c_str());
            }
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[GetQuestions] EXCEPTION: " + ex.message() + L"\n").c_str());
        }
        catch (...)
        {
            OutputDebugStringW(L"[GetQuestions] UNKNOWN ERROR\n");
        }

        OutputDebugStringW(L"[GetQuestions] Returning empty array\n");
        co_return result;
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
            OutputDebugStringW(L"[CreateQuestion] START - Using RPC function\n");

            HttpBaseProtocolFilter filter;
            filter.CacheControl().ReadBehavior(HttpCacheReadBehavior::MostRecent);
            filter.CacheControl().WriteBehavior(HttpCacheWriteBehavior::NoCache);
            HttpClient freshClient(filter);

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

            OutputDebugStringW((L"[CreateQuestion] RPC JSON: " + params.Stringify() + L"\n").c_str());

            Uri uri(m_projectUrl + L"/rest/v1/rpc/create_question_validated");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await freshClient.SendRequestAsync(request);
            auto statusCode = static_cast<int>(response.StatusCode());
            OutputDebugStringW((L"[CreateQuestion] HTTP Status: " + std::to_wstring(statusCode) + L"\n").c_str());

            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                auto content = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[CreateQuestion] Response: " + content + L"\n").c_str());

                // RPC returns {"status": "success/error", "message": "...", "question_id": "..."}
                auto rpcResult = JsonObject::Parse(content);
                auto status = rpcResult.GetNamedString(L"status", L"error");

                if (status == L"success")
                {
                    resultJson.SetNamedValue(L"success", JsonValue::CreateBooleanValue(true));
                    resultJson.Insert(L"message", JsonValue::CreateStringValue(rpcResult.GetNamedString(L"message", L"Question created successfully")));
                    OutputDebugStringW(L"[CreateQuestion] SUCCESS\n");
                }
                else
                {
                    resultJson.Insert(L"message", JsonValue::CreateStringValue(rpcResult.GetNamedString(L"message", L"Failed to create question")));
                    OutputDebugStringW((L"[CreateQuestion] RPC ERROR: " + rpcResult.GetNamedString(L"message") + L"\n").c_str());
                }
            }
            else
            {
                auto errorContent = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[CreateQuestion] HTTP Error: " + errorContent + L"\n").c_str());
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Failed to create question"));
            }
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[CreateQuestion] Exception: " + ex.message() + L"\n").c_str());
            resultJson.Insert(L"message", JsonValue::CreateStringValue(ex.message()));
        }
        catch (...)
        {
            OutputDebugStringW(L"[CreateQuestion] Unknown exception\n");
            resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Unknown error occurred"));
        }

        co_return resultJson.Stringify();
    }

    IAsyncOperation<hstring> SupabaseClientAsync::UpdateQuestionValidatedAsync(
        hstring const &id, hstring const &text,
        hstring const &optA, hstring const &optB, hstring const &optC, hstring const &optD,
        hstring const &correctOpt, hstring const &difficulty, hstring const &topic)
    {
        JsonObject resultJson;
        resultJson.Insert(L"success", JsonValue::CreateBooleanValue(false));

        try
        {
            OutputDebugStringW(L"[UpdateQuestion] START - Using RPC function\n");

            HttpBaseProtocolFilter filter;
            filter.CacheControl().ReadBehavior(HttpCacheReadBehavior::MostRecent);
            filter.CacheControl().WriteBehavior(HttpCacheWriteBehavior::NoCache);
            HttpClient freshClient(filter);

            JsonObject params;
            params.Insert(L"p_id", JsonValue::CreateStringValue(id));
            params.Insert(L"p_question_text", JsonValue::CreateStringValue(text));
            params.Insert(L"p_option_a", JsonValue::CreateStringValue(optA));
            params.Insert(L"p_option_b", JsonValue::CreateStringValue(optB));
            params.Insert(L"p_option_c", JsonValue::CreateStringValue(optC));
            params.Insert(L"p_option_d", JsonValue::CreateStringValue(optD));
            params.Insert(L"p_correct_option", JsonValue::CreateStringValue(correctOpt));
            params.Insert(L"p_difficulty_level", JsonValue::CreateStringValue(difficulty));
            params.Insert(L"p_topic", JsonValue::CreateStringValue(topic));

            OutputDebugStringW((L"[UpdateQuestion] RPC JSON: " + params.Stringify() + L"\n").c_str());

            Uri uri(m_projectUrl + L"/rest/v1/rpc/update_question_validated");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await freshClient.SendRequestAsync(request);
            auto statusCode = static_cast<int>(response.StatusCode());
            OutputDebugStringW((L"[UpdateQuestion] HTTP Status: " + std::to_wstring(statusCode) + L"\n").c_str());

            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                auto content = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[UpdateQuestion] Response: " + content + L"\n").c_str());

                auto rpcResult = JsonObject::Parse(content);
                auto status = rpcResult.GetNamedString(L"status", L"error");

                if (status == L"success")
                {
                    resultJson.SetNamedValue(L"success", JsonValue::CreateBooleanValue(true));
                    resultJson.Insert(L"message", JsonValue::CreateStringValue(rpcResult.GetNamedString(L"message", L"Question updated successfully")));
                    OutputDebugStringW(L"[UpdateQuestion] SUCCESS\n");
                }
                else
                {
                    resultJson.Insert(L"message", JsonValue::CreateStringValue(rpcResult.GetNamedString(L"message", L"Failed to update question")));
                    OutputDebugStringW((L"[UpdateQuestion] RPC ERROR: " + rpcResult.GetNamedString(L"message") + L"\n").c_str());
                }
            }
            else
            {
                auto errorContent = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[UpdateQuestion] HTTP Error: " + errorContent + L"\n").c_str());
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Failed to update question"));
            }
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[UpdateQuestion] Exception: " + ex.message() + L"\n").c_str());
            resultJson.Insert(L"message", JsonValue::CreateStringValue(ex.message()));
        }
        catch (...)
        {
            OutputDebugStringW(L"[UpdateQuestion] Unknown exception\n");
            resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Unknown error occurred"));
        }

        co_return resultJson.Stringify();
    }

    IAsyncOperation<bool> SupabaseClientAsync::DeleteQuestionSafeAsync(hstring const &questionId)
    {
        try
        {
            OutputDebugStringW(L"[DeleteQuestionSafe] START - Using RPC function\n");
            OutputDebugStringW((L"[DeleteQuestionSafe] QuestionId: '" + questionId + L"'\n").c_str());

            HttpBaseProtocolFilter filter;
            filter.CacheControl().ReadBehavior(HttpCacheReadBehavior::MostRecent);
            filter.CacheControl().WriteBehavior(HttpCacheWriteBehavior::NoCache);
            HttpClient freshClient(filter);

            JsonObject params;
            params.Insert(L"question_id_input", JsonValue::CreateStringValue(questionId));

            OutputDebugStringW((L"[DeleteQuestionSafe] RPC JSON: " + params.Stringify() + L"\n").c_str());

            Uri uri(m_projectUrl + L"/rest/v1/rpc/delete_question_safe");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await freshClient.SendRequestAsync(request);
            auto statusCode = static_cast<int>(response.StatusCode());
            OutputDebugStringW((L"[DeleteQuestionSafe] HTTP Status: " + std::to_wstring(statusCode) + L"\n").c_str());

            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                auto content = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[DeleteQuestionSafe] Response: " + content + L"\n").c_str());

                // RPC returns {"status": "success/error/blocked", "message": "..."}
                auto rpcResult = JsonObject::Parse(content);
                auto status = rpcResult.GetNamedString(L"status", L"error");

                if (status == L"success")
                {
                    OutputDebugStringW(L"[DeleteQuestionSafe] SUCCESS\n");
                    co_return true;
                }
                else
                {
                    OutputDebugStringW((L"[DeleteQuestionSafe] RPC returned: " + status + L" - " + rpcResult.GetNamedString(L"message") + L"\n").c_str());
                    co_return false;
                }
            }
            else
            {
                auto errorContent = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[DeleteQuestionSafe] HTTP Error: " + errorContent + L"\n").c_str());
                co_return false;
            }
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[DeleteQuestionSafe] Exception: " + ex.message() + L"\n").c_str());
            co_return false;
        }
        catch (...)
        {
            OutputDebugStringW(L"[DeleteQuestionSafe] Unknown exception\n");
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

    IAsyncOperation<bool> SupabaseClientAsync::UpdateUserRoleAsync(hstring const &userId, hstring const &newRole)
    {
        try
        {
            OutputDebugStringW(L"[UpdateUserRole] START\n");
            auto trimmedUserId = TrimUserId(userId);
            OutputDebugStringW((L"[UpdateUserRole] UserId: " + trimmedUserId + L"\n").c_str());
            OutputDebugStringW((L"[UpdateUserRole] New role: " + std::wstring(newRole) + L"\n").c_str());

            JsonObject params;
            params.Insert(L"role", JsonValue::CreateStringValue(newRole));

            auto uriString = m_projectUrl + L"/rest/v1/users?id=eq." + trimmedUserId;
            OutputDebugStringW((L"[UpdateUserRole] URI: " + uriString + L"\n").c_str());
            OutputDebugStringW((L"[UpdateUserRole] Body: " + params.Stringify() + L"\n").c_str());

            Uri uri(uriString);
            HttpRequestMessage request(HttpMethod::Patch(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Headers().Insert(L"Prefer", L"return=minimal");
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            OutputDebugStringW(L"[UpdateUserRole] Sending PATCH request...\n");
            auto response = co_await m_httpClient.SendRequestAsync(request);
            auto statusCode = response.StatusCode();
            OutputDebugStringW((L"[UpdateUserRole] HTTP Status: " + std::to_wstring(static_cast<int>(statusCode)) + L"\n").c_str());

            auto responseBody = co_await response.Content().ReadAsStringAsync();
            OutputDebugStringW((L"[UpdateUserRole] Response: " + responseBody + L"\n").c_str());

            bool success = statusCode == HttpStatusCode::NoContent || statusCode == HttpStatusCode::Ok;
            OutputDebugStringW((L"[UpdateUserRole] Result: " + std::wstring(success ? L"SUCCESS" : L"FAILED") + L"\n").c_str());
            co_return success;
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[UpdateUserRole] Exception: " + ex.message() + L"\n").c_str());
            co_return false;
        }
        catch (...)
        {
            OutputDebugStringW(L"[UpdateUserRole] Unknown exception\n");
            co_return false;
        }
    }

    IAsyncOperation<hstring> SupabaseClientAsync::GetQuizzesJsonAsync(hstring const &createdBy)
    {
        try
        {
            auto trimmedUserId = TrimUserId(createdBy);
            OutputDebugStringW((L"[GetQuizzes] Fetching quizzes for teacher: " + trimmedUserId + L"\n").c_str());

            HttpBaseProtocolFilter filter;
            filter.CacheControl().ReadBehavior(HttpCacheReadBehavior::MostRecent);
            filter.CacheControl().WriteBehavior(HttpCacheWriteBehavior::NoCache);
            HttpClient freshClient(filter);

            auto uriString = m_projectUrl + L"/rest/v1/quizzes?created_by=eq." + trimmedUserId + 
                           L"&select=id,title,time_limit_minutes,total_points,max_attempts,result_visibility,shuffle_questions,shuffle_answers,created_at";
            
            Uri uri(uriString);
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            auto response = co_await freshClient.SendRequestAsync(request);
            auto content = co_await response.Content().ReadAsStringAsync();

            if (response.StatusCode() != HttpStatusCode::Ok)
            {
                OutputDebugStringW((L"[GetQuizzes] Error: " + content + L"\n").c_str());
                co_return L"[]";
            }

            auto quizzesArray = JsonArray::Parse(content);
            JsonArray enrichedQuizzes;

            for (uint32_t i = 0; i < quizzesArray.Size(); ++i)
            {
                auto quizObj = quizzesArray.GetObjectAt(i);
                auto quizId = quizObj.GetNamedString(L"id", L"");

                auto countUri = m_projectUrl + L"/rest/v1/quiz_questions?quiz_id=eq." + quizId + L"&select=id";
                HttpRequestMessage countRequest(HttpMethod::Get(), Uri(countUri));
                countRequest.Headers().Insert(L"apikey", m_anonKey);
                countRequest.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

                auto countResponse = co_await freshClient.SendRequestAsync(countRequest);
                auto countContent = co_await countResponse.Content().ReadAsStringAsync();
                auto countArray = JsonArray::Parse(countContent);

                quizObj.Insert(L"question_count", JsonValue::CreateNumberValue(static_cast<double>(countArray.Size())));
                enrichedQuizzes.Append(quizObj);
            }

            OutputDebugStringW((L"[GetQuizzes] SUCCESS - " + std::to_wstring(enrichedQuizzes.Size()) + L" quizzes\n").c_str());
            co_return enrichedQuizzes.Stringify();
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[GetQuizzes] Exception: " + ex.message() + L"\n").c_str());
            co_return L"[]";
        }
    }

    IAsyncOperation<hstring> SupabaseClientAsync::CreateQuizFullAsync(
        hstring const &quizId,
        hstring const &teacherId,
        hstring const &title,
        int32_t timeLimitMinutes,
        int32_t totalPoints,
        hstring const &maxAttempts,
        hstring const &resultVisibility,
        bool shuffleQuestions,
        bool shuffleAnswers,
        hstring const &questionIdsJson)
    {
        JsonObject resultJson;
        resultJson.Insert(L"success", JsonValue::CreateBooleanValue(false));

        try
        {
            auto trimmedTeacherId = TrimUserId(teacherId);
            auto trimmedQuizId = TrimUserId(quizId);
            
            OutputDebugStringW((L"[CreateQuizFull] Creating quiz: " + trimmedQuizId + L"\n").c_str());
            OutputDebugStringW((L"[CreateQuizFull] Teacher: " + trimmedTeacherId + L"\n").c_str());
            OutputDebugStringW((L"[CreateQuizFull] Title: " + title + L"\n").c_str());

            JsonObject rpcParams;
            rpcParams.Insert(L"p_quiz_id", JsonValue::CreateStringValue(trimmedQuizId));
            rpcParams.Insert(L"p_teacher_id", JsonValue::CreateStringValue(trimmedTeacherId));
            rpcParams.Insert(L"p_title", JsonValue::CreateStringValue(title));
            rpcParams.Insert(L"p_time_limit", JsonValue::CreateNumberValue(timeLimitMinutes));
            rpcParams.Insert(L"p_total_points", JsonValue::CreateNumberValue(totalPoints));
            rpcParams.Insert(L"p_max_attempts", JsonValue::CreateStringValue(maxAttempts));
            rpcParams.Insert(L"p_result_visibility", JsonValue::CreateStringValue(resultVisibility));
            rpcParams.Insert(L"p_shuffle_questions", JsonValue::CreateBooleanValue(shuffleQuestions));
            rpcParams.Insert(L"p_shuffle_answers", JsonValue::CreateBooleanValue(shuffleAnswers));
            rpcParams.Insert(L"p_question_ids_json", JsonValue::CreateStringValue(questionIdsJson));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/create_quiz_full");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(rpcParams.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await m_httpClient.SendRequestAsync(request);
            auto content = co_await response.Content().ReadAsStringAsync();

            OutputDebugStringW((L"[CreateQuizFull] Response: " + content + L"\n").c_str());

            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                auto responseObj = JsonObject::Parse(content);
                co_return responseObj.Stringify();
            }
            else
            {
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Failed to create quiz"));
                co_return resultJson.Stringify();
            }
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[CreateQuizFull] Exception: " + ex.message() + L"\n").c_str());
            resultJson.Insert(L"message", JsonValue::CreateStringValue(ex.message()));
            co_return resultJson.Stringify();
        }
    }

    IAsyncOperation<hstring> SupabaseClientAsync::UpdateQuizAsync(
        hstring const &quizId,
        hstring const &title,
        int32_t timeLimitMinutes,
        int32_t totalPoints,
        hstring const &maxAttempts,
        hstring const &resultVisibility,
        bool shuffleQuestions,
        bool shuffleAnswers)
    {
        JsonObject resultJson;
        resultJson.Insert(L"success", JsonValue::CreateBooleanValue(false));

        try
        {
            auto trimmedQuizId = TrimUserId(quizId);
            OutputDebugStringW((L"[UpdateQuiz] Updating quiz: " + trimmedQuizId + L"\n").c_str());

            JsonObject params;
            params.Insert(L"title", JsonValue::CreateStringValue(title));
            params.Insert(L"time_limit_minutes", JsonValue::CreateNumberValue(timeLimitMinutes));
            params.Insert(L"total_points", JsonValue::CreateNumberValue(totalPoints));
            params.Insert(L"max_attempts", JsonValue::CreateStringValue(maxAttempts));
            params.Insert(L"result_visibility", JsonValue::CreateStringValue(resultVisibility));
            params.Insert(L"shuffle_questions", JsonValue::CreateBooleanValue(shuffleQuestions));
            params.Insert(L"shuffle_answers", JsonValue::CreateBooleanValue(shuffleAnswers));

            Uri uri(m_projectUrl + L"/rest/v1/quizzes?id=eq." + trimmedQuizId);
            HttpRequestMessage request(HttpMethod::Patch(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await m_httpClient.SendRequestAsync(request);

            if (response.StatusCode() == HttpStatusCode::NoContent || response.StatusCode() == HttpStatusCode::Ok)
            {
                resultJson.SetNamedValue(L"success", JsonValue::CreateBooleanValue(true));
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Quiz updated successfully"));
            }
            else
            {
                auto content = co_await response.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[UpdateQuiz] Error: " + content + L"\n").c_str());
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Failed to update quiz"));
            }

            co_return resultJson.Stringify();
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[UpdateQuiz] Exception: " + ex.message() + L"\n").c_str());
            resultJson.Insert(L"message", JsonValue::CreateStringValue(ex.message()));
            co_return resultJson.Stringify();
        }
    }

    IAsyncOperation<hstring> SupabaseClientAsync::DeleteQuizSafeAsync(hstring const &quizId)
    {
        JsonObject resultJson;
        resultJson.Insert(L"success", JsonValue::CreateBooleanValue(false));

        try
        {
            auto trimmedQuizId = TrimUserId(quizId);
            OutputDebugStringW((L"[DeleteQuizSafe] Checking quiz: " + trimmedQuizId + L"\n").c_str());

            auto checkUri = m_projectUrl + L"/rest/v1/quiz_attempts?quiz_id=eq." + trimmedQuizId + L"&select=id";
            HttpRequestMessage checkRequest(HttpMethod::Get(), Uri(checkUri));
            checkRequest.Headers().Insert(L"apikey", m_anonKey);
            checkRequest.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            auto checkResponse = co_await m_httpClient.SendRequestAsync(checkRequest);
            auto checkContent = co_await checkResponse.Content().ReadAsStringAsync();
            auto attemptsArray = JsonArray::Parse(checkContent);

            if (attemptsArray.Size() > 0)
            {
                OutputDebugStringW(L"[DeleteQuizSafe] Quiz has attempts, cannot delete\n");
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Cannot delete quiz with student attempts. Contact admin."));
                co_return resultJson.Stringify();
            }

            Uri deleteUri(m_projectUrl + L"/rest/v1/quizzes?id=eq." + trimmedQuizId);
            HttpRequestMessage deleteRequest(HttpMethod::Delete(), deleteUri);
            deleteRequest.Headers().Insert(L"apikey", m_anonKey);
            deleteRequest.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            auto deleteResponse = co_await m_httpClient.SendRequestAsync(deleteRequest);

            if (deleteResponse.StatusCode() == HttpStatusCode::NoContent)
            {
                resultJson.SetNamedValue(L"success", JsonValue::CreateBooleanValue(true));
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Quiz deleted successfully"));
            }
            else
            {
                auto content = co_await deleteResponse.Content().ReadAsStringAsync();
                OutputDebugStringW((L"[DeleteQuizSafe] Error: " + content + L"\n").c_str());
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Failed to delete quiz"));
            }

            co_return resultJson.Stringify();
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[DeleteQuizSafe] Exception: " + ex.message() + L"\n").c_str());
            resultJson.Insert(L"message", JsonValue::CreateStringValue(ex.message()));
            co_return resultJson.Stringify();
        }
    }

    IAsyncOperation<hstring> SupabaseClientAsync::GetQuizAttemptsForReviewAsync(hstring const &quizId)
    {
        try
        {
            JsonObject params;
            params.Insert(L"p_quiz_id", JsonValue::CreateStringValue(quizId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/get_quiz_attempts_for_review");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await m_httpClient.SendRequestAsync(request);
            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                co_return co_await response.Content().ReadAsStringAsync();
            }
        }
        catch (...)
        {
        }

        co_return L"[]";
    }

    IAsyncOperation<hstring> SupabaseClientAsync::DeleteQuizAttemptAsync(hstring const &attemptId, hstring const &teacherId)
    {
        JsonObject resultJson;
        resultJson.Insert(L"success", JsonValue::CreateBooleanValue(false));

        try
        {
            auto trimmedTeacherId = TrimUserId(teacherId);

            JsonObject params;
            params.Insert(L"p_attempt_id", JsonValue::CreateStringValue(attemptId));
            params.Insert(L"p_teacher_id", JsonValue::CreateStringValue(trimmedTeacherId));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/delete_quiz_attempt");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await m_httpClient.SendRequestAsync(request);
            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                co_return co_await response.Content().ReadAsStringAsync();
            }
            else
            {
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Failed to delete attempt"));
            }
        }
        catch (hresult_error const &ex)
        {
            resultJson.Insert(L"message", JsonValue::CreateStringValue(ex.message()));
        }

        co_return resultJson.Stringify();
    }

    IAsyncOperation<hstring> SupabaseClientAsync::ToggleResultReleaseAsync(hstring const &quizId, hstring const &teacherId, bool released)
    {
        JsonObject resultJson;
        resultJson.Insert(L"success", JsonValue::CreateBooleanValue(false));

        try
        {
            auto trimmedTeacherId = TrimUserId(teacherId);

            JsonObject params;
            params.Insert(L"p_quiz_id", JsonValue::CreateStringValue(quizId));
            params.Insert(L"p_teacher_id", JsonValue::CreateStringValue(trimmedTeacherId));
            params.Insert(L"p_released", JsonValue::CreateBooleanValue(released));

            Uri uri(m_projectUrl + L"/rest/v1/rpc/toggle_result_release");
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);
            request.Content(HttpStringContent(params.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

            auto response = co_await m_httpClient.SendRequestAsync(request);
            if (response.StatusCode() == HttpStatusCode::Ok)
            {
                co_return co_await response.Content().ReadAsStringAsync();
            }
            else
            {
                resultJson.Insert(L"message", JsonValue::CreateStringValue(L"Failed to update result release status"));
            }
        }
        catch (hresult_error const &ex)
        {
            resultJson.Insert(L"message", JsonValue::CreateStringValue(ex.message()));
        }

        co_return resultJson.Stringify();
    }

    IAsyncOperation<hstring> SupabaseClientAsync::GetStudentHistoryAsync(hstring const &studentId)
    {
        try
        {
            auto trimmedStudentId = TrimUserId(studentId);
            OutputDebugStringW((L"[GetStudentHistory] Fetching history for student: " + trimmedStudentId + L"\n").c_str());

            // Query quiz_attempts with quiz title join
            auto uriString = m_projectUrl + L"/rest/v1/quiz_attempts?student_id=eq." + trimmedStudentId + 
                            L"&select=id,quiz_id,attempt_number,score,total_points,correct_count,incorrect_count,time_spent_seconds,status,submitted_at,quizzes(title)&order=submitted_at.desc";

            Uri uri(uriString);
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Insert(L"apikey", m_anonKey);
            request.Headers().Insert(L"Authorization", hstring(L"Bearer ") + m_anonKey);

            auto response = co_await m_httpClient.SendRequestAsync(request);
            auto statusCode = response.StatusCode();
            OutputDebugStringW((L"[GetStudentHistory] HTTP Status: " + to_hstring(static_cast<int>(statusCode)) + L"\n").c_str());

            if (statusCode == HttpStatusCode::Ok)
            {
                auto responseBody = co_await response.Content().ReadAsStringAsync();
                
                // Parse and transform to include quiz_title at top level
                JsonArray inputArray;
                if (JsonArray::TryParse(responseBody, inputArray))
                {
                    JsonArray outputArray;
                    for (uint32_t i = 0; i < inputArray.Size(); i++)
                    {
                        auto obj = inputArray.GetAt(i).GetObject();
                        JsonObject newObj;

                        newObj.Insert(L"id", obj.GetNamedValue(L"id"));
                        newObj.Insert(L"quiz_id", obj.GetNamedValue(L"quiz_id"));
                        newObj.Insert(L"attempt_number", obj.GetNamedValue(L"attempt_number"));
                        newObj.Insert(L"score", obj.GetNamedValue(L"score"));
                        newObj.Insert(L"total_points", obj.GetNamedValue(L"total_points"));
                        newObj.Insert(L"correct_count", obj.GetNamedValue(L"correct_count"));
                        newObj.Insert(L"incorrect_count", obj.GetNamedValue(L"incorrect_count"));
                        newObj.Insert(L"time_spent_seconds", obj.GetNamedValue(L"time_spent_seconds"));
                        newObj.Insert(L"status", obj.GetNamedValue(L"status"));
                        newObj.Insert(L"submitted_at", obj.GetNamedValue(L"submitted_at"));

                        // Extract quiz title from nested object
                        auto quizzesVal = obj.GetNamedValue(L"quizzes");
                        if (quizzesVal.ValueType() == JsonValueType::Object)
                        {
                            auto quizzesObj = quizzesVal.GetObject();
                            newObj.Insert(L"quiz_title", quizzesObj.GetNamedValue(L"title"));
                        }
                        else
                        {
                            newObj.Insert(L"quiz_title", JsonValue::CreateStringValue(L"Unknown Quiz"));
                        }

                        outputArray.Append(newObj);
                    }
                    co_return outputArray.Stringify();
                }
                co_return responseBody;
            }
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[GetStudentHistory] Error: " + ex.message() + L"\n").c_str());
        }

        co_return L"[]";
    }
}


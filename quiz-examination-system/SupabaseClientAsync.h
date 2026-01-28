#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Data.Json.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Data::Json;

namespace quiz_examination_system
{
    struct LoginResult
    {
        bool success;
        hstring username;
        hstring displayRole;
        hstring role;
        hstring userId;
        hstring errorMessage;
    };

    struct QuestionData
    {
        hstring quiz_question_id;
        hstring question_id;
        hstring question_text;
        hstring option_a;
        hstring option_b;
        hstring option_c;
        hstring option_d;
        hstring difficulty_level;
        int points;
        int order_num;
    };

    struct QuizData
    {
        hstring quiz_id;
        hstring quiz_title;
        int time_limit_minutes;
        int total_points;
        hstring max_attempts;
        int attempts_used;
        hstring result_visibility;
    };

    struct QuestionCreateResult
    {
        bool success;
        hstring message;
    };

    class SupabaseClientAsync
    {
    public:
        SupabaseClientAsync();

        IAsyncOperation<hstring> LoginAsync(hstring const &username, hstring const &password);

        IAsyncOperation<bool> ChangePasswordAsync(hstring const &username, hstring const &currentPassword, hstring const &newPassword);

        IAsyncOperation<hstring> GetQuestionsJsonAsync(hstring const &createdBy);

        IAsyncOperation<hstring> CreateQuestionValidatedAsync(
            hstring const &id,
            hstring const &teacherId,
            hstring const &text,
            hstring const &optA, hstring const &optB, hstring const &optC, hstring const &optD,
            hstring const &correctOpt,
            hstring const &difficulty,
            hstring const &topic);

        IAsyncOperation<bool> DeleteQuestionSafeAsync(hstring const &questionId);

        IAsyncOperation<hstring> UpdateQuestionValidatedAsync(
            hstring const &id,
            hstring const &text,
            hstring const &optA, hstring const &optB, hstring const &optC, hstring const &optD,
            hstring const &correctOpt,
            hstring const &difficulty,
            hstring const &topic);

        IAsyncOperation<hstring> GetQuizQuestionsJsonAsync(hstring const &quizId);

        IAsyncOperation<hstring> GetStudentQuizzesJsonAsync(hstring const &studentId);

        IAsyncOperation<hstring> GetAllUsersAsync();

        IAsyncOperation<bool> CreateUserAsync(hstring const &adminUsername, hstring const &newUsername, hstring const &hashedPassword, hstring const &role);

        IAsyncOperation<bool> ResetUserPasswordAsync(hstring const &userId, hstring const &hashedPassword);

        IAsyncOperation<bool> UpdateUserStatusAsync(hstring const &userId, bool isActive);

        bool IsConnected() const { return m_connected; }
        hstring GetLastError() const { return m_lastError; }

    private:
        HttpClient m_httpClient;
        bool m_connected;
        hstring m_lastError;

        hstring m_projectUrl{L"https://tuciofxdzzrzwzqsltps.supabase.co"};
        hstring m_anonKey{L"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI"};
    };
}

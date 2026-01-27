#include "pch.h"
#include "AllQuizzesManagementPage.xaml.h"
#if __has_include("AllQuizzesManagementPage.g.cpp")
#include "AllQuizzesManagementPage.g.cpp"
#endif
#include "QuizManagementItem.h"
#include "SupabaseClientManager.h"
#include <algorithm>
#include <sstream>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Filters.h>
#include <winrt/Windows.Web.Http.Headers.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Windows::Data::Json;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Headers;

namespace winrt::quiz_examination_system::implementation
{
    AllQuizzesManagementPage::AllQuizzesManagementPage()
    {
        InitializeComponent();
        m_quizzes = single_threaded_observable_vector<quiz_examination_system::QuizManagementItem>();
        LoadQuizzes();
    }

    AllQuizzesManagementPage::~AllQuizzesManagementPage()
    {
        if (m_messageTimer)
        {
            m_messageTimer.Stop();
            m_messageTimer = nullptr;
        }
    }

    winrt::fire_and_forget AllQuizzesManagementPage::LoadQuizzes()
    {
        auto lifetime = get_strong();

        OutputDebugStringW(L"[LoadQuizzes] Starting to load quiz list\n");
        LoadingRing().IsActive(true);

        try
        {
            hstring query = L"select=id,title,created_at,created_by,users!quizzes_created_by_fkey(username)&order=created_at.desc";
            hstring uriString = L"https://tuciofxdzzrzwzqsltps.supabase.co/rest/v1/quizzes?" + query;

            Uri uri(uriString);
            HttpRequestMessage request(HttpMethod::Get(), uri);
            request.Headers().Append(L"apikey", L"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");
            request.Headers().Append(L"Authorization", L"Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");

            HttpClient client;
            auto response = co_await client.SendRequestAsync(request);
            auto content = co_await response.Content().ReadAsStringAsync();

            OutputDebugStringW((L"[LoadQuizzes] Response: " + std::wstring(content).substr(0, 200) + L"...\n").c_str());

            auto quizzesArray = JsonArray::Parse(content);
            OutputDebugStringW((L"[LoadQuizzes] Parsed " + std::to_wstring(quizzesArray.Size()) + L" quizzes\n").c_str());

            m_allQuizzes.clear();
            m_quizzes.Clear();

            for (uint32_t i = 0; i < quizzesArray.Size(); i++)
            {
                auto quizObj = quizzesArray.GetObjectAt(i);
                auto id = quizObj.GetNamedString(L"id");
                auto title = quizObj.GetNamedString(L"title");
                auto createdAt = quizObj.GetNamedString(L"created_at");

                hstring teacherName = L"Unknown";
                if (quizObj.HasKey(L"users"))
                {
                    auto usersValue = quizObj.GetNamedValue(L"users");
                    if (usersValue.ValueType() == JsonValueType::Object)
                    {
                        auto userObj = quizObj.GetNamedObject(L"users");
                        teacherName = userObj.GetNamedString(L"username", L"Unknown");
                    }
                }

                hstring attemptUriString = L"https://tuciofxdzzrzwzqsltps.supabase.co/rest/v1/quiz_attempts?select=id&quiz_id=eq." + id;
                Uri attemptUri(attemptUriString);
                HttpRequestMessage attemptRequest(HttpMethod::Get(), attemptUri);
                attemptRequest.Headers().Append(L"apikey", L"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");
                attemptRequest.Headers().Append(L"Authorization", L"Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");

                auto attemptResponse = co_await client.SendRequestAsync(attemptRequest);
                auto attemptContent = co_await attemptResponse.Content().ReadAsStringAsync();
                auto attemptsArray = JsonArray::Parse(attemptContent);
                int32_t attemptCount = static_cast<int32_t>(attemptsArray.Size());

                std::wstring dateStr(createdAt);
                hstring formattedDate = dateStr.size() >= 10 ? hstring(dateStr.substr(0, 10)) : createdAt;

                auto quizItem = make<QuizManagementItem>(id, title, teacherName, formattedDate, attemptCount);
                m_allQuizzes.push_back(quizItem);
                m_quizzes.Append(quizItem);
            }

            OutputDebugStringW((L"[LoadQuizzes] Success - Added " + std::to_wstring(m_quizzes.Size()) + L" quizzes\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[LoadQuizzes] Error: " + ex.message() + L"\n").c_str());
            ShowMessage(L"Error", L"Failed to load quiz list: " + ex.message(), InfoBarSeverity::Error);
        }

        LoadingRing().IsActive(false);
    }

    void AllQuizzesManagementPage::OnSearchTextChanged(AutoSuggestBox const &sender, AutoSuggestBoxTextChangedEventArgs const &)
    {
        FilterQuizzes(sender.Text());
    }

    void AllQuizzesManagementPage::FilterQuizzes(hstring const &searchText)
    {
        m_quizzes.Clear();

        auto lowerSearch = to_string(searchText);
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

        for (auto const &quiz : m_allQuizzes)
        {
            if (searchText.empty())
            {
                m_quizzes.Append(quiz);
            }
            else
            {
                auto title = to_string(quiz.Title());
                auto teacher = to_string(quiz.TeacherName());
                std::transform(title.begin(), title.end(), title.begin(), ::tolower);
                std::transform(teacher.begin(), teacher.end(), teacher.begin(), ::tolower);

                if (title.find(lowerSearch) != std::string::npos || teacher.find(lowerSearch) != std::string::npos)
                {
                    m_quizzes.Append(quiz);
                }
            }
        }
    }

    void AllQuizzesManagementPage::OnPurgeClicked(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto button = sender.as<Button>();
        auto quizId = unbox_value<hstring>(button.Tag());

        for (auto const &quiz : m_allQuizzes)
        {
            if (quiz.QuizId() == quizId)
            {
                PurgeQuiz(quizId, quiz.AttemptCount());
                break;
            }
        }
    }

    winrt::fire_and_forget AllQuizzesManagementPage::PurgeQuiz(hstring quizId, int32_t attemptCount)
    {
        auto lifetime = get_strong();

        ContentDialog dialog;
        dialog.XamlRoot(this->XamlRoot());
        dialog.Title(box_value(L"Critical warning"));
        dialog.PrimaryButtonText(L"Confirm purge");
        dialog.CloseButtonText(L"Cancel");
        dialog.DefaultButton(ContentDialogButton::Close);

        StackPanel panel;
        panel.Spacing(12);

        TextBlock warningText;
        if (attemptCount > 0)
        {
            warningText.Text(L"Warning: This quiz contains " + std::to_wstring(attemptCount) +
                             L" student attempts. Deleting will permanently remove all scores and attempt history. This action cannot be undone.");
        }
        else
        {
            warningText.Text(L"Confirm deletion of this quiz from the system. This action cannot be undone.");
        }
        warningText.TextWrapping(TextWrapping::Wrap);
        warningText.Foreground(Media::SolidColorBrush(Microsoft::UI::Colors::Red()));
        panel.Children().Append(warningText);

        dialog.Content(panel);

        auto result = co_await dialog.ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            LoadingRing().IsActive(true);

            try
            {
                HttpClient client;

                OutputDebugStringW((L"[PurgeQuiz] Deleting quiz_id: " + quizId + L"\n").c_str());
                Uri answersUri(L"https://tuciofxdzzrzwzqsltps.supabase.co/rest/v1/rpc/purge_quiz_cascade");
                HttpRequestMessage answersRequest(HttpMethod::Post(), answersUri);
                answersRequest.Headers().Append(L"apikey", L"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");
                answersRequest.Headers().Append(L"Authorization", L"Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");
                answersRequest.Headers().Append(L"Content-Type", L"application/json");

                JsonObject payload;
                payload.SetNamedValue(L"target_quiz_id", JsonValue::CreateStringValue(quizId));
                answersRequest.Content(HttpStringContent(payload.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

                auto response = co_await client.SendRequestAsync(answersRequest);

                if (response.IsSuccessStatusCode())
                {
                    OutputDebugStringW(L"[PurgeQuiz] Deleted successfully\n");
                    ShowMessage(L"Success", L"Quiz and all related data have been deleted", InfoBarSeverity::Success);

                    InsertAuditLog(L"PURGE_QUIZ", L"quizzes", quizId,
                                   L"Purged quiz ID: " + quizId + L" causing cascade delete of " +
                                       to_hstring(attemptCount) + L" attempts");

                    LoadQuizzes();
                }
                else
                {
                    auto errorContent = co_await response.Content().ReadAsStringAsync();
                    OutputDebugStringW((L"[PurgeQuiz] Error: " + errorContent + L"\n").c_str());
                    ShowMessage(L"Error", L"Failed to delete quiz", InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                OutputDebugStringW((L"[PurgeQuiz] Exception: " + ex.message() + L"\n").c_str());
                ShowMessage(L"Error", L"Error during deletion: " + ex.message(), InfoBarSeverity::Error);
            }

            LoadingRing().IsActive(false);
        }
    }

    void AllQuizzesManagementPage::ShowMessage(hstring const &title, hstring const &message, InfoBarSeverity severity)
    {
        if (m_messageTimer)
        {
            m_messageTimer.Stop();
            m_messageTimer = nullptr;
        }

        ActionMessage().Title(title);
        ActionMessage().Message(message);
        ActionMessage().Severity(severity);
        ActionMessage().IsOpen(true);

        m_messageTimer = DispatcherQueue().CreateTimer();
        m_messageTimer.Interval(std::chrono::seconds(5));
        m_messageTimer.IsRepeating(false);
        m_messageTimer.Tick([weak_this = get_weak()](auto const &, auto const &)
                            {
            if (auto strong_this = weak_this.get())
            {
                strong_this->ActionMessage().IsOpen(false);
                if (strong_this->m_messageTimer)
                {
                    strong_this->m_messageTimer.Stop();
                    strong_this->m_messageTimer = nullptr;
                }
            } });
        m_messageTimer.Start();
    }

    winrt::fire_and_forget AllQuizzesManagementPage::InsertAuditLog(hstring action, hstring targetTable, hstring targetId, hstring details)
    {
        auto lifetime = get_strong();

        try
        {
            auto actorUsername = ::quiz_examination_system::SupabaseClientManager::GetInstance().GetUsername();
            auto actorId = ::quiz_examination_system::SupabaseClientManager::GetInstance().GetUserId();

            JsonObject logData;
            logData.SetNamedValue(L"action", JsonValue::CreateStringValue(action));
            logData.SetNamedValue(L"actor_id", JsonValue::CreateStringValue(actorId));

            if (!targetTable.empty())
            {
                logData.SetNamedValue(L"target_table", JsonValue::CreateStringValue(targetTable));
            }

            if (!targetId.empty())
            {
                logData.SetNamedValue(L"target_id", JsonValue::CreateStringValue(targetId));
            }

            JsonObject detailsObj;
            detailsObj.SetNamedValue(L"description", JsonValue::CreateStringValue(details));
            logData.SetNamedValue(L"details", detailsObj);

            hstring jsonBody = logData.Stringify();

            hstring uriString = L"https://tuciofxdzzrzwzqsltps.supabase.co/rest/v1/audit_logs";
            Uri uri(uriString);
            HttpRequestMessage request(HttpMethod::Post(), uri);
            request.Headers().Append(L"apikey", L"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");
            request.Headers().Append(L"Authorization", L"Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");
            request.Headers().Insert(L"Content-Type", L"application/json");

            auto bodyString = Windows::Storage::Streams::InMemoryRandomAccessStream();
            auto writer = Windows::Storage::Streams::DataWriter(bodyString);
            writer.WriteString(jsonBody);
            co_await writer.StoreAsync();
            writer.DetachStream();
            bodyString.Seek(0);

            request.Content(HttpStreamContent(bodyString));

            HttpClient client;
            auto response = co_await client.SendRequestAsync(request);

            OutputDebugStringW((L"[InsertAuditLog] Log inserted: " + action + L"\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[InsertAuditLog] ERROR: " + ex.message() + L"\n").c_str());
        }
        catch (...)
        {
            OutputDebugStringW(L"[InsertAuditLog] Unknown error\n");
        }
    }
}

#include "pch.h"
#include "AllQuizzesManagementPage.xaml.h"
#if __has_include("AllQuizzesManagementPage.g.cpp")
#include "AllQuizzesManagementPage.g.cpp"
#endif
#include "QuizManagementItem.h"
#include "SupabaseClientManager.h"
#include "PageHelper.h"
#include "HttpHelper.h"
#include "SupabaseConfig.h"
#include <algorithm>

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

    winrt::fire_and_forget AllQuizzesManagementPage::LoadQuizzes()
    {
        auto lifetime = get_strong();

        OutputDebugStringW(L"[LoadQuizzes] Starting to load quiz list\n");
        LoadingRingCenter().IsActive(true);
        EmptyStateText().Visibility(Visibility::Collapsed);

        try
        {
            auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
            hstring currentUserId = manager.GetUserId();

            if (currentUserId.empty())
            {
                ShowMessage(L"Error", L"User not logged in", InfoBarSeverity::Error);
                LoadingRingCenter().IsActive(false);
                co_return;
            }

            hstring query = L"select=id,title,created_by,time_limit_minutes,total_points,created_at&created_by=eq." + currentUserId + L"&order=created_at.desc";
            hstring endpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(L"quizzes?" + query);
            hstring content = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(endpoint, L"", HttpMethod::Get());

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
                auto createdBy = quizObj.GetNamedString(L"created_by");
                auto timeLimitMinutes = static_cast<int32_t>(quizObj.GetNamedNumber(L"time_limit_minutes"));
                auto totalPoints = static_cast<int32_t>(quizObj.GetNamedNumber(L"total_points"));
                auto createdAt = quizObj.GetNamedString(L"created_at");

                hstring questionEndpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(L"quiz_questions?select=id&quiz_id=eq." + id);
                hstring questionContent = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(questionEndpoint, L"", HttpMethod::Get());
                auto questionsArray = JsonArray::Parse(questionContent);
                int32_t questionCount = static_cast<int32_t>(questionsArray.Size());

                std::wstring dateStr(createdAt);
                hstring formattedDate = dateStr.size() >= 10 ? hstring(dateStr.substr(0, 10)) : createdAt;

                auto quizItem = make<QuizManagementItem>(id, title, createdBy, timeLimitMinutes, totalPoints, questionCount, formattedDate);
                m_allQuizzes.push_back(quizItem);
                m_quizzes.Append(quizItem);
            }

            if (m_quizzes.Size() == 0)
            {
                EmptyStateText().Visibility(Visibility::Visible);
            }

            OutputDebugStringW((L"[LoadQuizzes] Success - Added " + std::to_wstring(m_quizzes.Size()) + L" quizzes\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[LoadQuizzes] Error: " + ex.message() + L"\n").c_str());
            ShowMessage(L"Error", L"Failed to load quiz list: " + ex.message(), InfoBarSeverity::Error);
        }

        LoadingRingCenter().IsActive(false);
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
                std::transform(title.begin(), title.end(), title.begin(), ::tolower);

                if (title.find(lowerSearch) != std::string::npos)
                {
                    m_quizzes.Append(quiz);
                }
            }
        }

        EmptyStateText().Visibility(m_quizzes.Size() == 0 ? Visibility::Visible : Visibility::Collapsed);
    }

    void AllQuizzesManagementPage::OnRefreshClicked(IInspectable const &, RoutedEventArgs const &)
    {
        LoadQuizzes();
    }

    void AllQuizzesManagementPage::OnCreateQuizClicked(IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Info", L"Quiz editor page coming soon", InfoBarSeverity::Informational);
    }

    void AllQuizzesManagementPage::OnEditClicked(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto button = sender.as<Button>();
        auto quizId = unbox_value<hstring>(button.Tag());
        ShowMessage(L"Info", L"Edit quiz: " + quizId, InfoBarSeverity::Informational);
    }

    void AllQuizzesManagementPage::OnAssignClicked(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto button = sender.as<Button>();
        auto quizId = unbox_value<hstring>(button.Tag());
        ShowMessage(L"Info", L"Assign quiz: " + quizId, InfoBarSeverity::Informational);
    }

    void AllQuizzesManagementPage::OnDeleteClicked(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto button = sender.as<Button>();
        auto quizId = unbox_value<hstring>(button.Tag());
        DeleteQuiz(quizId);
    }

    winrt::fire_and_forget AllQuizzesManagementPage::DeleteQuiz(hstring quizId)
    {
        auto lifetime = get_strong();

        try
        {
            hstring attemptEndpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(L"quiz_attempts?select=id&quiz_id=eq." + quizId);
            hstring attemptContent = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(attemptEndpoint, L"", HttpMethod::Get());
            auto attemptsArray = JsonArray::Parse(attemptContent);

            if (attemptsArray.Size() > 0)
            {
                ShowMessage(L"Error", L"Cannot delete. This quiz has " + winrt::to_hstring(attemptsArray.Size()) + L" submissions.", InfoBarSeverity::Error);
                co_return;
            }

            ContentDialog dialog;
            dialog.XamlRoot(this->XamlRoot());
            dialog.Title(box_value(L"Delete quiz"));
            dialog.Content(box_value(L"Are you sure you want to delete this quiz? This action cannot be undone."));
            dialog.PrimaryButtonText(L"Delete");
            dialog.CloseButtonText(L"Cancel");
            dialog.DefaultButton(ContentDialogButton::Close);

            auto result = co_await dialog.ShowAsync();

            if (result == ContentDialogResult::Primary)
            {
                LoadingRingCenter().IsActive(true);

                hstring deleteEndpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(L"quizzes?id=eq." + quizId);
                co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(deleteEndpoint, L"", HttpMethod::Delete());

                ShowMessage(L"Success", L"Quiz deleted successfully", InfoBarSeverity::Success);
                LoadQuizzes();
            }
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[DeleteQuiz] Error: " + ex.message() + L"\n").c_str());
            ShowMessage(L"Error", L"Failed to delete quiz: " + ex.message(), InfoBarSeverity::Error);
            LoadingRingCenter().IsActive(false);
        }
    }

    void AllQuizzesManagementPage::ShowMessage(hstring const &title, hstring const &message, InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), title, message, severity);
    }
}

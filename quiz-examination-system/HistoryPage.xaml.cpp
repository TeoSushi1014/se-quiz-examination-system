#include "pch.h"
#include "HistoryPage.xaml.h"
#if __has_include("HistoryPage.g.cpp")
#include "HistoryPage.g.cpp"
#endif
#include "SupabaseClientManager.h"
#include "PageHelper.h"
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Windows::Data::Json;

namespace winrt::quiz_examination_system::implementation
{
    HistoryPage::HistoryPage()
    {
        InitializeComponent();
        m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClientAsync>();
    }

    void HistoryPage::Page_Loaded(IInspectable const &, RoutedEventArgs const &)
    {
        m_studentId = ::quiz_examination_system::SupabaseClientManager::GetInstance().GetUserId();
        LoadHistory();
    }

    fire_and_forget HistoryPage::LoadHistory()
    {
        auto lifetime = get_strong();

        LoadingRing().IsActive(true);
        EmptyState().Visibility(Visibility::Collapsed);
        HistoryListView().Visibility(Visibility::Collapsed);

        try
        {
            OutputDebugStringW(L"[HistoryPage] Loading history...\n");

            auto jsonStr = co_await m_supabaseClient->GetStudentHistoryAsync(m_studentId);

            std::wstring jsonStrW(jsonStr);
            OutputDebugStringW((L"[HistoryPage] Response: " + jsonStrW.substr(0, min((size_t)200, jsonStrW.size())) + L"\n").c_str());

            JsonArray historyArray;
            if (!JsonArray::TryParse(jsonStr, historyArray))
            {
                ShowMessage(L"Failed to parse history data", InfoBarSeverity::Error);
                LoadingRing().IsActive(false);
                co_return;
            }

            if (historyArray.Size() == 0)
            {
                EmptyState().Visibility(Visibility::Visible);
                LoadingRing().IsActive(false);
                co_return;
            }

            Windows::Foundation::Collections::IVector<IInspectable> items =
                single_threaded_observable_vector<IInspectable>();

            for (uint32_t i = 0; i < historyArray.Size(); i++)
            {
                auto obj = historyArray.GetAt(i).GetObject();

                auto attemptId = obj.GetNamedString(L"id", L"");
                auto quizTitle = obj.GetNamedString(L"quiz_title", L"Unknown Quiz");
                auto attemptNumber = static_cast<int32_t>(obj.GetNamedNumber(L"attempt_number", 1));
                auto score = static_cast<int32_t>(obj.GetNamedNumber(L"score", 0));
                auto totalPoints = static_cast<int32_t>(obj.GetNamedNumber(L"total_points", 0));
                auto correctCount = static_cast<int32_t>(obj.GetNamedNumber(L"correct_count", 0));
                auto incorrectCount = static_cast<int32_t>(obj.GetNamedNumber(L"incorrect_count", 0));
                auto timeSpentSeconds = static_cast<int32_t>(obj.GetNamedNumber(L"time_spent_seconds", 0));
                auto submittedAt = obj.GetNamedString(L"submitted_at", L"");

                int minutes = timeSpentSeconds / 60;
                int seconds = timeSpentSeconds % 60;
                wchar_t timeStr[32];
                swprintf_s(timeStr, L"%d:%02d", minutes, seconds);

                std::wstring formattedDate = std::wstring(submittedAt);
                if (formattedDate.length() > 16)
                {
                    formattedDate = formattedDate.substr(0, 16);
                    std::replace(formattedDate.begin(), formattedDate.end(), 'T', ' ');
                }

                double percentage = totalPoints > 0 ? (double)score / totalPoints * 100 : 0;
                Microsoft::UI::Xaml::Media::SolidColorBrush badgeColor{nullptr};

                if (percentage >= 80)
                {
                    badgeColor = Microsoft::UI::Xaml::Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 39, 174, 96)); // Green
                }
                else if (percentage >= 50)
                {
                    badgeColor = Microsoft::UI::Xaml::Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 243, 156, 18)); // Orange
                }
                else
                {
                    badgeColor = Microsoft::UI::Xaml::Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 231, 76, 60)); // Red
                }

                auto item = make<AttemptItem>();
                item.AttemptId(attemptId);
                item.QuizTitle(quizTitle);
                item.AttemptNumber(attemptNumber);
                item.Score(score);
                item.TotalPoints(totalPoints);
                item.CorrectCount(correctCount);
                item.IncorrectCount(incorrectCount);
                item.TimeSpent(hstring(timeStr));
                item.SubmittedAt(hstring(formattedDate));
                item.ScoreBadgeColor(badgeColor);

                items.Append(item);
            }

            HistoryListView().ItemsSource(items);
            HistoryListView().Visibility(Visibility::Visible);

            OutputDebugStringW((L"[HistoryPage] Loaded " + to_hstring(items.Size()) + L" attempts\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[HistoryPage] Error: " + ex.message() + L"\n").c_str());
            ShowMessage(L"Error loading history: " + ex.message(), InfoBarSeverity::Error);
        }

        LoadingRing().IsActive(false);
    }

    void HistoryPage::ShowMessage(hstring const &message, InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), message, severity);
    }
}

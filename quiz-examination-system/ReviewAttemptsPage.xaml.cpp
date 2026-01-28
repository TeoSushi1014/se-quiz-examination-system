#include "pch.h"
#include "ReviewAttemptsPage.xaml.h"
#if __has_include("ReviewAttemptsPage.g.cpp")
#include "ReviewAttemptsPage.g.cpp"
#endif
#include "PageHelper.h"
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>
#include <sstream>
#include <iomanip>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Data::Json;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;

namespace winrt::quiz_examination_system::implementation
{
    ReviewAttemptsPage::ReviewAttemptsPage()
    {
        InitializeComponent();
        m_attempts = single_threaded_observable_vector<quiz_examination_system::AttemptItem>();
        m_client = std::make_unique<::quiz_examination_system::SupabaseClientAsync>();
    }

    void ReviewAttemptsPage::Page_Loaded(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        LoadQuizList();
    }

    winrt::fire_and_forget ReviewAttemptsPage::LoadQuizList()
    {
        auto lifetime = get_strong();
        LoadingRing().IsActive(true);

        try
        {
            auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
            auto teacherId = manager.GetUserId();

            OutputDebugStringW(L"[ReviewAttemptsPage] Loading quiz list for teacher...\n");
            auto quizzesJson = co_await m_client->GetQuizzesJsonAsync(teacherId);

            OutputDebugStringW(L"[ReviewAttemptsPage] Received quiz response\n");

            if (quizzesJson.empty() || quizzesJson == L"[]" || quizzesJson == L"null")
            {
                OutputDebugStringW(L"[ReviewAttemptsPage] Empty quiz response\n");
                m_quizOptions.clear();
                QuizSelector().Items().Clear();
                LoadingRing().IsActive(false);
                co_return;
            }

            OutputDebugStringW(L"[ReviewAttemptsPage] Parsing quiz JSON...\n");
            auto quizzesArray = JsonArray::Parse(quizzesJson);
            m_quizOptions.clear();

            for (uint32_t i = 0; i < quizzesArray.Size(); ++i)
            {
                auto qObj = quizzesArray.GetObjectAt(i);
                QuizOption option;
                option.QuizId = qObj.GetNamedString(L"id", L"");
                option.Title = qObj.GetNamedString(L"title", L"");
                option.ResultVisibility = qObj.GetNamedString(L"result_visibility", L"Immediate");
                option.ResultReleased = qObj.GetNamedBoolean(L"result_released", false);
                m_quizOptions.push_back(option);
            }

            QuizSelector().Items().Clear();
            for (auto const &opt : m_quizOptions)
            {
                ComboBoxItem item;
                item.Content(box_value(opt.Title));
                item.Tag(box_value(opt.QuizId));
                QuizSelector().Items().Append(item);
            }
        }
        catch (hresult_error const &)
        {
            OutputDebugStringW(L"[ReviewAttemptsPage] Error in LoadQuizList\n");
            ShowMessage(L"Failed to load quizzes", InfoBarSeverity::Error);
        }

        LoadingRing().IsActive(false);
    }

    void ReviewAttemptsPage::QuizSelector_SelectionChanged(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const &)
    {
        auto combo = sender.as<ComboBox>();
        if (combo.SelectedIndex() >= 0)
        {
            auto selectedItem = combo.SelectedItem().as<ComboBoxItem>();
            auto quizId = unbox_value<hstring>(selectedItem.Tag());
            m_currentQuizId = quizId;

            LoadAttempts(quizId);
            ExportCsvBtn().IsEnabled(true);
            UpdateReleaseButton();
        }
        else
        {
            ExportCsvBtn().IsEnabled(false);
            ToggleReleaseBtn().Visibility(Visibility::Collapsed);
        }
    }

    winrt::fire_and_forget ReviewAttemptsPage::LoadAttempts(hstring const &quizId)
    {
        auto lifetime = get_strong();

        LoadingRing().IsActive(true);
        EmptyStateText().Visibility(Visibility::Collapsed);

        try
        {
            OutputDebugStringW(L"[ReviewAttemptsPage] Loading attempts for quiz\n");

            hstring attemptsJson;
            try
            {
                attemptsJson = co_await m_client->GetQuizAttemptsForReviewAsync(quizId);
                OutputDebugStringW(L"[ReviewAttemptsPage] Received attempts response\n");
            }
            catch (...)
            {
                OutputDebugStringW(L"[ReviewAttemptsPage] Failed to get attempts from server\n");
                throw;
            }

            m_attempts.Clear();

            if (attemptsJson.empty())
            {
                OutputDebugStringW(L"[ReviewAttemptsPage] Empty attempts response\n");
                EmptyStateText().Visibility(Visibility::Visible);
                LoadingRing().IsActive(false);
                co_return;
            }

            std::wstring jsonStr(attemptsJson);
            if (jsonStr == L"[]" || jsonStr == L"null" || jsonStr.find(L'{') == std::wstring::npos)
            {
                OutputDebugStringW(L"[ReviewAttemptsPage] No attempts found\n");
                EmptyStateText().Visibility(Visibility::Visible);
                LoadingRing().IsActive(false);
                co_return;
            }

            OutputDebugStringW(L"[ReviewAttemptsPage] Parsing attempts JSON...\n");
            JsonArray attemptsArray{nullptr};
            try
            {
                attemptsArray = JsonArray::Parse(attemptsJson);
                OutputDebugStringW(L"[ReviewAttemptsPage] JSON parsed successfully\n");
            }
            catch (hresult_error const &)
            {
                OutputDebugStringW(L"[ReviewAttemptsPage] Attempts JSON parse failed\n");
                throw;
            }

            for (uint32_t i = 0; i < attemptsArray.Size(); ++i)
            {
                auto aObj = attemptsArray.GetObjectAt(i);

                auto item = make<AttemptItem>();
                item.AttemptId(aObj.GetNamedString(L"attempt_id", L""));
                item.StudentId(aObj.GetNamedString(L"student_id", L""));
                item.StudentUsername(aObj.GetNamedString(L"student_username", L""));
                item.AttemptNumber(static_cast<int32_t>(aObj.GetNamedNumber(L"attempt_number", 0)));
                item.Score(static_cast<int32_t>(aObj.GetNamedNumber(L"score", 0)));
                item.TotalPoints(static_cast<int32_t>(aObj.GetNamedNumber(L"total_points", 0)));
                item.CorrectCount(static_cast<int32_t>(aObj.GetNamedNumber(L"correct_count", 0)));
                item.IncorrectCount(static_cast<int32_t>(aObj.GetNamedNumber(L"incorrect_count", 0)));
                item.TimeSpentSeconds(static_cast<int32_t>(aObj.GetNamedNumber(L"time_spent_seconds", 0)));
                item.Status(aObj.GetNamedString(L"status", L""));
                item.StartedAt(aObj.GetNamedString(L"started_at", L""));
                item.SubmittedAt(aObj.GetNamedString(L"submitted_at", L""));

                m_attempts.Append(item);
            }

            AttemptsListView().ItemsSource(m_attempts);

            if (m_attempts.Size() == 0)
            {
                EmptyStateText().Visibility(Visibility::Visible);
            }
        }
        catch (hresult_error const &)
        {
            OutputDebugStringW(L"[ReviewAttemptsPage] Error in LoadAttempts\n");
            ShowMessage(L"Failed to load attempts", InfoBarSeverity::Error);
        }

        LoadingRing().IsActive(false);
    }

    void ReviewAttemptsPage::Refresh_Click(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        if (!m_currentQuizId.empty())
        {
            LoadAttempts(m_currentQuizId);
        }
        else
        {
            LoadQuizList();
        }
    }

    winrt::fire_and_forget ReviewAttemptsPage::ExportCsv_Click(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        auto lifetime = get_strong();

        if (m_attempts.Size() == 0)
        {
            ShowMessage(L"No attempts to export", InfoBarSeverity::Warning);
            co_return;
        }

        try
        {
            FileSavePicker savePicker;
            savePicker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
            savePicker.FileTypeChoices().Insert(L"CSV File", single_threaded_vector<hstring>({L".csv"}));
            savePicker.SuggestedFileName(L"quiz_attempts");

            auto hwnd = GetActiveWindow();
            auto initializeWithWindow{savePicker.as<::IInitializeWithWindow>()};
            initializeWithWindow->Initialize(hwnd);

            auto file = co_await savePicker.PickSaveFileAsync();
            if (!file)
            {
                co_return;
            }

            std::wstringstream csv;
            csv << L"Student,Attempt,Score,Total Points,Correct,Incorrect,Time Spent (s),Submitted At\n";

            for (uint32_t i = 0; i < m_attempts.Size(); ++i)
            {
                auto item = m_attempts.GetAt(i);
                csv << item.StudentUsername().c_str() << L","
                    << item.AttemptNumber() << L","
                    << item.Score() << L","
                    << item.TotalPoints() << L","
                    << item.CorrectCount() << L","
                    << item.IncorrectCount() << L","
                    << item.TimeSpentSeconds() << L","
                    << item.SubmittedAt().c_str() << L"\n";
            }

            co_await FileIO::WriteTextAsync(file, csv.str());
            ShowMessage(L"CSV exported successfully", InfoBarSeverity::Success);
        }
        catch (hresult_error const &ex)
        {
            ShowMessage(ex.message(), InfoBarSeverity::Error);
        }
    }

    winrt::fire_and_forget ReviewAttemptsPage::DeleteAttempt_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        auto lifetime = get_strong();
        auto button = sender.as<Button>();
        auto attemptId = unbox_value<hstring>(button.Tag());

        ContentDialog confirmDialog;
        confirmDialog.XamlRoot(this->XamlRoot());
        confirmDialog.Title(box_value(L"Confirm deletion"));
        confirmDialog.Content(box_value(L"Are you sure you want to delete this attempt? This action cannot be undone."));
        confirmDialog.PrimaryButtonText(L"Delete");
        confirmDialog.CloseButtonText(L"Cancel");
        confirmDialog.DefaultButton(ContentDialogButton::Close);

        auto result = co_await confirmDialog.ShowAsync();
        if (result != ContentDialogResult::Primary)
        {
            co_return;
        }

        try
        {
            auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
            auto teacherId = manager.GetUserId();

            auto resultJson = co_await m_client->DeleteQuizAttemptAsync(attemptId, teacherId);
            auto resultObj = JsonObject::Parse(resultJson);
            bool success = resultObj.GetNamedBoolean(L"success", false);

            if (success)
            {
                ShowMessage(L"Attempt deleted successfully", InfoBarSeverity::Success);
                LoadAttempts(m_currentQuizId);
            }
            else
            {
                auto message = resultObj.GetNamedString(L"message", L"Failed to delete attempt");
                ShowMessage(message, InfoBarSeverity::Error);
            }
        }
        catch (hresult_error const &ex)
        {
            ShowMessage(ex.message(), InfoBarSeverity::Error);
        }
    }

    winrt::fire_and_forget ReviewAttemptsPage::ToggleRelease_Click(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        auto lifetime = get_strong();

        try
        {
            auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
            auto teacherId = manager.GetUserId();

            // Find current quiz option
            auto it = std::find_if(m_quizOptions.begin(), m_quizOptions.end(),
                                   [this](const QuizOption &opt)
                                   { return opt.QuizId == m_currentQuizId; });

            if (it == m_quizOptions.end())
            {
                co_return;
            }

            bool newReleaseState = !it->ResultReleased;

            auto resultJson = co_await m_client->ToggleResultReleaseAsync(m_currentQuizId, teacherId, newReleaseState);
            auto resultObj = JsonObject::Parse(resultJson);
            bool success = resultObj.GetNamedBoolean(L"success", false);

            if (success)
            {
                it->ResultReleased = newReleaseState;
                auto message = resultObj.GetNamedString(L"message", L"Status updated");
                ShowMessage(message, InfoBarSeverity::Success);
                UpdateReleaseButton();
            }
            else
            {
                auto message = resultObj.GetNamedString(L"message", L"Failed to update status");
                ShowMessage(message, InfoBarSeverity::Error);
            }
        }
        catch (hresult_error const &ex)
        {
            ShowMessage(ex.message(), InfoBarSeverity::Error);
        }
    }

    void ReviewAttemptsPage::UpdateReleaseButton()
    {
        auto it = std::find_if(m_quizOptions.begin(), m_quizOptions.end(),
                               [this](const QuizOption &opt)
                               { return opt.QuizId == m_currentQuizId; });

        if (it != m_quizOptions.end() && it->ResultVisibility == L"Manual release")
        {
            ToggleReleaseBtn().Visibility(Visibility::Visible);
            ToggleReleaseBtn().IsEnabled(true);
            ToggleReleaseBtn().Content(box_value(it->ResultReleased ? L"Hide results" : L"Release results"));
        }
        else
        {
            ToggleReleaseBtn().Visibility(Visibility::Collapsed);
        }
    }

    void ReviewAttemptsPage::ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), message, severity);
    }
}

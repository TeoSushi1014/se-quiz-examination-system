#include "pch.h"
#include "StudentDashboardPage.xaml.h"
#include "ExamPage.xaml.h"
#include "QuizItemStudent.h"
#if __has_include("StudentDashboardPage.g.cpp")
#include "StudentDashboardPage.g.cpp"
#endif
#include "PageHelper.h"
#include "SupabaseClientManager.h"
#include "SupabaseClientAsync.h"
#include <winrt/Windows.Data.Json.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Interop;
using namespace Windows::Data::Json;

namespace winrt::quiz_examination_system::implementation
{
    StudentDashboardPage::StudentDashboardPage()
    {
        InitializeComponent();
        m_quizzes = single_threaded_observable_vector<quiz_examination_system::QuizItemStudent>();
        m_client = std::make_unique<::quiz_examination_system::SupabaseClientAsync>();

        auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
        m_currentUserId = manager.GetUserId();
    }

    Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuizItemStudent> StudentDashboardPage::Quizzes()
    {
        return m_quizzes;
    }

    void StudentDashboardPage::Page_Loaded(IInspectable const &, RoutedEventArgs const &)
    {
        LoadQuizzes();
    }

    winrt::fire_and_forget StudentDashboardPage::LoadQuizzes()
    {
        auto lifetime = get_strong();

        LoadingRing().IsActive(true);
        m_quizzes.Clear();
        StartExamButton().IsEnabled(false);

        try
        {
            hstring endpoint = ::quiz_examination_system::SupabaseConfig::GetRpcEndpoint(L"get_student_quizzes");

            JsonObject params;
            params.Insert(L"input_student_id", JsonValue::CreateStringValue(m_currentUserId));

            auto responseText = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(
                endpoint, params.Stringify(), Windows::Web::Http::HttpMethod::Post());

            if (responseText.empty() || responseText == L"null" || responseText == L"[]")
            {
                ShowMessage(L"No quizzes assigned to you yet", InfoBarSeverity::Warning);
                LoadingRing().IsActive(false);
                co_return;
            }

            JsonArray quizzesArray = JsonArray::Parse(responseText);

            for (uint32_t i = 0; i < quizzesArray.Size(); ++i)
            {
                auto qObj = quizzesArray.GetObjectAt(i);

                auto item = make<QuizItemStudent>();
                item.QuizId(qObj.GetNamedString(L"quiz_id", L""));
                item.Title(qObj.GetNamedString(L"quiz_title", L""));
                item.TimeLimit(static_cast<int>(qObj.GetNamedNumber(L"time_limit_minutes", 0)));
                item.TotalPoints(static_cast<int>(qObj.GetNamedNumber(L"total_points", 0)));
                item.AttemptsUsed(static_cast<int>(qObj.GetNamedNumber(L"attempts_used", 0)));

                hstring maxAttempts = qObj.GetNamedString(L"max_attempts", L"Unlimited");
                item.MaxAttempts(maxAttempts);

                int attemptsUsed = item.AttemptsUsed();
                bool hasAttemptsLeft = true;

                if (maxAttempts != L"Unlimited")
                {
                    try
                    {
                        int maxCount = std::stoi(maxAttempts.c_str());
                        hasAttemptsLeft = attemptsUsed < maxCount;
                    }
                    catch (...)
                    {
                        hasAttemptsLeft = true;
                    }
                }

                item.Status(hasAttemptsLeft ? L"Available" : L"No attempts left");

                m_quizzes.Append(item);
            }

            // Only show message if no quizzes available
            if (m_quizzes.Size() == 0)
            {
                ShowMessage(L"No quizzes available", InfoBarSeverity::Warning);
            }
        }
        catch (hresult_error const &ex)
        {
            ShowMessage(L"Failed to load quizzes: " + ex.message(), InfoBarSeverity::Error);
        }
        catch (...)
        {
            ShowMessage(L"An error occurred while loading quizzes", InfoBarSeverity::Error);
        }

        LoadingRing().IsActive(false);
    }

    void StudentDashboardPage::QuizzesGridView_SelectionChanged(IInspectable const &, SelectionChangedEventArgs const &)
    {
        int index = QuizzesGridView().SelectedIndex();
        if (index >= 0 && index < static_cast<int>(m_quizzes.Size()))
        {
            m_selectedQuiz = m_quizzes.GetAt(index);

            // Check if user still has attempts left
            hstring maxAttempts = m_selectedQuiz.MaxAttempts();
            bool hasAttemptsLeft = true;

            if (maxAttempts != L"Unlimited")
            {
                try
                {
                    int maxCount = std::stoi(maxAttempts.c_str());
                    hasAttemptsLeft = m_selectedQuiz.AttemptsUsed() < maxCount;
                }
                catch (...)
                {
                    hasAttemptsLeft = true;
                }
            }

            StartExamButton().IsEnabled(hasAttemptsLeft);
        }
        else
        {
            StartExamButton().IsEnabled(false);
        }
    }

    void StudentDashboardPage::StartExam_Click(IInspectable const &, RoutedEventArgs const &)
    {
        if (!m_selectedQuiz || m_selectedQuiz.QuizId().empty())
        {
            ShowMessage(L"Please select a quiz", InfoBarSeverity::Warning);
            return;
        }

        hstring maxAttempts = m_selectedQuiz.MaxAttempts();
        if (maxAttempts != L"Unlimited")
        {
            try
            {
                int maxCount = std::stoi(maxAttempts.c_str());
                if (m_selectedQuiz.AttemptsUsed() >= maxCount)
                {
                    ShowMessage(L"You have reached the maximum attempts for this quiz", InfoBarSeverity::Error);
                    return;
                }
            }
            catch (...)
            {
            }
        }

        auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
        hstring studentId = manager.GetUserId();

        hstring navParam = m_selectedQuiz.QuizId() + L"|" + studentId;
        Frame().Navigate(xaml_typename<quiz_examination_system::ExamPage>(), box_value(navParam));
    }

    void StudentDashboardPage::ShowMessage(hstring const &message, InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), message, severity);
    }
}

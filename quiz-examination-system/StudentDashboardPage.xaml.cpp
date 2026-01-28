#include "pch.h"
#include "StudentDashboardPage.xaml.h"
#include "ExamPage.xaml.h"
#include "QuizItemStudent.h"
#if __has_include("StudentDashboardPage.g.cpp")
#include "StudentDashboardPage.g.cpp"
#endif
#include "PageHelper.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Interop;

namespace winrt::quiz_examination_system::implementation
{
    StudentDashboardPage::StudentDashboardPage()
    {
        InitializeComponent();
        m_quizzes = single_threaded_observable_vector<quiz_examination_system::QuizItemStudent>();
        m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        m_currentUserId = L"";
    }

    Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuizItemStudent> StudentDashboardPage::Quizzes()
    {
        return m_quizzes;
    }

    void StudentDashboardPage::Page_Loaded(IInspectable const &, RoutedEventArgs const &)
    {
        LoadQuizzes();
    }

    void StudentDashboardPage::LoadQuizzes()
    {
        if (!m_supabaseClient)
        {
            ShowMessage(L"Error: Cannot connect to database", InfoBarSeverity::Error);
            return;
        }

        LoadingRing().IsActive(true);
        ShowMessage(L"Loading quiz list...", InfoBarSeverity::Informational);

        auto dispatcher = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();

        m_supabaseClient->OnStudentQuizzesLoaded = [this, dispatcher](bool success, std::vector<::quiz_examination_system::SupabaseClient::QuizData> quizzes)
        {
            dispatcher.TryEnqueue([this, success, quizzes]()
                                  {
                LoadingRing().IsActive(false);
                ActionMessage().IsOpen(false);

                if (!success)
                {
                    ShowMessage(L"Failed to load quiz list", InfoBarSeverity::Error);
                    return;
                }

                if (quizzes.empty())
                {
                    ShowMessage(L"No quizzes assigned to you yet", InfoBarSeverity::Warning);
                    return;
                }

                m_quizzes.Clear();

                for (const auto &quiz : quizzes)
                {
                    auto item = make<QuizItemStudent>();
                    item.QuizId(quiz.quiz_id);
                    item.Title(quiz.quiz_title);
                    item.TimeLimit(quiz.time_limit_minutes);
                    item.TotalPoints(quiz.total_points);
                    item.AttemptsUsed(quiz.attempts_used);
                    item.MaxAttempts(quiz.max_attempts);
                    item.Status(L"Available");

                    m_quizzes.Append(item);
                }

                ShowMessage(hstring(L"Loaded ") + to_hstring(quizzes.size()) + L" quizzes", InfoBarSeverity::Success); });
        };

        m_supabaseClient->GetStudentQuizzes(m_currentUserId);
    }

    void StudentDashboardPage::QuizzesGridView_SelectionChanged(IInspectable const &, SelectionChangedEventArgs const &)
    {
        int index = QuizzesGridView().SelectedIndex();
        if (index >= 0 && index < static_cast<int>(m_quizzes.Size()))
        {
            m_selectedQuiz = m_quizzes.GetAt(index);
            StartExamButton().IsEnabled(true);
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

        if (m_selectedQuiz.MaxAttempts() != L"unlimited")
        {
            int maxAttempts = std::stoi(m_selectedQuiz.MaxAttempts().c_str());
            if (m_selectedQuiz.AttemptsUsed() >= maxAttempts)
            {
                ShowMessage(L"You have reached the maximum attempts for this quiz", InfoBarSeverity::Error);
                return;
            }
        }

        Frame().Navigate(xaml_typename<quiz_examination_system::ExamPage>());

        auto page = Frame().Content().try_as<quiz_examination_system::ExamPage>();
        if (page)
        {
            page.as<quiz_examination_system::implementation::ExamPage>()->SetQuizData(m_selectedQuiz.QuizId(), m_currentUserId);
        }
    }

    void StudentDashboardPage::ShowMessage(hstring const &message, InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), message, severity);
    }
}

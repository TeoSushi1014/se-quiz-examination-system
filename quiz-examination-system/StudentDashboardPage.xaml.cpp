#include "pch.h"
#include "StudentDashboardPage.xaml.h"
#if __has_include("StudentDashboardPage.g.cpp")
#include "StudentDashboardPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::quiz_examination_system::implementation
{
    StudentDashboardPage::StudentDashboardPage()
    {
        InitializeComponent();
        m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
    }

    void StudentDashboardPage::TakeQuiz_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);

        if (!m_supabaseClient)
        {
            ActionMessage().Message(L"Database client not initialized");
            ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }

        m_supabaseClient->OnQuizQuestionsLoaded = [this](bool success, std::vector<::quiz_examination_system::SupabaseClient::QuestionData> questions)
        {
            if (success && !questions.empty())
            {
                hstring message = hstring(L"Loaded ") + to_hstring(questions.size()) + L" questions. Demo: Question 1 is '" + questions[0].question_text + L"'";
                ActionMessage().Message(message);
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Success);
            }
            else
            {
                ActionMessage().Message(L"Failed to load quiz questions");
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            }
            ActionMessage().IsOpen(true);
        };

        m_supabaseClient->GetQuizQuestions(L"Q01");
    }

    void StudentDashboardPage::DemoSubmitAttempt_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);

        if (!m_supabaseClient)
        {
            ActionMessage().Message(L"Database client not initialized");
            ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }

        // Demo: Create sample answers for quiz Q01
        // Assuming student 004 takes quiz Q01 with answers for questions Q001, Q002, Q003
        JsonArray answersArray;

        JsonObject answer1;
        answer1.Insert(L"question_id", JsonValue::CreateStringValue(L"Q001"));
        answer1.Insert(L"selected_option", JsonValue::CreateStringValue(L"A"));
        answersArray.Append(answer1);

        JsonObject answer2;
        answer2.Insert(L"question_id", JsonValue::CreateStringValue(L"Q002"));
        answer2.Insert(L"selected_option", JsonValue::CreateStringValue(L"B"));
        answersArray.Append(answer2);

        JsonObject answer3;
        answer3.Insert(L"question_id", JsonValue::CreateStringValue(L"Q003"));
        answer3.Insert(L"selected_option", JsonValue::CreateStringValue(L"C"));
        answersArray.Append(answer3);

        m_supabaseClient->OnAttemptSubmitted = [this](::quiz_examination_system::SupabaseClient::AttemptResult result)
        {
            if (result.success)
            {
                hstring message = hstring(L"Attempt submitted!\nID: ") + result.attempt_id +
                                  L"\nScore: " + to_hstring(result.score) + L"/" + to_hstring(result.total_points) +
                                  L"\nCorrect: " + to_hstring(result.correct_count);
                ActionMessage().Message(message);
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Success);
            }
            else
            {
                ActionMessage().Message(hstring(L"Failed to submit: ") + result.message);
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            }
            ActionMessage().IsOpen(true);
        };

        m_supabaseClient->SubmitQuizAttempt(L"004", L"Q01", answersArray.Stringify(), 120);
    }

    void StudentDashboardPage::DemoLoadQuizzes_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);

        if (!m_supabaseClient)
        {
            ActionMessage().Message(L"Database client not initialized");
            ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }

        m_supabaseClient->OnStudentQuizzesLoaded = [this](bool success, std::vector<::quiz_examination_system::SupabaseClient::QuizData> quizzes)
        {
            if (success && !quizzes.empty())
            {
                hstring message = hstring(L"Loaded ") + to_hstring(quizzes.size()) + L" available quizzes\n";
                for (size_t i = 0; i < quizzes.size() && i < 3; ++i)
                {
                    message = message + L"\n- " + quizzes[i].quiz_title +
                              L" (Attempts: " + to_hstring(quizzes[i].attempts_used) + L"/" + quizzes[i].max_attempts + L")";
                }
                ActionMessage().Message(message);
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Success);
            }
            else
            {
                ActionMessage().Message(L"Failed to load quizzes or no quizzes available");
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Warning);
            }
            ActionMessage().IsOpen(true);
        };

        m_supabaseClient->GetStudentQuizzes(L"004");
    }

    void StudentDashboardPage::DemoViewAttemptResults_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);

        if (!m_supabaseClient)
        {
            ActionMessage().Message(L"Database client not initialized");
            ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }

        m_supabaseClient->OnAttemptResultsLoaded = [this](bool success, std::vector<::quiz_examination_system::SupabaseClient::AttemptData> attempts)
        {
            if (success && !attempts.empty())
            {
                hstring message = hstring(L"Found ") + to_hstring(attempts.size()) + L" attempts:\n";
                for (size_t i = 0; i < attempts.size() && i < 3; ++i)
                {
                    message = message + L"\nAttempt #" + to_hstring(attempts[i].attempt_number) +
                              L": " + to_hstring(attempts[i].score) + L"/" + to_hstring(attempts[i].total_points);
                }
                ActionMessage().Message(message);
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Success);
            }
            else
            {
                ActionMessage().Message(L"No attempts found");
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Warning);
            }
            ActionMessage().IsOpen(true);
        };

        m_supabaseClient->GetAttemptResults(L"004", L"Q01");
    }

    void StudentDashboardPage::DemoViewAttemptDetails_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);

        if (!m_supabaseClient)
        {
            ActionMessage().Message(L"Database client not initialized");
            ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }

        m_supabaseClient->OnAttemptDetailsLoaded = [this](bool success, std::vector<::quiz_examination_system::SupabaseClient::AnswerDetail> answers)
        {
            if (success && !answers.empty())
            {
                hstring message = hstring(L"Attempt details (") + to_hstring(answers.size()) + L" questions):\n";
                int correctCount = 0;
                for (size_t i = 0; i < answers.size() && i < 2; ++i)
                {
                    if (answers[i].is_correct)
                        correctCount++;
                    std::wstring qText = answers[i].question_text.c_str();
                    if (qText.length() > 30)
                        qText = qText.substr(0, 30);
                    message = message + L"\nQ: " + hstring(qText) +
                              (answers[i].is_correct ? L" [✓]" : L" [✗]");
                }
                message = message + hstring(L"\n\nTotal Correct: ") + to_hstring(correctCount) + L"/" + to_hstring(answers.size());
                ActionMessage().Message(message);
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Success);
            }
            else
            {
                ActionMessage().Message(L"Failed to load attempt details");
                ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            }
            ActionMessage().IsOpen(true);
        };

        m_supabaseClient->GetAttemptDetailsWithAnswers(L"ATM000");
    }
}

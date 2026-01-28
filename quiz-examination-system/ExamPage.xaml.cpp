#include "pch.h"
#include "ExamPage.xaml.h"
#if __has_include("ExamPage.g.cpp")
#include "ExamPage.g.cpp"
#endif
#include "PageHelper.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Dispatching;
using namespace Windows::UI::Xaml::Interop;

namespace winrt::quiz_examination_system::implementation
{
    ExamPage::ExamPage()
    {
        InitializeComponent();
        m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
    }

    ExamPage::ExamPage(hstring const &quizId, hstring const &studentId) : ExamPage()
    {
        m_quizId = quizId;
        m_studentId = studentId;
    }

    void ExamPage::Page_Loaded(IInspectable const &, RoutedEventArgs const &)
    {
        if (m_quizId.empty() || m_studentId.empty())
        {
            ShowMessage(L"Missing quiz or student information", InfoBarSeverity::Error);
            return;
        }

        LoadQuizData();
        LoadQuestions();
    }

    void ExamPage::LoadQuizData()
    {
        QuizTitleText().Text(L"Loading quiz information...");
        QuizInfoText().Text(L"");
    }

    void ExamPage::LoadQuestions()
    {
        m_supabaseClient->OnQuizQuestionsLoaded = [this](bool success, std::vector<::quiz_examination_system::SupabaseClient::QuestionData> questions)
        {
            if (!success || questions.empty())
            {
                ShowMessage(L"Failed to load questions", InfoBarSeverity::Error);
                return;
            }

            m_questions = questions;

            std::sort(m_questions.begin(), m_questions.end(),
                      [](const auto &a, const auto &b)
                      { return a.order_num < b.order_num; });

            Windows::Foundation::Collections::IVector<IInspectable> items =
                winrt::single_threaded_observable_vector<IInspectable>();

            for (size_t i = 0; i < m_questions.size(); ++i)
            {
                hstring itemText = hstring(L"Q") + to_hstring(i + 1);
                items.Append(box_value(itemText));
            }
            QuestionListView().ItemsSource(items);

            if (!m_questions.empty())
            {
                int totalPoints = 0;
                for (const auto &q : m_questions)
                {
                    totalPoints += q.points;
                }

                QuizTitleText().Text(L"Quiz Examination");
                QuizInfoText().Text(hstring(L"Questions: ") + to_hstring(m_questions.size()) +
                                    L" | Total Points: " + to_hstring(totalPoints));

                m_timeLimitSeconds = 30 * 60;
                m_timeRemainingSeconds = m_timeLimitSeconds;

                DisplayQuestion(0);
                StartTimer();
            }
        };

        m_supabaseClient->GetQuizQuestions(m_quizId);
    }

    void ExamPage::DisplayQuestion(int index)
    {
        if (index < 0 || index >= static_cast<int>(m_questions.size()))
            return;

        SaveCurrentAnswer();

        m_currentQuestionIndex = index;
        auto &q = m_questions[index];

        QuestionNumberText().Text(hstring(L"Question ") + to_hstring(index + 1) + L" / " + to_hstring(m_questions.size()));
        QuestionText().Text(q.question_text);

        OptionA().Content(box_value(hstring(L"A. ") + q.option_a));
        OptionB().Content(box_value(hstring(L"B. ") + q.option_b));
        OptionC().Content(box_value(hstring(L"C. ") + q.option_c));
        OptionD().Content(box_value(hstring(L"D. ") + q.option_d));

        OptionA().IsChecked(false);
        OptionB().IsChecked(false);
        OptionC().IsChecked(false);
        OptionD().IsChecked(false);

        auto it = m_answers.find(q.question_id);
        if (it != m_answers.end())
        {
            if (it->second == L"A")
                OptionA().IsChecked(true);
            else if (it->second == L"B")
                OptionB().IsChecked(true);
            else if (it->second == L"C")
                OptionC().IsChecked(true);
            else if (it->second == L"D")
                OptionD().IsChecked(true);
        }

        PrevButton().IsEnabled(index > 0);
        NextButton().IsEnabled(index < static_cast<int>(m_questions.size()) - 1);

        QuestionListView().SelectedIndex(index);
    }

    void ExamPage::SaveCurrentAnswer()
    {
        if (m_currentQuestionIndex < 0 || m_currentQuestionIndex >= static_cast<int>(m_questions.size()))
            return;

        auto &q = m_questions[m_currentQuestionIndex];

        if (OptionA().IsChecked().GetBoolean())
            m_answers[q.question_id] = L"A";
        else if (OptionB().IsChecked().GetBoolean())
            m_answers[q.question_id] = L"B";
        else if (OptionC().IsChecked().GetBoolean())
            m_answers[q.question_id] = L"C";
        else if (OptionD().IsChecked().GetBoolean())
            m_answers[q.question_id] = L"D";
        else
            m_answers.erase(q.question_id);
    }

    void ExamPage::StartTimer()
    {
        auto queue = DispatcherQueue::GetForCurrentThread();
        m_timer = queue.CreateTimer();

        m_timer.Interval(std::chrono::seconds(1));
        m_timer.IsRepeating(true);

        m_timer.Tick([this](auto &&, auto &&)
                     {
            m_timeRemainingSeconds--;
            UpdateTimerDisplay();

            if (m_timeRemainingSeconds <= 0)
            {
                m_timer.Stop();
                ShowMessage(L"Time's up! Auto-submitting...", InfoBarSeverity::Warning);
                SubmitAttempt();
            } });

        m_timer.Start();
        UpdateTimerDisplay();
    }

    void ExamPage::UpdateTimerDisplay()
    {
        int minutes = m_timeRemainingSeconds / 60;
        int seconds = m_timeRemainingSeconds % 60;

        wchar_t buffer[10];
        swprintf_s(buffer, L"%02d:%02d", minutes, seconds);
        TimerText().Text(buffer);

        if (m_timeRemainingSeconds <= 60)
        {
            TimerText().Foreground(Microsoft::UI::Xaml::Media::SolidColorBrush(
                Microsoft::UI::Colors::Red()));
        }
    }

    void ExamPage::QuestionListView_SelectionChanged(IInspectable const &, SelectionChangedEventArgs const &)
    {
        int index = QuestionListView().SelectedIndex();
        if (index >= 0 && index < static_cast<int>(m_questions.size()))
        {
            DisplayQuestion(index);
        }
    }

    void ExamPage::PrevButton_Click(IInspectable const &, RoutedEventArgs const &)
    {
        if (m_currentQuestionIndex > 0)
        {
            DisplayQuestion(m_currentQuestionIndex - 1);
        }
    }

    void ExamPage::NextButton_Click(IInspectable const &, RoutedEventArgs const &)
    {
        if (m_currentQuestionIndex < static_cast<int>(m_questions.size()) - 1)
        {
            DisplayQuestion(m_currentQuestionIndex + 1);
        }
    }

    fire_and_forget ExamPage::SubmitButton_Click(IInspectable const &, RoutedEventArgs const &)
    {
        auto lifetime = get_strong();

        ContentDialog confirmDialog;
        confirmDialog.XamlRoot(this->XamlRoot());
        confirmDialog.Title(box_value(L"Confirm Submission"));
        confirmDialog.Content(box_value(hstring(L"Are you sure you want to submit? You have answered ") +
                                        to_hstring(m_answers.size()) + L"/" + to_hstring(m_questions.size()) + L" questions."));
        confirmDialog.PrimaryButtonText(L"Submit");
        confirmDialog.CloseButtonText(L"Cancel");
        confirmDialog.DefaultButton(ContentDialogButton::Close);

        auto result = co_await confirmDialog.ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            SubmitAttempt();
        }
    }

    void ExamPage::SubmitAttempt()
    {
        if (m_timer)
        {
            m_timer.Stop();
        }

        SaveCurrentAnswer();

        SubmitButton().IsEnabled(false);
        ShowMessage(L"Submitting...", InfoBarSeverity::Informational);

        Windows::Data::Json::JsonArray answersArray;
        for (const auto &q : m_questions)
        {
            Windows::Data::Json::JsonObject answerObj;
            answerObj.Insert(L"question_id", Windows::Data::Json::JsonValue::CreateStringValue(q.question_id));

            auto it = m_answers.find(q.question_id);
            if (it != m_answers.end())
            {
                answerObj.Insert(L"selected_option", Windows::Data::Json::JsonValue::CreateStringValue(it->second));
            }
            else
            {
                answerObj.Insert(L"selected_option", Windows::Data::Json::JsonValue::CreateStringValue(L""));
            }

            answersArray.Append(answerObj);
        }

        int timeSpent = m_timeLimitSeconds - m_timeRemainingSeconds;

        m_supabaseClient->OnAttemptSubmitted = [this](::quiz_examination_system::SupabaseClient::AttemptResult result)
        {
            if (result.success)
            {
                hstring message = hstring(L"Submission successful!\n\n") +
                                  L"Score: " + to_hstring(result.score) + L"/" + to_hstring(result.total_points) + L"\n" +
                                  L"Correct: " + to_hstring(result.correct_count) + L"\n" +
                                  L"Incorrect: " + to_hstring(result.incorrect_count);

                ShowMessage(message, InfoBarSeverity::Success);

                PrevButton().IsEnabled(false);
                NextButton().IsEnabled(false);
                OptionA().IsEnabled(false);
                OptionB().IsEnabled(false);
                OptionC().IsEnabled(false);
                OptionD().IsEnabled(false);
            }
            else
            {
                ShowMessage(hstring(L"Submission error: ") + result.message, InfoBarSeverity::Error);
                SubmitButton().IsEnabled(true);
            }
        };

        m_supabaseClient->SubmitQuizAttempt(m_studentId, m_quizId, answersArray.Stringify(), timeSpent);
    }

    void ExamPage::ShowMessage(hstring const &message, InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(MessageBar(), message, severity);
    }
}

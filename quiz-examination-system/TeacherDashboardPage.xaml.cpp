#include "pch.h"
#include "TeacherDashboardPage.xaml.h"
#if __has_include("TeacherDashboardPage.g.cpp")
#include "TeacherDashboardPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::quiz_examination_system::implementation
{
    TeacherDashboardPage::TeacherDashboardPage()
    {
        InitializeComponent();
        TitleText().Text(L"Question Bank");
        DescriptionText().Text(L"Manage your question bank");
    }

    void TeacherDashboardPage::TeacherNav_SelectionChanged(NavigationView const &, NavigationViewSelectionChangedEventArgs const &e)
    {
        auto tag = unbox_value<hstring>(e.SelectedItem().as<NavigationViewItem>().Tag());
        UpdateContent(tag);
    }

    void TeacherDashboardPage::UpdateContent(hstring const &section)
    {
        ActionMessage().IsOpen(false);
        ContentArea().Children().Clear();

        if (section == L"QuestionBank")
        {
            TitleText().Text(L"Question Bank");
            DescriptionText().Text(L"Create and manage your questions");
            QuestionBank_Click(nullptr, nullptr);
        }
        else if (section == L"ManageQuizzes")
        {
            TitleText().Text(L"Manage Quizzes");
            DescriptionText().Text(L"Create quizzes and assign questions");
            ManageQuizzes_Click(nullptr, nullptr);
        }
        else if (section == L"ReviewAttempts")
        {
            TitleText().Text(L"Review Attempts");
            DescriptionText().Text(L"Review student quiz attempts and grades");
            ReviewAttempts_Click(nullptr, nullptr);
        }
    }

    void TeacherDashboardPage::QuestionBank_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        ActionMessage().Message(L"Question bank management interface");
        ActionMessage().Severity(InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }

    void TeacherDashboardPage::ManageQuizzes_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        ActionMessage().Message(L"Quiz management interface");
        ActionMessage().Severity(InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }

    void TeacherDashboardPage::ReviewAttempts_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        ActionMessage().Message(L"Review student attempts");
        ActionMessage().Severity(InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }

    void TeacherDashboardPage::ShowMessage(hstring const &title, hstring const &message, InfoBarSeverity severity)
    {
        ActionMessage().Title(title);
        ActionMessage().Message(message);
        ActionMessage().Severity(severity);
        ActionMessage().IsOpen(true);
    }

    // =====================================================
    // UC02: Question Bank Management - Demo Functions
    // =====================================================

    void TeacherDashboardPage::DemoCreateQuestion_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Creating Question", L"Calling RPC: create_question_validated...", InfoBarSeverity::Informational);

        // Demo data - create a sample question
        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        // Set callback
        m_supabaseClient->OnQuestionValidatedCreated = [this](bool success, hstring message)
        {
            if (success)
            {
                ShowMessage(L"Success", message, InfoBarSeverity::Success);
            }
            else
            {
                ShowMessage(L"Validation Failed", message, InfoBarSeverity::Error);
            }
        };

        // Call RPC with demo data
        m_supabaseClient->CreateQuestionValidated(
            L"DQ1",                 // id
            L"002",                 // teacher_id
            L"What is 2+2?",        // question_text
            L"3", L"4", L"5", L"6", // options
            L"B",                   // correct (B = 4)
            L"easy",                // difficulty
            L"Math"                 // topic
        );
    }

    void TeacherDashboardPage::DemoDeleteQuestion_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Deleting Question", L"Calling RPC: delete_question_safe for DQ1...", InfoBarSeverity::Informational);

        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        // Set callback
        m_supabaseClient->OnQuestionDeleteResult = [this](hstring status, hstring message, int quizCount)
        {
            if (status == L"success")
            {
                ShowMessage(L"Deleted", message, InfoBarSeverity::Success);
            }
            else if (status == L"blocked")
            {
                hstring msg = message + L" (Used in " + winrt::to_hstring(quizCount) + L" quiz(es))";
                ShowMessage(L"Cannot Delete", msg, InfoBarSeverity::Warning);
            }
            else
            {
                ShowMessage(L"Error", message, InfoBarSeverity::Error);
            }
        };

        // Try to delete the demo question
        m_supabaseClient->DeleteQuestionSafe(L"DQ1");
    }

    void TeacherDashboardPage::DemoDeleteQuestionInUse_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Test: Delete In-Use Question", L"This will demonstrate blocking when question is in a quiz...", InfoBarSeverity::Informational);

        // First create a quiz that uses a question, then try to delete it
        // This is for demonstration of NFR-07 (Integrity Constraints)
        ShowMessage(L"Demo", L"See RPC_INTEGRATION_GUIDE.md for full implementation", InfoBarSeverity::Informational);
    }

    void TeacherDashboardPage::DemoDeleteQuiz_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Deleting Quiz", L"Calling RPC: delete_quiz_teacher...", InfoBarSeverity::Informational);

        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        // Set callback
        m_supabaseClient->OnQuizDeleteResult = [this](bool success, hstring message, int attemptCount)
        {
            if (success)
            {
                ShowMessage(L"Deleted", message, InfoBarSeverity::Success);
            }
            else
            {
                if (attemptCount > 0)
                {
                    hstring msg = message + L" (" + winrt::to_hstring(attemptCount) + L" student(s) took this quiz)";
                    ShowMessage(L"Cannot Delete", msg, InfoBarSeverity::Warning);
                }
                else
                {
                    ShowMessage(L"Error", message, InfoBarSeverity::Error);
                }
            }
        };

        // Try to delete a quiz (replace with actual quiz ID)
        m_supabaseClient->DeleteQuizAsTeacher(L"Q01", L"002");
    }
}

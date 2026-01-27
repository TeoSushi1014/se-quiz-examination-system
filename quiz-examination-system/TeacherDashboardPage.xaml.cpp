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
}

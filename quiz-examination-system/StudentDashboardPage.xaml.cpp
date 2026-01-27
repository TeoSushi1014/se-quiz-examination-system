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
    }

    void StudentDashboardPage::TakeQuiz_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        ActionMessage().Message(L"Launch quiz interface");
        ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }
}

#include "pch.h"
#include "AdminDashboardPage.xaml.h"
#if __has_include("AdminDashboardPage.g.cpp")
#include "AdminDashboardPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::quiz_examination_system::implementation
{
    AdminDashboardPage::AdminDashboardPage()
    {
        InitializeComponent();
    }

    void AdminDashboardPage::ManageUsers_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        ActionMessage().Message(L"Open user management window");
        ActionMessage().Severity(Microsoft::UI::Xaml::Controls::InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }
}

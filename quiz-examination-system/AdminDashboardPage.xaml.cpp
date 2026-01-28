#include "pch.h"
#include "AdminDashboardPage.xaml.h"
#if __has_include("AdminDashboardPage.g.cpp")
#include "AdminDashboardPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::quiz_examination_system::implementation
{
    AdminDashboardPage::AdminDashboardPage()
    {
        InitializeComponent();
    }
}

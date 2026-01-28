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
    }
}

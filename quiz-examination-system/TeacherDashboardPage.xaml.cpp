#include "pch.h"
#include "TeacherDashboardPage.xaml.h"
#if __has_include("TeacherDashboardPage.g.cpp")
#include "TeacherDashboardPage.g.cpp"
#endif
// #include "QuestionBankPage.xaml.h" // Temporarily commented - not in project yet

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::quiz_examination_system::implementation
{
    TeacherDashboardPage::TeacherDashboardPage()
    {
        InitializeComponent();

        if (TeacherNav().MenuItems().Size() > 0)
        {
            TeacherNav().SelectedItem(TeacherNav().MenuItems().GetAt(0));
            // ContentFrame().Navigate(xaml_typename<quiz_examination_system::QuestionBankPage>());
        }
    }

    void TeacherDashboardPage::TeacherNav_SelectionChanged(
        NavigationView const &,
        NavigationViewSelectionChangedEventArgs const &e)
    {
        auto selectedItem = e.SelectedItem().try_as<NavigationViewItem>();
        if (!selectedItem)
        {
            return;
        }

        auto tag = unbox_value<hstring>(selectedItem.Tag());

        if (tag == L"QuestionBank")
        {
            // ContentFrame().Navigate(xaml_typename<quiz_examination_system::QuestionBankPage>());
        }
        else if (tag == L"ManageQuizzes")
        {
        }
        else if (tag == L"ReviewAttempts")
        {
        }
    }
}

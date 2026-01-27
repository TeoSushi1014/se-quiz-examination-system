#pragma once

#include "TeacherDashboardPage.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct TeacherDashboardPage : TeacherDashboardPageT<TeacherDashboardPage>
    {
        TeacherDashboardPage();

        void TeacherNav_SelectionChanged(
            Microsoft::UI::Xaml::Controls::NavigationView const &sender,
            Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const &e);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct TeacherDashboardPage : TeacherDashboardPageT<TeacherDashboardPage, implementation::TeacherDashboardPage>
    {
    };
}

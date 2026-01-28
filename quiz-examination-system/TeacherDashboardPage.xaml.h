#pragma once

#include "TeacherDashboardPage.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct TeacherDashboardPage : TeacherDashboardPageT<TeacherDashboardPage>
    {
        TeacherDashboardPage();
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct TeacherDashboardPage : TeacherDashboardPageT<TeacherDashboardPage, implementation::TeacherDashboardPage>
    {
    };
}

#pragma once

#include "AdminDashboardPage.g.h"

namespace winrt::quiz_examination_system::implementation
{
    struct AdminDashboardPage : AdminDashboardPageT<AdminDashboardPage>
    {
        AdminDashboardPage();
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct AdminDashboardPage : AdminDashboardPageT<AdminDashboardPage, implementation::AdminDashboardPage>
    {
    };
}

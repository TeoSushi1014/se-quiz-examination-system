#pragma once

#include "MainWindow.g.h"
#include "SupabaseClientAsync.h"
#include <memory>

namespace winrt::quiz_examination_system::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        winrt::fire_and_forget Login_Click(IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void Logout_Click(IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &);
        void NavView_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView const &, Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const &);

    private:
        void UpdateView();

        std::unique_ptr<::quiz_examination_system::SupabaseClientAsync> m_client;
        bool m_authenticated{false};
        hstring m_currentUser;
        hstring m_currentUserId;
        hstring m_currentRole;
        hstring m_currentDbRole;
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}

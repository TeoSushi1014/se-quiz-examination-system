#include "pch.h"
#include "MainWindow.xaml.h"
#include "TeacherDashboardPage.xaml.h"
#include "StudentDashboardPage.xaml.h"
#include "AdminDashboardPage.xaml.h"
#include "SupabaseClientManager.h"
#include "SupabaseClientAsync.h"
#include <winrt/Windows.Data.Json.h>
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Data::Json;

namespace winrt::quiz_examination_system::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        m_client = std::make_unique<::quiz_examination_system::SupabaseClientAsync>();

        auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
        if (manager.HasActiveSession())
        {
            m_authenticated = true;
            m_currentUser = manager.GetUsername();
            m_currentDbRole = manager.GetRole();
            m_currentRole = (m_currentDbRole == L"Admin") ? L"Administrator" : (m_currentDbRole == L"Teacher") ? L"Lecturer"
                                                                                                               : L"Student";
            UpdateView();
        }
    }

    winrt::fire_and_forget MainWindow::Login_Click(IInspectable const &, RoutedEventArgs const &)
    {
        auto lifetime = get_strong();

        auto username = UsernameBox().Text();
        auto password = PasswordBox().Password();

        LoginMessage().IsOpen(false);

        if (username.empty() || password.empty())
        {
            LoginMessage().Message(L"Enter username and password");
            LoginMessage().Severity(InfoBarSeverity::Warning);
            LoginMessage().IsOpen(true);
            co_return;
        }

        LoginButton().IsEnabled(false);
        LoginProgressRing().IsActive(true);
        LoginMessage().Message(L"Signing in...");
        LoginMessage().Severity(InfoBarSeverity::Informational);
        LoginMessage().IsOpen(true);

        try
        {
            auto resultJson = co_await m_client->LoginAsync(username, password);

            // Debug: Show raw JSON response
            OutputDebugStringW((L"Login Response: " + resultJson + L"\n").c_str());

            auto result = JsonObject::Parse(resultJson);
            bool success = result.GetNamedBoolean(L"success", false);

            if (success)
            {
                m_authenticated = true;
                m_currentUser = result.GetNamedString(L"username", L"");
                m_currentUserId = result.GetNamedString(L"userId", L"");
                m_currentDbRole = result.GetNamedString(L"role", L"");
                m_currentRole = result.GetNamedString(L"displayRole", L"");

                auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
                manager.SaveSession(m_currentUserId, m_currentUser, m_currentDbRole);

                LoginMessage().IsOpen(false);
                UpdateView();
            }
            else
            {
                auto errorMessage = result.GetNamedString(L"errorMessage", L"Login failed");
                OutputDebugStringW((L"Login Error: " + errorMessage + L"\n").c_str());
                LoginMessage().Message(errorMessage);
                LoginMessage().Severity(InfoBarSeverity::Error);
                LoginMessage().IsOpen(true);
            }
        }
        catch (hresult_error const &ex)
        {
            LoginMessage().Message(ex.message());
            LoginMessage().Severity(InfoBarSeverity::Error);
            LoginMessage().IsOpen(true);
        }

        LoginButton().IsEnabled(true);
        LoginProgressRing().IsActive(false);
    }

    void MainWindow::Logout_Click(IInspectable const &, RoutedEventArgs const &)
    {
        ContentDialog confirmDialog;
        confirmDialog.XamlRoot(this->Content().XamlRoot());
        confirmDialog.Title(box_value(L"Confirm logout"));
        confirmDialog.Content(box_value(L"Are you sure you want to log out?"));
        confirmDialog.PrimaryButtonText(L"Logout");
        confirmDialog.CloseButtonText(L"Cancel");
        confirmDialog.DefaultButton(ContentDialogButton::Close);

        confirmDialog.PrimaryButtonClick([this](auto &&, auto &&)
                                         {
            auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
            manager.ClearSession();

            m_authenticated = false;
            m_currentUser = L"";
            m_currentUserId = L"";
            m_currentRole = L"";
            m_currentDbRole = L"";
            LoginMessage().IsOpen(false);
            UpdateView(); });

        confirmDialog.ShowAsync();
    }

    void MainWindow::UpdateView()
    {
        LoginPanel().Visibility(m_authenticated ? Visibility::Collapsed : Visibility::Visible);
        DashboardPanel().Visibility(m_authenticated ? Visibility::Visible : Visibility::Collapsed);

        if (m_authenticated)
        {
            WelcomeText().Text(hstring(L"Welcome, ") + m_currentUser);
            RoleText().Text(hstring(L"(") + m_currentRole + hstring(L")"));

            if (m_currentRole == L"Administrator")
            {
                ContentFrame().Navigate(xaml_typename<quiz_examination_system::AdminDashboardPage>());
            }
            else if (m_currentRole == L"Lecturer")
            {
                ContentFrame().Navigate(xaml_typename<quiz_examination_system::TeacherDashboardPage>());
            }
            else if (m_currentRole == L"Student")
            {
                ContentFrame().Navigate(xaml_typename<quiz_examination_system::StudentDashboardPage>());
            }
        }
        else
        {
            UsernameBox().Text(L"");
            PasswordBox().Password(L"");
        }
    }
}

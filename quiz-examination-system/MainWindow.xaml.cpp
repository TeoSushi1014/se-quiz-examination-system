#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::quiz_examination_system::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        Closed({this, &MainWindow::OnClosed});

        m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        m_supabaseClient->OnLoginSuccess = [this](hstring username, hstring displayRole, hstring dbRole)
        {
            OnLoginSuccess(username, displayRole, dbRole);
        };
        m_supabaseClient->OnLoginFailed = [this](hstring message)
        {
            OnLoginFailed(message);
        };
        m_supabaseClient->OnPasswordChanged = [this](hstring message)
        {
            OnPasswordChanged(message);
        };
        m_supabaseClient->OnPasswordChangeFailed = [this](hstring message)
        {
            OnPasswordChangeFailed(message);
        };

        m_dbConnected = ConnectDatabase(L"");

        UpdateView();
    }

    void MainWindow::OnClosed(IInspectable const &, Microsoft::UI::Xaml::WindowEventArgs const &)
    {
        m_isClosing = true;
        if (m_supabaseClient)
        {
            m_supabaseClient->OnLoginSuccess = nullptr;
            m_supabaseClient->OnLoginFailed = nullptr;
            m_supabaseClient->OnPasswordChanged = nullptr;
            m_supabaseClient->OnPasswordChangeFailed = nullptr;
        }
    }

    void MainWindow::ManageUsers_Click(IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        if (!m_dbConnected)
        {
            ActionMessage().Message(L"Database is not connected");
            ActionMessage().Severity(InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }
        if (!m_authenticated)
        {
            ActionMessage().Message(L"Sign in first");
            ActionMessage().Severity(InfoBarSeverity::Warning);
            ActionMessage().IsOpen(true);
            return;
        }
        if (m_currentRole != L"Administrator")
        {
            ActionMessage().Message(L"You do not have permission");
            ActionMessage().Severity(InfoBarSeverity::Warning);
            ActionMessage().IsOpen(true);
            return;
        }
        ActionMessage().Message(L"Open user management");
        ActionMessage().Severity(InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }

    void MainWindow::ManageQuizzes_Click(IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        if (!m_dbConnected)
        {
            ActionMessage().Message(L"Database is not connected");
            ActionMessage().Severity(InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }
        if (!m_authenticated)
        {
            ActionMessage().Message(L"Sign in first");
            ActionMessage().Severity(InfoBarSeverity::Warning);
            ActionMessage().IsOpen(true);
            return;
        }
        if (m_currentRole != L"Administrator" && m_currentRole != L"Lecturer")
        {
            ActionMessage().Message(L"You do not have permission");
            ActionMessage().Severity(InfoBarSeverity::Warning);
            ActionMessage().IsOpen(true);
            return;
        }
        ActionMessage().Message(L"Open quiz management");
        ActionMessage().Severity(InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }

    void MainWindow::ReviewAttempts_Click(IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        if (!m_dbConnected)
        {
            ActionMessage().Message(L"Database is not connected");
            ActionMessage().Severity(InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }
        if (!m_authenticated)
        {
            ActionMessage().Message(L"Sign in first");
            ActionMessage().Severity(InfoBarSeverity::Warning);
            ActionMessage().IsOpen(true);
            return;
        }
        if (m_currentRole != L"Administrator" && m_currentRole != L"Lecturer")
        {
            ActionMessage().Message(L"You do not have permission");
            ActionMessage().Severity(InfoBarSeverity::Warning);
            ActionMessage().IsOpen(true);
            return;
        }
        ActionMessage().Message(L"Open attempt review");
        ActionMessage().Severity(InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }

    void MainWindow::TakeQuiz_Click(IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        if (!m_dbConnected)
        {
            ActionMessage().Message(L"Database is not connected");
            ActionMessage().Severity(InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }
        if (!m_authenticated)
        {
            ActionMessage().Message(L"Sign in first");
            ActionMessage().Severity(InfoBarSeverity::Warning);
            ActionMessage().IsOpen(true);
            return;
        }
        if (m_currentRole != L"Student")
        {
            ActionMessage().Message(L"You do not have permission");
            ActionMessage().Severity(InfoBarSeverity::Warning);
            ActionMessage().IsOpen(true);
            return;
        }
        ActionMessage().Message(L"Start quiz flow");
        ActionMessage().Severity(InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }

    void MainWindow::Login_Click(IInspectable const &, RoutedEventArgs const &)
    {
        auto username = UsernameBox().Text();
        auto password = PasswordBox().Password();

        LoginMessage().IsOpen(false);
        PasswordMessage().IsOpen(false);
        if (!m_dbConnected)
        {
            LoginMessage().Message(L"Database is not connected");
            LoginMessage().Severity(InfoBarSeverity::Error);
            LoginMessage().IsOpen(true);
            return;
        }

        if (username.empty() || password.empty())
        {
            LoginMessage().Message(L"Enter username and password");
            LoginMessage().Severity(InfoBarSeverity::Warning);
            LoginMessage().IsOpen(true);
            return;
        }

        LoginMessage().Message(L"Signing in...");
        LoginMessage().Severity(InfoBarSeverity::Informational);
        LoginMessage().IsOpen(true);
        m_supabaseClient->Login(username, password);
    }

    void MainWindow::Logout_Click(IInspectable const &, RoutedEventArgs const &)
    {
        m_authenticated = false;
        m_currentUser = L"";
        m_currentRole = L"";
        m_currentDbRole = L"";
        PasswordMessage().IsOpen(false);
        LoginMessage().IsOpen(false);
        ActionMessage().IsOpen(false);
        UpdateView();
    }

    void MainWindow::ChangePassword_Click(IInspectable const &, RoutedEventArgs const &)
    {
        PasswordMessage().IsOpen(false);

        if (!m_dbConnected)
        {
            PasswordMessage().Message(L"Database is not connected");
            PasswordMessage().Severity(InfoBarSeverity::Error);
            PasswordMessage().IsOpen(true);
            return;
        }

        if (!m_authenticated)
        {
            PasswordMessage().Message(L"Sign in first");
            PasswordMessage().Severity(InfoBarSeverity::Warning);
            PasswordMessage().IsOpen(true);
            return;
        }

        auto current = CurrentPasswordBox().Password();
        auto next = NewPasswordBox().Password();
        auto confirm = ConfirmPasswordBox().Password();

        if (next.empty() || confirm.empty())
        {
            PasswordMessage().Message(L"Enter the new password");
            PasswordMessage().Severity(InfoBarSeverity::Warning);
            PasswordMessage().IsOpen(true);
            return;
        }

        if (next != confirm)
        {
            PasswordMessage().Message(L"New password does not match");
            PasswordMessage().Severity(InfoBarSeverity::Warning);
            PasswordMessage().IsOpen(true);
            return;
        }

        PasswordMessage().Message(L"Updating...");
        PasswordMessage().Severity(InfoBarSeverity::Informational);
        PasswordMessage().IsOpen(true);
        m_supabaseClient->ChangePassword(m_currentUser, current, next);
    }

    void MainWindow::UpdateView()
    {
        LoginPanel().Visibility(m_authenticated ? Visibility::Collapsed : Visibility::Visible);
        DashboardPanel().Visibility(m_authenticated ? Visibility::Visible : Visibility::Collapsed);

        if (m_authenticated)
        {
            WelcomeText().Text(hstring(L"Welcome, ") + m_currentUser);
            RoleText().Text(hstring(L"(") + m_currentRole + hstring(L")"));

            hstring actions;
            if (m_currentRole == L"Administrator")
            {
                actions = L"Manage users, quizzes, grading, and system settings.";
            }
            else if (m_currentRole == L"Lecturer")
            {
                actions = L"Create questions, schedule quizzes, review attempts.";
            }
            else
            {
                actions = L"Take quizzes, view results, update profile.";
            }

            ActionsText().Text(actions);
            ApplyPermissions(m_currentRole);

            DashboardView().Visibility(Visibility::Visible);
            PasswordView().Visibility(Visibility::Collapsed);

            auto navView = DashboardNav();
            if (navView.MenuItems().Size() > 0)
            {
                auto firstItem = navView.MenuItems().GetAt(0).as<NavigationViewItem>();
                navView.SelectedItem(firstItem);
            }
        }
        else
        {
            WelcomeText().Text(L"");
            RoleText().Text(L"");
            ActionsText().Text(L"");
            ActionMessage().IsOpen(false);
            ApplyPermissions(L"");
        }
    }

    bool MainWindow::ConnectDatabase(hstring const &)
    {
        return true;
    }

    void MainWindow::ApplyPermissions(hstring const &role)
    {
        auto isAdmin = role == L"Administrator";
        auto isLecturer = role == L"Lecturer";
        auto isStudent = role == L"Student";

        ManageUsersBtn().IsEnabled(isAdmin);
        ManageQuizzesBtn().IsEnabled(isAdmin || isLecturer);
        ReviewAttemptsBtn().IsEnabled(isAdmin || isLecturer);
        TakeQuizBtn().IsEnabled(isStudent);
    }

    void MainWindow::OnLoginSuccess(hstring username, hstring displayRole, hstring dbRole)
    {
        if (m_isClosing)
            return;
        m_authenticated = true;
        m_currentUser = username;
        m_currentRole = displayRole;
        m_currentDbRole = dbRole;
        UsernameBox().Text(L"");
        PasswordBox().Password(L"");
        LoginMessage().IsOpen(false);
        UpdateView();
    }

    void MainWindow::OnLoginFailed(hstring message)
    {
        if (m_isClosing)
            return;
        m_authenticated = false;
        m_currentUser = L"";
        m_currentRole = L"";
        m_currentDbRole = L"";
        LoginMessage().Message(message);
        LoginMessage().Severity(InfoBarSeverity::Error);
        LoginMessage().IsOpen(true);
        UpdateView();
    }

    void MainWindow::OnPasswordChanged(hstring message)
    {
        if (m_isClosing)
            return;
        CurrentPasswordBox().Password(L"");
        NewPasswordBox().Password(L"");
        ConfirmPasswordBox().Password(L"");
        PasswordMessage().Message(message);
        PasswordMessage().Severity(InfoBarSeverity::Success);
        PasswordMessage().IsOpen(true);
    }

    void MainWindow::OnPasswordChangeFailed(hstring message)
    {
        if (m_isClosing)
            return;
        PasswordMessage().Message(message);
        PasswordMessage().Severity(InfoBarSeverity::Error);
        PasswordMessage().IsOpen(true);
    }

    void MainWindow::DashboardNav_SelectionChanged(IInspectable const &, IInspectable const &)
    {
        if (m_isClosing)
            return;

        auto navView = DashboardNav();
        auto selectedItem = navView.SelectedItem();
        if (!selectedItem)
            return;

        auto navItem = selectedItem.as<NavigationViewItem>();
        auto tag = unbox_value<hstring>(navItem.Tag());

        if (tag == L"Dashboard")
        {
            DashboardView().Visibility(Visibility::Visible);
            PasswordView().Visibility(Visibility::Collapsed);
        }
        else if (tag == L"Password")
        {
            DashboardView().Visibility(Visibility::Collapsed);
            PasswordView().Visibility(Visibility::Visible);
        }
    }
}

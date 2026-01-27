#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::Foundation;

namespace winrt::quiz_examination_system::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        Closed({this, &MainWindow::OnClosed});

        m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        m_supabaseClient->OnLoginSuccess = [this](hstring username, hstring displayRole, hstring dbRole, hstring userId)
        {
            OnLoginSuccess(username, displayRole, dbRole, userId);
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

    void MainWindow::ManageUsers_Click(IInspectable const &, RoutedEventArgs const &) {}
    void MainWindow::ManageQuestions_Click(IInspectable const &, RoutedEventArgs const &) {}
    void MainWindow::ManageQuizzes_Click(IInspectable const &, RoutedEventArgs const &) {}
    void MainWindow::ReviewAttempts_Click(IInspectable const &, RoutedEventArgs const &) {}
    void MainWindow::TakeQuiz_Click(IInspectable const &, RoutedEventArgs const &) {}

    void MainWindow::Login_Click(IInspectable const &, RoutedEventArgs const &)
    {
        auto username = UsernameBox().Text();
        auto password = PasswordBox().Password();

        LoginMessage().IsOpen(false);
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
        m_currentUserId = L"";
        m_currentRole = L"";
        m_currentDbRole = L"";
        LoginMessage().IsOpen(false);
        UpdateView();
    }

    void MainWindow::ChangePassword_Click(IInspectable const &, RoutedEventArgs const &)
    {
        // Implement password change in a future modal
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
                TypeName adminType;
                adminType.Name = L"quiz_examination_system.AdminDashboardPage";
                adminType.Kind = TypeKind::Metadata;
                ContentFrame().Navigate(adminType);
            }
            else if (m_currentRole == L"Lecturer")
            {
                TypeName teacherType;
                teacherType.Name = L"quiz_examination_system.TeacherDashboardPage";
                teacherType.Kind = TypeKind::Metadata;
                ContentFrame().Navigate(teacherType);
            }
            else if (m_currentRole == L"Student")
            {
                TypeName studentType;
                studentType.Name = L"quiz_examination_system.StudentDashboardPage";
                studentType.Kind = TypeKind::Metadata;
                ContentFrame().Navigate(studentType);
            }
        }
    }

    bool MainWindow::ConnectDatabase(hstring const &)
    {
        return true;
    }

    void MainWindow::ApplyPermissions(hstring const &)
    {
    }

    void MainWindow::OnLoginSuccess(hstring username, hstring displayRole, hstring dbRole, hstring userId)
    {
        if (m_isClosing)
            return;
        m_authenticated = true;
        m_currentUser = username;
        m_currentUserId = userId;
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
        m_currentUserId = L"";
        m_currentRole = L"";
        m_currentDbRole = L"";
        LoginMessage().Message(message);
        LoginMessage().Severity(InfoBarSeverity::Error);
        LoginMessage().IsOpen(true);
        UpdateView();
    }

    void MainWindow::OnPasswordChanged(hstring)
    {
        if (m_isClosing)
            return;
    }

    void MainWindow::OnPasswordChangeFailed(hstring)
    {
        if (m_isClosing)
            return;
    }

    void MainWindow::DashboardNav_SelectionChanged(IInspectable const &, IInspectable const &)
    {
        if (m_isClosing)
            return;
    }
}

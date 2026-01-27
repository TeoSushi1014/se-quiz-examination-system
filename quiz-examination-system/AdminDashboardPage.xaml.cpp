#include "pch.h"
#include "AdminDashboardPage.xaml.h"
#if __has_include("AdminDashboardPage.g.cpp")
#include "AdminDashboardPage.g.cpp"
#endif
#include "UserManagementPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::quiz_examination_system::implementation
{
    AdminDashboardPage::AdminDashboardPage()
    {
        InitializeComponent();
        AdminNavView().SelectedItem(AdminNavView().MenuItems().GetAt(0));
        NavigateToDashboard();
    }

    void AdminDashboardPage::AdminNav_SelectionChanged(NavigationView const &, NavigationViewSelectionChangedEventArgs const &args)
    {
        if (args.SelectedItem())
        {
            auto selectedItem = args.SelectedItem().as<NavigationViewItem>();
            auto tag = unbox_value<hstring>(selectedItem.Tag());

            if (tag == L"dashboard")
            {
                NavigateToDashboard();
            }
            else if (tag == L"users")
            {
                ContentFrame().Navigate(xaml_typename<quiz_examination_system::UserManagementPage>());
            }
            else if (tag == L"quizzes")
            {
                ShowMessage(L"Feature in development", L"Quiz management page will be available soon", InfoBarSeverity::Informational);
            }
            else if (tag == L"reports")
            {
                ShowMessage(L"Feature in development", L"Reports and analytics page will be available soon", InfoBarSeverity::Informational);
            }
            else if (tag == L"logs")
            {
                ShowMessage(L"Feature in development", L"Audit logs page will be available soon", InfoBarSeverity::Informational);
            }
        }
    }

    void AdminDashboardPage::NavigateToDashboard()
    {
        auto scrollViewer = ScrollViewer();
        scrollViewer.Padding({32, 24, 32, 24});

        auto mainStack = StackPanel();
        mainStack.Spacing(24);

        auto headerStack = StackPanel();
        headerStack.Spacing(8);

        auto titleText = TextBlock();
        titleText.Text(L"Administrator dashboard");
        titleText.FontSize(28);
        headerStack.Children().Append(titleText);

        auto descText = TextBlock();
        descText.Text(L"Manage users, quizzes, view reports and monitor system activities");
        descText.FontSize(14);
        descText.Opacity(0.7);
        headerStack.Children().Append(descText);

        mainStack.Children().Append(headerStack);

        auto cardsGrid = Grid();
        cardsGrid.ColumnSpacing(16);
        cardsGrid.RowSpacing(16);

        auto col1 = ColumnDefinition();
        col1.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));
        auto col2 = ColumnDefinition();
        col2.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));
        cardsGrid.ColumnDefinitions().Append(col1);
        cardsGrid.ColumnDefinitions().Append(col2);

        auto row1 = RowDefinition();
        row1.Height(GridLengthHelper::FromValueAndType(1, GridUnitType::Auto));
        auto row2 = RowDefinition();
        row2.Height(GridLengthHelper::FromValueAndType(1, GridUnitType::Auto));
        cardsGrid.RowDefinitions().Append(row1);
        cardsGrid.RowDefinitions().Append(row2);

        auto usersCard = Border();
        usersCard.Padding({24, 24, 24, 24});
        usersCard.CornerRadius({8, 8, 8, 8});
        usersCard.BorderThickness({1, 1, 1, 1});
        usersCard.MinHeight(160);
        usersCard.Background(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 250, 250, 250)));
        usersCard.BorderBrush(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 225, 225, 225)));
        Grid::SetColumn(usersCard, 0);
        Grid::SetRow(usersCard, 0);

        auto usersStack = StackPanel();
        usersStack.Spacing(8);
        auto usersIcon = FontIcon();
        usersIcon.Glyph(L"\uE716");
        usersIcon.FontSize(32);
        usersStack.Children().Append(usersIcon);
        auto usersTitle = TextBlock();
        usersTitle.Text(L"User management");
        usersTitle.FontSize(20);
        usersStack.Children().Append(usersTitle);
        auto usersDesc = TextBlock();
        usersDesc.Text(L"Create, edit and manage user accounts");
        usersDesc.FontSize(12);
        usersDesc.Opacity(0.7);
        usersDesc.TextWrapping(TextWrapping::Wrap);
        usersStack.Children().Append(usersDesc);
        usersCard.Child(usersStack);
        cardsGrid.Children().Append(usersCard);

        auto quizzesCard = Border();
        quizzesCard.Padding({24, 24, 24, 24});
        quizzesCard.CornerRadius({8, 8, 8, 8});
        quizzesCard.BorderThickness({1, 1, 1, 1});
        quizzesCard.MinHeight(160);
        quizzesCard.Background(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 250, 250, 250)));
        quizzesCard.BorderBrush(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 225, 225, 225)));
        Grid::SetColumn(quizzesCard, 1);
        Grid::SetRow(quizzesCard, 0);

        auto quizzesStack = StackPanel();
        quizzesStack.Spacing(8);
        auto quizzesIcon = FontIcon();
        quizzesIcon.Glyph(L"\uE8F1");
        quizzesIcon.FontSize(32);
        quizzesStack.Children().Append(quizzesIcon);
        auto quizzesTitle = TextBlock();
        quizzesTitle.Text(L"Quiz management");
        quizzesTitle.FontSize(20);
        quizzesStack.Children().Append(quizzesTitle);
        auto quizzesDesc = TextBlock();
        quizzesDesc.Text(L"View and permanently delete quizzes with related data");
        quizzesDesc.FontSize(12);
        quizzesDesc.Opacity(0.7);
        quizzesDesc.TextWrapping(TextWrapping::Wrap);
        quizzesStack.Children().Append(quizzesDesc);
        quizzesCard.Child(quizzesStack);
        cardsGrid.Children().Append(quizzesCard);

        auto reportsCard = Border();
        reportsCard.Padding({24, 24, 24, 24});
        reportsCard.CornerRadius({8, 8, 8, 8});
        reportsCard.BorderThickness({1, 1, 1, 1});
        reportsCard.MinHeight(160);
        reportsCard.Background(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 250, 250, 250)));
        reportsCard.BorderBrush(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 225, 225, 225)));
        Grid::SetColumn(reportsCard, 0);
        Grid::SetRow(reportsCard, 1);

        auto reportsStack = StackPanel();
        reportsStack.Spacing(8);
        auto reportsIcon = FontIcon();
        reportsIcon.Glyph(L"\uE9F9");
        reportsIcon.FontSize(32);
        reportsStack.Children().Append(reportsIcon);
        auto reportsTitle = TextBlock();
        reportsTitle.Text(L"Reports and analytics");
        reportsTitle.FontSize(20);
        reportsStack.Children().Append(reportsTitle);
        auto reportsDesc = TextBlock();
        reportsDesc.Text(L"View exam results and export CSV reports");
        reportsDesc.FontSize(12);
        reportsDesc.Opacity(0.7);
        reportsDesc.TextWrapping(TextWrapping::Wrap);
        reportsStack.Children().Append(reportsDesc);
        reportsCard.Child(reportsStack);
        cardsGrid.Children().Append(reportsCard);

        auto logsCard = Border();
        logsCard.Padding({24, 24, 24, 24});
        logsCard.CornerRadius({8, 8, 8, 8});
        logsCard.BorderThickness({1, 1, 1, 1});
        logsCard.MinHeight(160);
        logsCard.Background(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 250, 250, 250)));
        logsCard.BorderBrush(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 225, 225, 225)));
        Grid::SetColumn(logsCard, 1);
        Grid::SetRow(logsCard, 1);

        auto logsStack = StackPanel();
        logsStack.Spacing(8);
        auto logsIcon = FontIcon();
        logsIcon.Glyph(L"\uE7C3");
        logsIcon.FontSize(32);
        logsStack.Children().Append(logsIcon);
        auto logsTitle = TextBlock();
        logsTitle.Text(L"Audit logs");
        logsTitle.FontSize(20);
        logsStack.Children().Append(logsTitle);
        auto logsDesc = TextBlock();
        logsDesc.Text(L"Monitor and view system activity history");
        logsDesc.FontSize(12);
        logsDesc.Opacity(0.7);
        logsDesc.TextWrapping(TextWrapping::Wrap);
        logsStack.Children().Append(logsDesc);
        logsCard.Child(logsStack);
        cardsGrid.Children().Append(logsCard);

        mainStack.Children().Append(cardsGrid);
        scrollViewer.Content(mainStack);

        ContentFrame().Content(scrollViewer);
    }

    void AdminDashboardPage::ShowMessage(hstring const &title, hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        ActionMessage().Title(title);
        ActionMessage().Message(message);
        ActionMessage().Severity(severity);
        ActionMessage().IsOpen(true);
    }

}

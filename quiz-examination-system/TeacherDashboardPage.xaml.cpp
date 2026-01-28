#include "pch.h"
#include "TeacherDashboardPage.xaml.h"
#if __has_include("TeacherDashboardPage.g.cpp")
#include "TeacherDashboardPage.g.cpp"
#endif
#include "PageHelper.h"
#include "QuestionBankPage.xaml.h"
#include "QuizManagementPage.xaml.h"
#include "ReportsPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::quiz_examination_system::implementation
{
    TeacherDashboardPage::TeacherDashboardPage()
    {
        InitializeComponent();
        TeacherNav().SelectedItem(TeacherNav().MenuItems().GetAt(0));
        NavigateToDashboard();
    }

    void TeacherDashboardPage::TeacherNav_SelectionChanged(
        NavigationView const &,
        NavigationViewSelectionChangedEventArgs const &args)
    {
        if (args.SelectedItem())
        {
            auto selectedItem = args.SelectedItem().as<NavigationViewItem>();
            auto tag = unbox_value<hstring>(selectedItem.Tag());

            if (tag == L"dashboard")
            {
                NavigateToDashboard();
            }
            else if (tag == L"QuestionBank")
            {
                ContentFrame().Navigate(xaml_typename<quiz_examination_system::QuestionBankPage>());
            }
            else if (tag == L"ManageQuizzes")
            {
                ContentFrame().Navigate(xaml_typename<quiz_examination_system::QuizManagementPage>());
            }
            else if (tag == L"ReviewAttempts")
            {
            }
            else if (tag == L"Reports")
            {
                ContentFrame().Navigate(xaml_typename<quiz_examination_system::ReportsPage>());
            }
        }
    }

    void TeacherDashboardPage::NavigateToDashboard()
    {
        auto scrollViewer = ScrollViewer();
        scrollViewer.Padding({32, 24, 32, 24});

        auto mainStack = StackPanel();
        mainStack.Spacing(24);

        auto headerStack = StackPanel();
        headerStack.Spacing(8);

        auto titleText = TextBlock();
        titleText.Text(L"Teacher dashboard");
        titleText.FontSize(28);
        headerStack.Children().Append(titleText);

        auto descText = TextBlock();
        descText.Text(L"Manage questions, create quizzes, review student attempts and view reports");
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

        auto questionBankCard = Border();
        questionBankCard.Padding({24, 24, 24, 24});
        questionBankCard.CornerRadius({8, 8, 8, 8});
        questionBankCard.BorderThickness({1, 1, 1, 1});
        questionBankCard.MinHeight(160);
        questionBankCard.Background(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 250, 250, 250)));
        questionBankCard.BorderBrush(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 225, 225, 225)));
        Grid::SetColumn(questionBankCard, 0);
        Grid::SetRow(questionBankCard, 0);

        auto questionBankStack = StackPanel();
        questionBankStack.Spacing(8);
        auto questionBankIcon = FontIcon();
        questionBankIcon.Glyph(L"\uE8E6");
        questionBankIcon.FontSize(32);
        questionBankStack.Children().Append(questionBankIcon);
        auto questionBankTitle = TextBlock();
        questionBankTitle.Text(L"Question bank");
        questionBankTitle.FontSize(20);
        questionBankStack.Children().Append(questionBankTitle);
        auto questionBankDesc = TextBlock();
        questionBankDesc.Text(L"Create and manage quiz questions");
        questionBankDesc.FontSize(12);
        questionBankDesc.Opacity(0.7);
        questionBankDesc.TextWrapping(TextWrapping::Wrap);
        questionBankStack.Children().Append(questionBankDesc);
        questionBankCard.Child(questionBankStack);
        cardsGrid.Children().Append(questionBankCard);

        auto manageQuizzesCard = Border();
        manageQuizzesCard.Padding({24, 24, 24, 24});
        manageQuizzesCard.CornerRadius({8, 8, 8, 8});
        manageQuizzesCard.BorderThickness({1, 1, 1, 1});
        manageQuizzesCard.MinHeight(160);
        manageQuizzesCard.Background(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 250, 250, 250)));
        manageQuizzesCard.BorderBrush(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 225, 225, 225)));
        Grid::SetColumn(manageQuizzesCard, 1);
        Grid::SetRow(manageQuizzesCard, 0);

        auto manageQuizzesStack = StackPanel();
        manageQuizzesStack.Spacing(8);
        auto manageQuizzesIcon = FontIcon();
        manageQuizzesIcon.Glyph(L"\uE8C8");
        manageQuizzesIcon.FontSize(32);
        manageQuizzesStack.Children().Append(manageQuizzesIcon);
        auto manageQuizzesTitle = TextBlock();
        manageQuizzesTitle.Text(L"Manage quizzes");
        manageQuizzesTitle.FontSize(20);
        manageQuizzesStack.Children().Append(manageQuizzesTitle);
        auto manageQuizzesDesc = TextBlock();
        manageQuizzesDesc.Text(L"Create, edit and publish quizzes");
        manageQuizzesDesc.FontSize(12);
        manageQuizzesDesc.Opacity(0.7);
        manageQuizzesDesc.TextWrapping(TextWrapping::Wrap);
        manageQuizzesStack.Children().Append(manageQuizzesDesc);
        manageQuizzesCard.Child(manageQuizzesStack);
        cardsGrid.Children().Append(manageQuizzesCard);

        auto reviewAttemptsCard = Border();
        reviewAttemptsCard.Padding({24, 24, 24, 24});
        reviewAttemptsCard.CornerRadius({8, 8, 8, 8});
        reviewAttemptsCard.BorderThickness({1, 1, 1, 1});
        reviewAttemptsCard.MinHeight(160);
        reviewAttemptsCard.Background(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 250, 250, 250)));
        reviewAttemptsCard.BorderBrush(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 225, 225, 225)));
        Grid::SetColumn(reviewAttemptsCard, 0);
        Grid::SetRow(reviewAttemptsCard, 1);

        auto reviewAttemptsStack = StackPanel();
        reviewAttemptsStack.Spacing(8);
        auto reviewAttemptsIcon = FontIcon();
        reviewAttemptsIcon.Glyph(L"\uE81E");
        reviewAttemptsIcon.FontSize(32);
        reviewAttemptsStack.Children().Append(reviewAttemptsIcon);
        auto reviewAttemptsTitle = TextBlock();
        reviewAttemptsTitle.Text(L"Review attempts");
        reviewAttemptsTitle.FontSize(20);
        reviewAttemptsStack.Children().Append(reviewAttemptsTitle);
        auto reviewAttemptsDesc = TextBlock();
        reviewAttemptsDesc.Text(L"Review and grade student quiz attempts");
        reviewAttemptsDesc.FontSize(12);
        reviewAttemptsDesc.Opacity(0.7);
        reviewAttemptsDesc.TextWrapping(TextWrapping::Wrap);
        reviewAttemptsStack.Children().Append(reviewAttemptsDesc);
        reviewAttemptsCard.Child(reviewAttemptsStack);
        cardsGrid.Children().Append(reviewAttemptsCard);

        auto reportsCard = Border();
        reportsCard.Padding({24, 24, 24, 24});
        reportsCard.CornerRadius({8, 8, 8, 8});
        reportsCard.BorderThickness({1, 1, 1, 1});
        reportsCard.MinHeight(160);
        reportsCard.Background(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 250, 250, 250)));
        reportsCard.BorderBrush(Media::SolidColorBrush(Microsoft::UI::ColorHelper::FromArgb(255, 225, 225, 225)));
        Grid::SetColumn(reportsCard, 1);
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

        mainStack.Children().Append(cardsGrid);
        scrollViewer.Content(mainStack);

        ContentFrame().Content(scrollViewer);
    }

    void TeacherDashboardPage::ShowMessage(hstring const &title, hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), title, message, severity);
    }
}

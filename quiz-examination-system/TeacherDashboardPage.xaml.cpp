#include "pch.h"
#include "TeacherDashboardPage.xaml.h"
#if __has_include("TeacherDashboardPage.g.cpp")
#include "TeacherDashboardPage.g.cpp"
#endif
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <cstdlib>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Data::Json;

namespace winrt::quiz_examination_system::implementation
{
    TeacherDashboardPage::TeacherDashboardPage()
    {
        InitializeComponent();
        TitleText().Text(L"Question Bank");
        DescriptionText().Text(L"Manage your question bank");

        if (TeacherNav().MenuItems().Size() > 0)
        {
            TeacherNav().SelectedItem(TeacherNav().MenuItems().GetAt(0));
        }

        QuestionBank_Click(nullptr, nullptr);
    }

    void TeacherDashboardPage::TeacherNav_SelectionChanged(NavigationView const &, NavigationViewSelectionChangedEventArgs const &e)
    {
        auto tag = unbox_value<hstring>(e.SelectedItem().as<NavigationViewItem>().Tag());
        UpdateContent(tag);
    }

    void TeacherDashboardPage::UpdateContent(hstring const &section)
    {
        ActionMessage().IsOpen(false);
        ContentArea().Children().Clear();

        if (section == L"QuestionBank")
        {
            TitleText().Text(L"Question Bank");
            DescriptionText().Text(L"Create and manage your questions");
            QuestionBank_Click(nullptr, nullptr);
        }
        else if (section == L"ManageQuizzes")
        {
            TitleText().Text(L"Manage Quizzes");
            DescriptionText().Text(L"Create quizzes and assign questions");
            ManageQuizzes_Click(nullptr, nullptr);
        }
        else if (section == L"ReviewAttempts")
        {
            TitleText().Text(L"Review Attempts");
            DescriptionText().Text(L"Review student quiz attempts and grades");
            ReviewAttempts_Click(nullptr, nullptr);
        }
    }

    void TeacherDashboardPage::QuestionBank_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        ContentArea().Children().Clear();

        TextBlock headerBlock;
        headerBlock.Text(L"Question Bank Management");
        headerBlock.FontSize(20);
        Windows::UI::Text::FontWeight fw;
        fw.Weight = 600;
        headerBlock.FontWeight(fw);
        ContentArea().Children().Append(headerBlock);

        StackPanel buttonPanel;
        buttonPanel.Orientation(Microsoft::UI::Xaml::Controls::Orientation::Horizontal);
        buttonPanel.Spacing(12);
        buttonPanel.Margin({0, 12, 0, 12});

        Button addBtn;
        addBtn.Content(box_value(L"Add Question"));
        addBtn.Click({this, &TeacherDashboardPage::AddQuestion_Click});
        buttonPanel.Children().Append(addBtn);

        ContentArea().Children().Append(buttonPanel);

        m_questionsListView = ListView();
        m_questionsListView.Height(400);
        m_questionsListView.BorderThickness({1, 1, 1, 1});
        ContentArea().Children().Append(m_questionsListView);

        LoadQuestions();
    }

    void TeacherDashboardPage::LoadQuestions()
    {
        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        auto dispatcher = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();

        m_supabaseClient->OnQuestionsFetched = [this, dispatcher](hstring questionsJson)
        {
            dispatcher.TryEnqueue([this, questionsJson]()
                                  {
                m_questionsListView.Items().Clear();

                try
                {
                    auto questionsArray = JsonArray::Parse(questionsJson);
                    for (uint32_t i = 0; i < questionsArray.Size(); ++i)
                    {
                        auto qObj = questionsArray.GetObjectAt(i);

                        StackPanel itemPanel;
                        itemPanel.Orientation(Microsoft::UI::Xaml::Controls::Orientation::Horizontal);
                        itemPanel.Spacing(12);
                        itemPanel.Padding({8, 8, 8, 8});

                        TextBlock textBlock;
                        hstring displayText = qObj.GetNamedString(L"question_text", L"") + L" | " +
                                              qObj.GetNamedString(L"difficulty_level", L"") + L" | " +
                                              qObj.GetNamedString(L"topic", L"");
                        textBlock.Text(displayText);
                        textBlock.Width(500);
                        itemPanel.Children().Append(textBlock);

                        Button deleteBtn;
                        deleteBtn.Content(box_value(L"Delete"));
                        deleteBtn.Tag(box_value(qObj.GetNamedString(L"id")));
                        deleteBtn.Click({this, &TeacherDashboardPage::DeleteQuestion_Click});
                        itemPanel.Children().Append(deleteBtn);

                        m_questionsListView.Items().Append(itemPanel);
                    }
                }
                catch (...)
                {
                    ShowMessage(L"Error", L"Failed to parse questions", InfoBarSeverity::Error);
                } });
        };

        m_supabaseClient->GetQuestions(L"002");
    }

    void TeacherDashboardPage::AddQuestion_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowAddQuestionDialog();
    }

    winrt::fire_and_forget TeacherDashboardPage::ShowAddQuestionDialog()
    {
        auto lifetime = get_strong();

        ContentDialog dialog;
        dialog.XamlRoot(this->XamlRoot());
        dialog.Title(box_value(L"Add New Question"));
        dialog.PrimaryButtonText(L"Save");
        dialog.CloseButtonText(L"Cancel");

        StackPanel contentPanel;
        contentPanel.Spacing(12);

        TextBox questionTextBox;
        questionTextBox.Header(box_value(L"Question Text"));
        questionTextBox.AcceptsReturn(true);
        questionTextBox.Height(80);
        contentPanel.Children().Append(questionTextBox);

        TextBox optionABox;
        optionABox.Header(box_value(L"Option A"));
        contentPanel.Children().Append(optionABox);

        TextBox optionBBox;
        optionBBox.Header(box_value(L"Option B"));
        contentPanel.Children().Append(optionBBox);

        TextBox optionCBox;
        optionCBox.Header(box_value(L"Option C"));
        contentPanel.Children().Append(optionCBox);

        TextBox optionDBox;
        optionDBox.Header(box_value(L"Option D"));
        contentPanel.Children().Append(optionDBox);

        ComboBox correctOptionBox;
        correctOptionBox.Header(box_value(L"Correct Option"));
        correctOptionBox.Items().Append(box_value(L"A"));
        correctOptionBox.Items().Append(box_value(L"B"));
        correctOptionBox.Items().Append(box_value(L"C"));
        correctOptionBox.Items().Append(box_value(L"D"));
        correctOptionBox.SelectedIndex(0);
        contentPanel.Children().Append(correctOptionBox);

        ComboBox difficultyBox;
        difficultyBox.Header(box_value(L"Difficulty"));
        difficultyBox.Items().Append(box_value(L"easy"));
        difficultyBox.Items().Append(box_value(L"medium"));
        difficultyBox.Items().Append(box_value(L"hard"));
        difficultyBox.SelectedIndex(0);
        contentPanel.Children().Append(difficultyBox);

        TextBox topicBox;
        topicBox.Header(box_value(L"Topic"));
        contentPanel.Children().Append(topicBox);

        dialog.Content(contentPanel);

        auto result = co_await dialog.ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            if (!m_supabaseClient)
            {
                m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
            }

            auto dispatcher = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();

            m_supabaseClient->OnQuestionValidatedCreated = [this, lifetime, dispatcher](bool success, hstring message)
            {
                dispatcher.TryEnqueue([this, success, message]()
                                      {
                    if (success)
                    {
                        ShowMessage(L"Success", message, InfoBarSeverity::Success);
                        LoadQuestions();
                    }
                    else
                    {
                        ShowMessage(L"Error", message, InfoBarSeverity::Error);
                    } });
            };

            hstring questionId = L"Q" + to_hstring(rand() % 10000);

            m_supabaseClient->CreateQuestionValidated(
                questionId,
                L"002",
                questionTextBox.Text(),
                optionABox.Text(),
                optionBBox.Text(),
                optionCBox.Text(),
                optionDBox.Text(),
                unbox_value<hstring>(correctOptionBox.SelectedItem()),
                unbox_value<hstring>(difficultyBox.SelectedItem()),
                topicBox.Text());
        }
    }

    void TeacherDashboardPage::DeleteQuestion_Click(winrt::Windows::Foundation::IInspectable const &sender, RoutedEventArgs const &)
    {
        auto button = sender.as<Button>();
        hstring questionId = unbox_value<hstring>(button.Tag());

        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        auto dispatcher = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();

        m_supabaseClient->OnQuestionDeleteResult = [this, dispatcher](hstring status, hstring message, int quizCount)
        {
            dispatcher.TryEnqueue([this, status, message, quizCount]()
                                  {
                if (status == L"deleted")
                {
                    ShowMessage(L"Success", message, InfoBarSeverity::Success);
                    LoadQuestions();
                }
                else if (status == L"blocked")
                {
                    ShowMessage(L"Error", hstring(L"Cannot delete question. It is used in ") + to_hstring(quizCount) + L" active quiz(es)", InfoBarSeverity::Error);
                }
                else
                {
                    ShowMessage(L"Error", message, InfoBarSeverity::Error);
                } });
        };

        m_supabaseClient->DeleteQuestionSafe(questionId);
    }

    void TeacherDashboardPage::ManageQuizzes_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        ActionMessage().Message(L"Quiz management interface");
        ActionMessage().Severity(InfoBarSeverity::Informational);
        ActionMessage().IsOpen(true);
    }

    void TeacherDashboardPage::ReviewAttempts_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);
        ContentArea().Children().Clear();

        TextBlock headerBlock;
        headerBlock.Text(L"Quiz Reports & Export");
        headerBlock.FontSize(20);
        Windows::UI::Text::FontWeight fw;
        fw.Weight = 600;
        headerBlock.FontWeight(fw);
        ContentArea().Children().Append(headerBlock);

        StackPanel buttonPanel;
        buttonPanel.Orientation(Microsoft::UI::Xaml::Controls::Orientation::Horizontal);
        buttonPanel.Spacing(12);

        Button viewReportBtn;
        viewReportBtn.Content(box_value(L"View Report (JSON)"));
        viewReportBtn.Click({this, &TeacherDashboardPage::DemoViewQuizReport_Click});
        buttonPanel.Children().Append(viewReportBtn);

        Button exportCsvBtn;
        exportCsvBtn.Content(box_value(L"Export CSV"));
        exportCsvBtn.Click({this, &TeacherDashboardPage::DemoExportCsvReport_Click});
        buttonPanel.Children().Append(exportCsvBtn);

        ContentArea().Children().Append(buttonPanel);
    }

    void TeacherDashboardPage::ShowMessage(hstring const &title, hstring const &message, InfoBarSeverity severity)
    {
        ActionMessage().Title(title);
        ActionMessage().Message(message);
        ActionMessage().Severity(severity);
        ActionMessage().IsOpen(true);
    }

    // =====================================================
    // UC02: Question Bank Management - Demo Functions
    // =====================================================

    void TeacherDashboardPage::DemoCreateQuestion_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Creating Question", L"Calling RPC: create_question_validated...", InfoBarSeverity::Informational);

        // Demo data - create a sample question
        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        // Set callback
        m_supabaseClient->OnQuestionValidatedCreated = [this](bool success, hstring message)
        {
            if (success)
            {
                ShowMessage(L"Success", message, InfoBarSeverity::Success);
            }
            else
            {
                ShowMessage(L"Validation Failed", message, InfoBarSeverity::Error);
            }
        };

        // Call RPC with demo data
        m_supabaseClient->CreateQuestionValidated(
            L"DQ1",                 // id
            L"002",                 // teacher_id
            L"What is 2+2?",        // question_text
            L"3", L"4", L"5", L"6", // options
            L"B",                   // correct (B = 4)
            L"easy",                // difficulty
            L"Math"                 // topic
        );
    }

    void TeacherDashboardPage::DemoDeleteQuestion_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Deleting Question", L"Calling RPC: delete_question_safe for DQ1...", InfoBarSeverity::Informational);

        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        // Set callback
        m_supabaseClient->OnQuestionDeleteResult = [this](hstring status, hstring message, int quizCount)
        {
            if (status == L"success")
            {
                ShowMessage(L"Deleted", message, InfoBarSeverity::Success);
            }
            else if (status == L"blocked")
            {
                hstring msg = message + L" (Used in " + winrt::to_hstring(quizCount) + L" quiz(es))";
                ShowMessage(L"Cannot Delete", msg, InfoBarSeverity::Warning);
            }
            else
            {
                ShowMessage(L"Error", message, InfoBarSeverity::Error);
            }
        };

        // Try to delete the demo question
        m_supabaseClient->DeleteQuestionSafe(L"DQ1");
    }

    void TeacherDashboardPage::DemoDeleteQuestionInUse_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Test: Delete In-Use Question", L"This will demonstrate blocking when question is in a quiz...", InfoBarSeverity::Informational);

        // First create a quiz that uses a question, then try to delete it
        // This is for demonstration of NFR-07 (Integrity Constraints)
        ShowMessage(L"Demo", L"See RPC_INTEGRATION_GUIDE.md for full implementation", InfoBarSeverity::Informational);
    }

    void TeacherDashboardPage::DemoDeleteQuiz_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ShowMessage(L"Deleting Quiz", L"Calling RPC: delete_quiz_teacher...", InfoBarSeverity::Informational);

        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        // Set callback
        m_supabaseClient->OnQuizDeleteResult = [this](bool success, hstring message, int attemptCount)
        {
            if (success)
            {
                ShowMessage(L"Deleted", message, InfoBarSeverity::Success);
            }
            else
            {
                if (attemptCount > 0)
                {
                    hstring msg = message + L" (" + winrt::to_hstring(attemptCount) + L" student(s) took this quiz)";
                    ShowMessage(L"Cannot Delete", msg, InfoBarSeverity::Warning);
                }
                else
                {
                    ShowMessage(L"Error", message, InfoBarSeverity::Error);
                }
            }
        };

        // Try to delete a quiz (replace with actual quiz ID)
        m_supabaseClient->DeleteQuizAsTeacher(L"Q01", L"002");
    }

    void TeacherDashboardPage::DemoViewQuizReport_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        ActionMessage().IsOpen(false);

        if (!m_supabaseClient)
        {
            ActionMessage().Message(L"Database client not initialized");
            ActionMessage().Severity(InfoBarSeverity::Error);
            ActionMessage().IsOpen(true);
            return;
        }

        m_supabaseClient->OnQuizReportLoaded = [this](bool success, std::vector<::quiz_examination_system::SupabaseClient::AttemptReportRow> reports)
        {
            if (success && !reports.empty())
            {
                hstring message = hstring(L"Quiz Report - Total Attempts: ") + to_hstring(reports.size()) + L"\n\n";

                std::map<hstring, int> studentAttempts;
                for (const auto &r : reports)
                {
                    studentAttempts[r.username]++;
                }

                int count = 0;
                for (const auto &pair : studentAttempts)
                {
                    if (count < 5)
                    {
                        message = message + pair.first + L": " + to_hstring(pair.second) + L" attempt(s)\n";
                        count++;
                    }
                }

                ActionMessage().Message(message);
                ActionMessage().Severity(InfoBarSeverity::Success);
            }
            else
            {
                ActionMessage().Message(L"No attempts found for this quiz");
                ActionMessage().Severity(InfoBarSeverity::Warning);
            }
            ActionMessage().IsOpen(true);
        };

        m_supabaseClient->GetQuizAttemptsReport(L"Q01");
    }

    winrt::fire_and_forget TeacherDashboardPage::DemoExportCsvReport_Click(winrt::Windows::Foundation::IInspectable const &, RoutedEventArgs const &)
    {
        auto lifetime = get_strong();

        ShowMessage(L"Exporting CSV", L"Requesting CSV report from Supabase...", InfoBarSeverity::Informational);

        if (!m_supabaseClient)
        {
            m_supabaseClient = std::make_unique<::quiz_examination_system::SupabaseClient>();
        }

        co_await winrt::resume_background();

        auto dispatcher = Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();

        m_supabaseClient->OnCsvReportLoaded = [this, dispatcher, lifetime](bool success, hstring csvContent)
        {
            dispatcher.TryEnqueue([this, success, csvContent, lifetime]() -> winrt::fire_and_forget
                                  {
                auto lifetime_inner = get_strong();

                if (!success || csvContent.empty())
                {
                    ShowMessage(L"Error", L"Failed to get CSV report", InfoBarSeverity::Error);
                    co_return;
                }

                FileSavePicker savePicker;
                savePicker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
                savePicker.FileTypeChoices().Insert(L"CSV File", winrt::single_threaded_vector<hstring>({L".csv"}));
                savePicker.SuggestedFileName(L"QuizReport");

                auto file = co_await savePicker.PickSaveFileAsync();

                if (file)
                {
                    try
                    {
                        co_await FileIO::WriteTextAsync(file, csvContent);
                        ShowMessage(L"Success", hstring(L"Exported to: ") + file.Path(), InfoBarSeverity::Success);
                    }
                    catch (...)
                    {
                        ShowMessage(L"Error", L"Failed to write file", InfoBarSeverity::Error);
                    }
                } });
        };

        m_supabaseClient->GetQuizReportCsv(L"Q01");
    }
}

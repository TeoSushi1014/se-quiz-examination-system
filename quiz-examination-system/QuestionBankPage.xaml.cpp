#include "pch.h"
#include "QuestionBankPage.xaml.h"
#if __has_include("QuestionBankPage.g.cpp")
#include "QuestionBankPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::quiz_examination_system::implementation
{
    QuestionBankPage::QuestionBankPage()
    {
        InitializeComponent();
        m_questions = single_threaded_observable_vector<quiz_examination_system::QuestionItem>();
        m_client = std::make_unique<::quiz_examination_system::SupabaseClientAsync>();
    }

    void QuestionBankPage::Page_Loaded(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        LoadQuestions();
    }

    winrt::fire_and_forget QuestionBankPage::LoadQuestions()
    {
        auto lifetime = get_strong();

        LoadingRing().IsActive(true);

        auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
        auto teacherId = manager.GetUserId();

        try
        {
            auto questionsJson = co_await m_client->GetQuestionsJsonAsync(teacherId);

            m_questions.Clear();
            auto questionsArray = Windows::Data::Json::JsonArray::Parse(questionsJson);

            for (uint32_t i = 0; i < questionsArray.Size(); ++i)
            {
                auto qObj = questionsArray.GetObjectAt(i);

                auto item = make<QuestionItem>();
                item.QuestionId(qObj.GetNamedString(L"id", L""));
                item.QuestionText(qObj.GetNamedString(L"question_text", L""));
                item.DifficultyLevel(qObj.GetNamedString(L"difficulty_level", L"medium"));
                item.Topic(qObj.GetNamedString(L"topic", L"General"));

                m_questions.Append(item);
            }

            QuestionListView().ItemsSource(m_questions);
        }
        catch (hresult_error const &ex)
        {
            ShowMessage(ex.message(), InfoBarSeverity::Error);
        }

        LoadingRing().IsActive(false);
    }

    void QuestionBankPage::AddQuestion_Click(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        ShowAddQuestionDialog();
    }

    winrt::fire_and_forget QuestionBankPage::ShowAddQuestionDialog()
    {
        auto lifetime = get_strong();

        InputQuestionText().Text(L"");
        InputOptionA().Text(L"");
        InputOptionB().Text(L"");
        InputOptionC().Text(L"");
        InputOptionD().Text(L"");
        InputTopic().Text(L"");
        ComboCorrectOption().SelectedIndex(0);
        ComboDifficulty().SelectedIndex(1);

        auto result = co_await AddQuestionDialog().ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            auto qText = InputQuestionText().Text();
            auto optA = InputOptionA().Text();
            auto optB = InputOptionB().Text();
            auto optC = InputOptionC().Text();
            auto optD = InputOptionD().Text();
            auto topic = InputTopic().Text();

            if (qText.empty() || optA.empty() || optB.empty() || optC.empty() || optD.empty())
            {
                ShowMessage(L"Question text and all 4 options are required", InfoBarSeverity::Warning);
                co_return;
            }

            std::set<std::wstring> uniqueOptions = {
                std::wstring(optA),
                std::wstring(optB),
                std::wstring(optC),
                std::wstring(optD)};

            if (uniqueOptions.size() != 4)
            {
                ShowMessage(L"All 4 options must be unique", InfoBarSeverity::Warning);
                co_return;
            }

            auto correctOpt = unbox_value<hstring>(ComboCorrectOption().SelectedItem().as<ComboBoxItem>().Content());
            auto difficulty = unbox_value<hstring>(ComboDifficulty().SelectedItem().as<ComboBoxItem>().Content());

            auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
            auto teacherId = manager.GetUserId();

            hstring questionId = to_hstring(std::chrono::system_clock::now().time_since_epoch().count() % 1000000);

            try
            {
                auto createResult = co_await m_client->CreateQuestionValidatedAsync(
                    questionId, teacherId, qText, optA, optB, optC, optD, correctOpt, difficulty, topic);

                if (createResult.success)
                {
                    ShowMessage(L"Question created successfully", InfoBarSeverity::Success);
                    LoadQuestions();
                }
                else
                {
                    ShowMessage(createResult.message, InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                ShowMessage(ex.message(), InfoBarSeverity::Error);
            }
        }
    }

    void QuestionBankPage::ImportQuestions_Click(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        ShowMessage(L"Import feature not implemented yet", InfoBarSeverity::Informational);
    }

    void QuestionBankPage::RefreshQuestions_Click(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        LoadQuestions();
    }

    void QuestionBankPage::EditQuestion_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        auto btn = sender.as<Button>();
        auto questionId = unbox_value<hstring>(btn.Tag());
        ShowMessage(hstring(L"Edit question: ") + questionId + L" (not implemented)", InfoBarSeverity::Informational);
    }

    winrt::fire_and_forget QuestionBankPage::DeleteQuestion_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        auto lifetime = get_strong();

        auto btn = sender.as<Button>();
        auto questionId = unbox_value<hstring>(btn.Tag());

        ContentDialog confirmDialog;
        confirmDialog.XamlRoot(this->XamlRoot());
        confirmDialog.Title(box_value(L"Confirm deletion"));
        confirmDialog.Content(box_value(hstring(L"Are you sure you want to delete this question?")));
        confirmDialog.PrimaryButtonText(L"Delete");
        confirmDialog.CloseButtonText(L"Cancel");
        confirmDialog.DefaultButton(ContentDialogButton::Close);

        auto result = co_await confirmDialog.ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            try
            {
                bool success = co_await m_client->DeleteQuestionSafeAsync(questionId);

                if (success)
                {
                    ShowMessage(L"Question deleted successfully", InfoBarSeverity::Success);
                    LoadQuestions();
                }
                else
                {
                    ShowMessage(L"Failed to delete question", InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                ShowMessage(ex.message(), InfoBarSeverity::Error);
            }
        }
    }

    void QuestionBankPage::ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        ActionMessage().Message(message);
        ActionMessage().Severity(severity);
        ActionMessage().IsOpen(true);
    }
}

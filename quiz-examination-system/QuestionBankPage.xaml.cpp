#include "pch.h"
#include "QuestionBankPage.xaml.h"
#if __has_include("QuestionBankPage.g.cpp")
#include "QuestionBankPage.g.cpp"
#endif
#include "PageHelper.h"

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

        OutputDebugStringW(L"[LoadQuestions] Called - Starting question refresh\n");
        LoadingRing().IsActive(true);

        auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
        auto teacherIdRaw = manager.GetUserId();
        std::wstring teacherIdStr(teacherIdRaw);
        teacherIdStr.erase(teacherIdStr.find_last_not_of(L" \t\n\r") + 1);
        hstring teacherId = hstring(teacherIdStr);

        try
        {
            OutputDebugStringW((L"[LoadQuestions] Fetching questions for teacherId: '" + teacherId + L"' (length: " + to_hstring(teacherId.size()) + L")\n").c_str());
            auto questionsJson = co_await m_client->GetQuestionsJsonAsync(teacherId);
            OutputDebugStringW((L"[LoadQuestions] Received JSON: " + std::wstring(questionsJson).substr(0, 100) + L"...\n").c_str());

            m_questions.Clear();
            auto questionsArray = Windows::Data::Json::JsonArray::Parse(questionsJson);
            OutputDebugStringW((L"[LoadQuestions] Parsed " + std::to_wstring(questionsArray.Size()) + L" questions\n").c_str());

            for (uint32_t i = 0; i < questionsArray.Size(); ++i)
            {
                auto qObj = questionsArray.GetObjectAt(i);

                auto item = make<QuestionItem>();
                item.QuestionId(qObj.GetNamedString(L"id", L""));
                item.QuestionText(qObj.GetNamedString(L"question_text", L""));
                item.DifficultyLevel(qObj.GetNamedString(L"difficulty_level", L"medium"));
                item.Topic(qObj.GetNamedString(L"topic", L"General"));
                item.OptionA(qObj.GetNamedString(L"option_a", L""));
                item.OptionB(qObj.GetNamedString(L"option_b", L""));
                item.OptionC(qObj.GetNamedString(L"option_c", L""));
                item.OptionD(qObj.GetNamedString(L"option_d", L""));
                item.CorrectOption(qObj.GetNamedString(L"correct_option", L"A"));

                m_questions.Append(item);
            }

            QuestionListView().ItemsSource(m_questions);
            OutputDebugStringW((L"[LoadQuestions] SUCCESS - Added " + std::to_wstring(m_questions.Size()) + L" questions to UI\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[LoadQuestions] ERROR: " + ex.message() + L"\n").c_str());
            ShowMessage(ex.message(), InfoBarSeverity::Error);
        }
        catch (...)
        {
            OutputDebugStringW(L"[LoadQuestions] Unknown error\n");
        }

        LoadingRing().IsActive(false);
        OutputDebugStringW(L"[LoadQuestions] Completed\n");
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

            auto trimString = [](hstring const &str) -> hstring
            {
                if (str.empty())
                    return str;
                std::wstring s(str);
                size_t start = s.find_first_not_of(L" \t\n\r");
                if (start == std::wstring::npos)
                    return L"";
                size_t end = s.find_last_not_of(L" \t\n\r");
                return hstring(s.substr(start, end - start + 1));
            };

            qText = trimString(qText);
            optA = trimString(optA);
            optB = trimString(optB);
            optC = trimString(optC);
            optD = trimString(optD);
            topic = trimString(topic);

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
            auto teacherIdRaw = manager.GetUserId();
            std::wstring teacherIdStr(teacherIdRaw);
            teacherIdStr.erase(teacherIdStr.find_last_not_of(L" \t\n\r") + 1);
            hstring teacherId = hstring(teacherIdStr);

            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            auto idNum = (timestamp % 900000) + 100000;
            hstring questionId = to_hstring(idNum);

            OutputDebugStringW((L"[CreateQuestion] Starting creation with ID: " + questionId + L"\n").c_str());
            OutputDebugStringW((L"[CreateQuestion] TeacherId: '" + teacherId + L"' (length: " + to_hstring(teacherId.size()) + L")\n").c_str());
            OutputDebugStringW((L"[CreateQuestion] Difficulty: " + difficulty + L"\n").c_str());

            try
            {
                auto resultJson = co_await m_client->CreateQuestionValidatedAsync(
                    questionId, teacherId, qText, optA, optB, optC, optD, correctOpt, difficulty, topic);

                OutputDebugStringW((L"[CreateQuestion] Response JSON: " + resultJson + L"\n").c_str());

                auto result = Windows::Data::Json::JsonObject::Parse(resultJson);
                bool success = result.GetNamedBoolean(L"success", false);

                if (success)
                {
                    OutputDebugStringW(L"[CreateQuestion] Success\n");

                    OutputDebugStringW(L"[CreateQuestion] Queueing UI updates...\n");
                    DispatcherQueue().TryEnqueue(Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal, [lifetime]()
                                                 {
                        OutputDebugStringW(L"[CreateQuestion] DispatcherQueue executing - showing message and refreshing\n");
                        lifetime->ShowMessage(L"Question created successfully", InfoBarSeverity::Success);
                        lifetime->LoadQuestions(); });
                }
                else
                {
                    auto errorMsg = result.GetNamedString(L"message", L"Failed to create question");
                    OutputDebugStringW((L"[CreateQuestion] Failed: " + errorMsg + L"\n").c_str());
                    ShowMessage(errorMsg, InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                OutputDebugStringW((L"[CreateQuestion] Exception: " + ex.message() + L"\n").c_str());
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

    winrt::fire_and_forget QuestionBankPage::EditQuestion_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        auto lifetime = get_strong();

        auto btn = sender.as<Button>();
        auto questionId = unbox_value<hstring>(btn.Tag());

        for (uint32_t i = 0; i < lifetime->m_questions.Size(); ++i)
        {
            auto question = lifetime->m_questions.GetAt(i);
            if (question.QuestionId() == questionId)
            {
                lifetime->InputQuestionText().Text(question.QuestionText());
                lifetime->InputOptionA().Text(question.OptionA());
                lifetime->InputOptionB().Text(question.OptionB());
                lifetime->InputOptionC().Text(question.OptionC());
                lifetime->InputOptionD().Text(question.OptionD());
                lifetime->InputTopic().Text(question.Topic());

                auto correctOpt = question.CorrectOption();
                if (correctOpt == L"A")
                    lifetime->ComboCorrectOption().SelectedIndex(0);
                else if (correctOpt == L"B")
                    lifetime->ComboCorrectOption().SelectedIndex(1);
                else if (correctOpt == L"C")
                    lifetime->ComboCorrectOption().SelectedIndex(2);
                else if (correctOpt == L"D")
                    lifetime->ComboCorrectOption().SelectedIndex(3);

                auto difficulty = question.DifficultyLevel();
                if (difficulty == L"Easy")
                    lifetime->ComboDifficulty().SelectedIndex(0);
                else if (difficulty == L"Medium")
                    lifetime->ComboDifficulty().SelectedIndex(1);
                else if (difficulty == L"Hard")
                    lifetime->ComboDifficulty().SelectedIndex(2);

                lifetime->AddQuestionDialog().Title(box_value(L"Edit Question"));
                lifetime->AddQuestionDialog().PrimaryButtonText(L"Update");

                auto result = co_await lifetime->AddQuestionDialog().ShowAsync();

                if (result == ContentDialogResult::Primary)
                {
                    auto qText = lifetime->InputQuestionText().Text();
                    auto optA = lifetime->InputOptionA().Text();
                    auto optB = lifetime->InputOptionB().Text();
                    auto optC = lifetime->InputOptionC().Text();
                    auto optD = lifetime->InputOptionD().Text();
                    auto topic = lifetime->InputTopic().Text();

                    auto trimString = [](hstring const &str) -> hstring
                    {
                        if (str.empty())
                            return str;
                        std::wstring s(str);
                        size_t start = s.find_first_not_of(L" \t\n\r");
                        if (start == std::wstring::npos)
                            return L"";
                        size_t end = s.find_last_not_of(L" \t\n\r");
                        return hstring(s.substr(start, end - start + 1));
                    };

                    qText = trimString(qText);
                    optA = trimString(optA);
                    optB = trimString(optB);
                    optC = trimString(optC);
                    optD = trimString(optD);
                    topic = trimString(topic);

                    if (qText.empty() || optA.empty() || optB.empty() || optC.empty() || optD.empty())
                    {
                        lifetime->ShowMessage(L"Question text and all 4 options are required", InfoBarSeverity::Warning);
                        co_return;
                    }

                    std::set<std::wstring> uniqueOptions = {
                        std::wstring(optA),
                        std::wstring(optB),
                        std::wstring(optC),
                        std::wstring(optD)};

                    if (uniqueOptions.size() != 4)
                    {
                        lifetime->ShowMessage(L"All 4 options must be unique", InfoBarSeverity::Warning);
                        co_return;
                    }

                    auto correctOptIdx = lifetime->ComboCorrectOption().SelectedIndex();
                    hstring correctOpt = correctOptIdx == 0 ? L"A" : correctOptIdx == 1 ? L"B"
                                                                 : correctOptIdx == 2   ? L"C"
                                                                                        : L"D";

                    auto diffIdx = lifetime->ComboDifficulty().SelectedIndex();
                    hstring difficulty = diffIdx == 0 ? L"Easy" : diffIdx == 1 ? L"Medium"
                                                                               : L"Hard";

                    OutputDebugStringW((L"[UpdateQuestion] Starting update for ID: " + questionId + L"\n").c_str());
                    OutputDebugStringW((L"[UpdateQuestion] Difficulty: " + difficulty + L"\n").c_str());

                    try
                    {
                        auto resultJson = co_await lifetime->m_client->UpdateQuestionValidatedAsync(
                            questionId, qText, optA, optB, optC, optD, correctOpt, difficulty, topic);

                        OutputDebugStringW((L"[UpdateQuestion] Response JSON: " + resultJson + L"\n").c_str());

                        auto result = Windows::Data::Json::JsonObject::Parse(resultJson);
                        bool success = result.GetNamedBoolean(L"success", false);

                        if (success)
                        {
                            OutputDebugStringW(L"[UpdateQuestion] Success\n");
                            OutputDebugStringW(L"[UpdateQuestion] Queueing UI updates...\n");

                            lifetime->DispatcherQueue().TryEnqueue([lifetime]()
                                                                   {
                                OutputDebugStringW(L"[UpdateQuestion] DispatcherQueue executing - showing message and refreshing\n");
                                lifetime->ShowMessage(L"Question updated successfully", InfoBarSeverity::Success);
                                lifetime->LoadQuestions(); });
                        }
                        else
                        {
                            auto errorMsg = result.GetNamedString(L"message", L"Failed to update question");
                            OutputDebugStringW((L"[UpdateQuestion] Failed: " + errorMsg + L"\n").c_str());
                            lifetime->ShowMessage(errorMsg, InfoBarSeverity::Error);
                        }
                    }
                    catch (hresult_error const &ex)
                    {
                        OutputDebugStringW((L"[UpdateQuestion] Exception: " + ex.message() + L"\n").c_str());
                        lifetime->ShowMessage(ex.message(), InfoBarSeverity::Error);
                    }
                }

                lifetime->AddQuestionDialog().Title(box_value(L"Add New Question"));
                lifetime->AddQuestionDialog().PrimaryButtonText(L"Add");
                break;
            }
        }
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
            OutputDebugStringW((L"[DeleteQuestion] Starting deletion for ID: " + questionId + L"\n").c_str());

            try
            {
                bool success = co_await m_client->DeleteQuestionSafeAsync(questionId);
                OutputDebugStringW((L"[DeleteQuestion] Result: " + std::wstring(success ? L"SUCCESS" : L"FAILED") + L"\n").c_str());

                if (success)
                {
                    OutputDebugStringW(L"[DeleteQuestion] Queueing UI updates...\n");
                    DispatcherQueue().TryEnqueue(Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal, [lifetime]()
                                                 {
                        OutputDebugStringW(L"[DeleteQuestion] DispatcherQueue executing - showing message and refreshing\n");
                        lifetime->ShowMessage(L"Question deleted successfully", InfoBarSeverity::Success);
                        lifetime->LoadQuestions(); });
                }
                else
                {
                    ShowMessage(L"Failed to delete question", InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                OutputDebugStringW((L"[DeleteQuestion] Exception: " + ex.message() + L"\n").c_str());
                ShowMessage(ex.message(), InfoBarSeverity::Error);
            }
        }
    }

    void QuestionBankPage::ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), message, severity);
    }
}

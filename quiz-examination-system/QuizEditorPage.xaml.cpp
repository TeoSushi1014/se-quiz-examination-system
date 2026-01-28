#include "pch.h"
#include "QuizEditorPage.xaml.h"
#if __has_include("QuizEditorPage.g.cpp")
#include "QuizEditorPage.g.cpp"
#endif
#include "QuestionItem.h"
#include "SupabaseClientManager.h"
#include "PageHelper.h"
#include "HttpHelper.h"
#include "SupabaseConfig.h"
#include <winrt/Windows.System.Threading.h>
#include <algorithm>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Windows::Data::Json;
using namespace Windows::Web::Http;

namespace winrt::quiz_examination_system::implementation
{
    QuizEditorPage::QuizEditorPage()
    {
        InitializeComponent();
        m_availableQuestions = single_threaded_observable_vector<quiz_examination_system::QuestionItem>();
        m_selectedQuestions = single_threaded_observable_vector<quiz_examination_system::QuestionItem>();
        AvailableQuestionsListView().ItemsSource(m_availableQuestions);
        SelectedQuestionsListView().ItemsSource(m_selectedQuestions);
    }

    void QuizEditorPage::Page_Loaded(IInspectable const &, RoutedEventArgs const &)
    {
        LoadQuestions();
    }

    fire_and_forget QuizEditorPage::LoadQuestions()
    {
        auto lifetime = get_strong();

        OutputDebugStringW(L"[LoadQuestions] Starting to load questions for quiz editor\n");
        LoadingRing().IsActive(true);

        try
        {
            auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
            hstring teacherId = manager.GetUserId();
            hstring userRole = manager.GetRole();

            if (teacherId.empty())
            {
                OutputDebugStringW(L"[LoadQuestions] Error: User not logged in\n");
                ShowMessage(L"User not logged in", InfoBarSeverity::Error);
                LoadingRing().IsActive(false);
                co_return;
            }

            OutputDebugStringW((L"[LoadQuestions] User ID: " + teacherId + L", Role: " + userRole + L"\n").c_str());

            // Admin can see all questions, Teacher only sees their own
            hstring queryParams;
            if (userRole == L"Admin")
            {
                OutputDebugStringW(L"[LoadQuestions] Admin user - Loading ALL questions\n");
                queryParams = L"questions?select=*&order=created_at.desc";
            }
            else
            {
                OutputDebugStringW(L"[LoadQuestions] Teacher user - Loading own questions\n");
                queryParams = L"questions?select=*&created_by=eq." + teacherId + L"&order=created_at.desc";
            }

            hstring endpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(queryParams);
            hstring content = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(endpoint, L"", HttpMethod::Get());

            OutputDebugStringW((L"[LoadQuestions] Response: " + std::wstring(content).substr(0, 200) + L"...\n").c_str());

            auto questionsArray = JsonArray::Parse(content);

            OutputDebugStringW((L"[LoadQuestions] Parsed " + std::to_wstring(questionsArray.Size()) + L" questions\n").c_str());

            m_allQuestions.clear();
            m_availableQuestions.Clear();

            for (uint32_t i = 0; i < questionsArray.Size(); ++i)
            {
                auto qObj = questionsArray.GetObjectAt(i);

                auto item = make<QuestionItem>();
                item.QuestionId(qObj.GetNamedString(L"id", L""));
                item.QuestionText(qObj.GetNamedString(L"question_text", L""));
                item.DifficultyLevel(qObj.GetNamedString(L"difficulty_level", L"Medium"));
                item.Topic(qObj.GetNamedString(L"topic", L"General"));
                item.OptionA(qObj.GetNamedString(L"option_a", L""));
                item.OptionB(qObj.GetNamedString(L"option_b", L""));
                item.OptionC(qObj.GetNamedString(L"option_c", L""));
                item.OptionD(qObj.GetNamedString(L"option_d", L""));
                item.CorrectOption(qObj.GetNamedString(L"correct_option", L"A"));

                m_allQuestions.push_back(item);
                m_availableQuestions.Append(item);
            }

            OutputDebugStringW((L"[LoadQuestions] Success - Added " + std::to_wstring(m_availableQuestions.Size()) + L" questions to UI\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[LoadQuestions] Error: " + ex.message() + L"\n").c_str());
            ShowMessage(L"Failed to load questions: " + ex.message(), InfoBarSeverity::Error);
        }

        LoadingRing().IsActive(false);
    }

    void QuizEditorPage::OnSearchTextChanged(AutoSuggestBox const &sender, AutoSuggestBoxTextChangedEventArgs const &)
    {
        FilterQuestions(sender.Text());
    }

    void QuizEditorPage::FilterQuestions(hstring const &searchText)
    {
        m_availableQuestions.Clear();

        auto lowerSearch = to_string(searchText);
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

        for (auto const &question : m_allQuestions)
        {
            if (searchText.empty())
            {
                m_availableQuestions.Append(question);
            }
            else
            {
                auto text = to_string(question.QuestionText());
                std::transform(text.begin(), text.end(), text.begin(), ::tolower);

                if (text.find(lowerSearch) != std::string::npos)
                {
                    m_availableQuestions.Append(question);
                }
            }
        }
    }

    void QuizEditorPage::OnQuestionChecked(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto checkbox = sender.as<CheckBox>();
        auto questionId = unbox_value<hstring>(checkbox.Tag());

        for (auto const &question : m_allQuestions)
        {
            if (question.QuestionId() == questionId)
            {
                bool alreadySelected = false;
                for (uint32_t i = 0; i < m_selectedQuestions.Size(); ++i)
                {
                    if (m_selectedQuestions.GetAt(i).QuestionId() == questionId)
                    {
                        alreadySelected = true;
                        break;
                    }
                }

                if (!alreadySelected)
                {
                    m_selectedQuestions.Append(question);
                    UpdateSelectedCount();
                }
                break;
            }
        }
    }

    void QuizEditorPage::OnQuestionUnchecked(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto checkbox = sender.as<CheckBox>();
        auto questionId = unbox_value<hstring>(checkbox.Tag());

        for (uint32_t i = 0; i < m_selectedQuestions.Size(); ++i)
        {
            if (m_selectedQuestions.GetAt(i).QuestionId() == questionId)
            {
                m_selectedQuestions.RemoveAt(i);
                UpdateSelectedCount();
                break;
            }
        }
    }

    void QuizEditorPage::OnRemoveQuestionClicked(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto button = sender.as<Button>();
        auto questionId = unbox_value<hstring>(button.Tag());

        for (uint32_t i = 0; i < m_selectedQuestions.Size(); ++i)
        {
            if (m_selectedQuestions.GetAt(i).QuestionId() == questionId)
            {
                m_selectedQuestions.RemoveAt(i);
                UpdateSelectedCount();
                break;
            }
        }
    }

    void QuizEditorPage::UpdateSelectedCount()
    {
        SelectedCountText().Text(to_hstring(m_selectedQuestions.Size()));
    }

    void QuizEditorPage::OnSaveClicked(IInspectable const &, RoutedEventArgs const &)
    {
        SaveQuiz();
    }

    fire_and_forget QuizEditorPage::SaveQuiz()
    {
        auto lifetime = get_strong();

        auto title = TitleTextBox().Text();
        if (title.empty())
        {
            ShowMessage(L"Please enter a quiz title", InfoBarSeverity::Warning);
            co_return;
        }

        if (m_selectedQuestions.Size() == 0)
        {
            ShowMessage(L"Please select at least one question", InfoBarSeverity::Warning);
            co_return;
        }

        SaveButton().IsEnabled(false);

        try
        {
            auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
            hstring createdBy = manager.GetUserId();

            auto timeLimit = static_cast<int32_t>(TimeLimitNumberBox().Value());
            auto totalPoints = static_cast<int32_t>(TotalPointsNumberBox().Value());
            auto shuffleQuestions = ShuffleQuestionsToggle().IsOn();
            auto shuffleAnswers = ShuffleAnswersToggle().IsOn();

            hstring resultVisibility = L"immediate";
            switch (ResultVisibilityComboBox().SelectedIndex())
            {
            case 0:
                resultVisibility = L"immediate";
                break;
            case 1:
                resultVisibility = L"after_quiz_end";
                break;
            case 2:
                resultVisibility = L"manual_release";
                break;
            }

            hstring maxAttempts = L"unlimited";
            switch (MaxAttemptsComboBox().SelectedIndex())
            {
            case 0:
                maxAttempts = L"unlimited";
                break;
            case 1:
                maxAttempts = L"1";
                break;
            case 2:
                maxAttempts = L"2";
                break;
            case 3:
                maxAttempts = L"3";
                break;
            }

            JsonArray questionsArray;
            int32_t orderNum = 1;
            int32_t pointsPerQuestion = totalPoints / static_cast<int32_t>(m_selectedQuestions.Size());

            for (uint32_t i = 0; i < m_selectedQuestions.Size(); ++i)
            {
                JsonObject qObj;
                qObj.SetNamedValue(L"question_id", JsonValue::CreateStringValue(m_selectedQuestions.GetAt(i).QuestionId()));
                qObj.SetNamedValue(L"points", JsonValue::CreateNumberValue(pointsPerQuestion));
                qObj.SetNamedValue(L"order_num", JsonValue::CreateNumberValue(orderNum++));
                questionsArray.Append(qObj);
            }

            JsonObject payload;
            payload.SetNamedValue(L"p_title", JsonValue::CreateStringValue(title));
            payload.SetNamedValue(L"p_created_by", JsonValue::CreateStringValue(createdBy));
            payload.SetNamedValue(L"p_time_limit_minutes", JsonValue::CreateNumberValue(timeLimit));
            payload.SetNamedValue(L"p_total_points", JsonValue::CreateNumberValue(totalPoints));
            payload.SetNamedValue(L"p_shuffle_questions", JsonValue::CreateBooleanValue(shuffleQuestions));
            payload.SetNamedValue(L"p_shuffle_answers", JsonValue::CreateBooleanValue(shuffleAnswers));
            payload.SetNamedValue(L"p_result_visibility", JsonValue::CreateStringValue(resultVisibility));
            payload.SetNamedValue(L"p_max_attempts", JsonValue::CreateStringValue(maxAttempts));
            payload.SetNamedValue(L"p_questions", questionsArray);

            hstring jsonBody = payload.Stringify();
            hstring endpoint = ::quiz_examination_system::SupabaseConfig::GetRpcEndpoint(L"create_quiz_full");
            hstring response = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(endpoint, jsonBody);

            auto resultObj = JsonObject::Parse(response);
            bool success = resultObj.GetNamedBoolean(L"success", false);

            DispatcherQueue().TryEnqueue(Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal, [lifetime, success, resultObj]()
                                         {
                if (success)
                {
                    lifetime->ShowMessage(L"Quiz created successfully", InfoBarSeverity::Success);
                    lifetime->SaveButton().IsEnabled(true);
                    
                    auto timer = Windows::System::Threading::ThreadPoolTimer::CreateTimer([lifetime](Windows::System::Threading::ThreadPoolTimer const&)
                    {
                        lifetime->DispatcherQueue().TryEnqueue([lifetime]()
                        {
                            lifetime->Frame().GoBack();
                        });
                    }, std::chrono::seconds(1));
                }
                else
                {
                    hstring error = resultObj.GetNamedString(L"error", L"Unknown error");
                    lifetime->ShowMessage(L"Failed to create quiz: " + error, InfoBarSeverity::Error);
                    lifetime->SaveButton().IsEnabled(true);
                } });
        }
        catch (hresult_error const &ex)
        {
            DispatcherQueue().TryEnqueue(Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal, [lifetime, ex]()
                                         {
                lifetime->ShowMessage(L"Error: " + ex.message(), InfoBarSeverity::Error);
                lifetime->SaveButton().IsEnabled(true); });
        }
    }

    void QuizEditorPage::OnCancelClicked(IInspectable const &, RoutedEventArgs const &)
    {
        Frame().GoBack();
    }

    void QuizEditorPage::ShowMessage(hstring const &message, InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), L"", message, severity);
    }
}

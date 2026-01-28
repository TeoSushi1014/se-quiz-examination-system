#include "pch.h"
#include "QuizManagementPage.xaml.h"
#if __has_include("QuizManagementPage.g.cpp")
#include "QuizManagementPage.g.cpp"
#endif
#include "PageHelper.h"
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Storage.Streams.h>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Web::Http;

namespace winrt::quiz_examination_system::implementation
{
    QuizManagementPage::QuizManagementPage()
    {
        InitializeComponent();
        m_quizzes = single_threaded_observable_vector<quiz_examination_system::QuizItem>();
        m_availableQuestions = single_threaded_observable_vector<quiz_examination_system::QuestionItem>();
        m_client = std::make_unique<::quiz_examination_system::SupabaseClientAsync>();
    }

    void QuizManagementPage::Page_Loaded(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        LoadQuizzes();
        LoadAvailableQuestions();
    }

    winrt::fire_and_forget QuizManagementPage::LoadQuizzes()
    {
        auto lifetime = get_strong();

        OutputDebugStringW(L"[LoadQuizzes] Starting quiz refresh\n");
        LoadingRing().IsActive(true);

        auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
        auto teacherIdRaw = manager.GetUserId();
        std::wstring teacherIdStr(teacherIdRaw);
        teacherIdStr.erase(teacherIdStr.find_last_not_of(L" \t\n\r") + 1);
        hstring teacherId = hstring(teacherIdStr);

        try
        {
            OutputDebugStringW((L"[LoadQuizzes] Fetching quizzes for teacherId: '" + std::wstring(teacherId) + L"'\n").c_str());
            auto quizzesJson = co_await m_client->GetQuizzesJsonAsync(teacherId);
            OutputDebugStringW(L"[LoadQuizzes] Received JSON\n");

            m_quizzes.Clear();
            auto quizzesArray = Windows::Data::Json::JsonArray::Parse(quizzesJson);
            OutputDebugStringW((L"[LoadQuizzes] Parsed " + std::to_wstring(quizzesArray.Size()) + L" quizzes\n").c_str());

            for (uint32_t i = 0; i < quizzesArray.Size(); ++i)
            {
                auto qObj = quizzesArray.GetObjectAt(i);

                auto item = make<QuizItem>();
                item.QuizId(qObj.GetNamedString(L"id", L""));
                item.Title(qObj.GetNamedString(L"title", L""));
                item.TimeLimitMinutes(static_cast<int32_t>(qObj.GetNamedNumber(L"time_limit_minutes", 0)));
                item.TotalPoints(static_cast<int32_t>(qObj.GetNamedNumber(L"total_points", 0)));
                item.QuestionCount(static_cast<int32_t>(qObj.GetNamedNumber(L"question_count", 0)));
                item.MaxAttempts(qObj.GetNamedString(L"max_attempts", L"unlimited"));
                item.ResultVisibility(qObj.GetNamedString(L"result_visibility", L"immediate"));
                item.ShuffleQuestions(qObj.GetNamedBoolean(L"shuffle_questions", false));
                item.ShuffleAnswers(qObj.GetNamedBoolean(L"shuffle_answers", false));
                item.CreatedAt(qObj.GetNamedString(L"created_at", L""));

                m_quizzes.Append(item);
            }

            QuizListView().ItemsSource(m_quizzes);
            OutputDebugStringW((L"[LoadQuizzes] SUCCESS - Added " + std::to_wstring(m_quizzes.Size()) + L" quizzes to UI\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[LoadQuizzes] ERROR: " + std::wstring(ex.message()) + L"\n").c_str());
            ShowMessage(ex.message(), InfoBarSeverity::Error);
        }
        catch (...)
        {
            OutputDebugStringW(L"[LoadQuizzes] Unknown error\n");
        }

        LoadingRing().IsActive(false);
    }

    winrt::fire_and_forget QuizManagementPage::LoadAvailableQuestions()
    {
        auto lifetime = get_strong();

        auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
        auto teacherIdRaw = manager.GetUserId();
        std::wstring teacherIdStr(teacherIdRaw);
        teacherIdStr.erase(teacherIdStr.find_last_not_of(L" \t\n\r") + 1);
        hstring teacherId = hstring(teacherIdStr);

        try
        {
            auto questionsJson = co_await m_client->GetQuestionsJsonAsync(teacherId);
            m_availableQuestions.Clear();
            m_allQuestions.clear();

            auto questionsArray = Windows::Data::Json::JsonArray::Parse(questionsJson);

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

            OutputDebugStringW((L"[LoadAvailableQuestions] Loaded " + std::to_wstring(m_availableQuestions.Size()) + L" questions\n").c_str());
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[LoadAvailableQuestions] ERROR: " + std::wstring(ex.message()) + L"\n").c_str());
        }
    }

    void QuizManagementPage::CreateQuiz_Click(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        ShowQuizEditorDialog();
    }

    winrt::fire_and_forget QuizManagementPage::ShowQuizEditorDialog()
    {
        auto lifetime = get_strong();

        InputTitle().Text(L"");
        InputTimeLimit().Value(60);
        InputTotalPoints().Value(100);
        InputMaxAttempts().Value(std::numeric_limits<double>::quiet_NaN());
        ComboResultVisibility().SelectedIndex(0);
        CheckShuffleQuestions().IsChecked(false);
        CheckShuffleAnswers().IsChecked(false);
        QuestionSelectionList().SelectedItems().Clear();

        auto result = co_await QuizEditorDialog().ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            auto title = InputTitle().Text();
            auto timeLimit = static_cast<int32_t>(InputTimeLimit().Value());
            auto totalPoints = static_cast<int32_t>(InputTotalPoints().Value());
            auto maxAttemptsValue = InputMaxAttempts().Value();
            hstring maxAttempts = std::isnan(maxAttemptsValue) ? L"Unlimited" : to_hstring(static_cast<int32_t>(maxAttemptsValue));
            auto shuffleQuestions = CheckShuffleQuestions().IsChecked().GetBoolean();
            auto shuffleAnswers = CheckShuffleAnswers().IsChecked().GetBoolean();

            auto resultVisIdx = ComboResultVisibility().SelectedIndex();
            hstring resultVisibility = resultVisIdx == 0 ? L"Immediate" : resultVisIdx == 1 ? L"After quiz end"
                                                                                            : L"Manual release";

            if (title.empty())
            {
                ShowMessage(L"Quiz title is required", InfoBarSeverity::Warning);
                co_return;
            }

            if (timeLimit <= 0)
            {
                ShowMessage(L"Time limit must be greater than 0", InfoBarSeverity::Warning);
                co_return;
            }

            if (totalPoints <= 0)
            {
                ShowMessage(L"Total points must be greater than 0", InfoBarSeverity::Warning);
                co_return;
            }

            auto selectedQuestions = QuestionSelectionList().SelectedItems();
            if (selectedQuestions.Size() == 0)
            {
                ShowMessage(L"Please select at least one question", InfoBarSeverity::Warning);
                co_return;
            }

            auto &manager = ::quiz_examination_system::SupabaseClientManager::GetInstance();
            auto teacherIdRaw = manager.GetUserId();
            std::wstring teacherIdStr(teacherIdRaw);
            teacherIdStr.erase(teacherIdStr.find_last_not_of(L" \t\n\r") + 1);
            hstring teacherId = hstring(teacherIdStr);

            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            auto idNum = (timestamp % 900000) + 100000;
            hstring quizId = to_hstring(idNum);

            OutputDebugStringW((L"[CreateQuiz] Starting creation with ID: " + std::wstring(quizId) + L"\n").c_str());

            try
            {
                Windows::Data::Json::JsonArray questionsArray;
                int pointsPerQuestion = totalPoints / static_cast<int>(selectedQuestions.Size());

                for (uint32_t i = 0; i < selectedQuestions.Size(); ++i)
                {
                    auto question = selectedQuestions.GetAt(i).as<quiz_examination_system::QuestionItem>();

                    JsonObject questionObj;
                    questionObj.Insert(L"question_id", JsonValue::CreateStringValue(question.QuestionId()));
                    questionObj.Insert(L"points", JsonValue::CreateNumberValue(pointsPerQuestion));
                    questionObj.Insert(L"order_num", JsonValue::CreateNumberValue(static_cast<double>(i + 1)));

                    questionsArray.Append(questionObj);
                }

                JsonObject rpcParams;
                rpcParams.Insert(L"p_title", JsonValue::CreateStringValue(title));
                rpcParams.Insert(L"p_created_by", JsonValue::CreateStringValue(teacherId));
                rpcParams.Insert(L"p_time_limit_minutes", JsonValue::CreateNumberValue(timeLimit));
                rpcParams.Insert(L"p_total_points", JsonValue::CreateNumberValue(totalPoints));
                rpcParams.Insert(L"p_shuffle_questions", JsonValue::CreateBooleanValue(shuffleQuestions));
                rpcParams.Insert(L"p_shuffle_answers", JsonValue::CreateBooleanValue(shuffleAnswers));
                rpcParams.Insert(L"p_result_visibility", JsonValue::CreateStringValue(resultVisibility));
                rpcParams.Insert(L"p_max_attempts", JsonValue::CreateStringValue(maxAttempts));
                rpcParams.Insert(L"p_questions", questionsArray);

                Uri uri(L"https://tuciofxdzzrzwzqsltps.supabase.co/rest/v1/rpc/create_quiz_full");
                HttpRequestMessage request(HttpMethod::Post(), uri);
                request.Headers().Insert(L"apikey", L"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");
                request.Headers().Insert(L"Authorization", L"Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InR1Y2lvZnhkenpyend6cXNsdHBzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njg3NTY5ODAsImV4cCI6MjA4NDMzMjk4MH0.2b1FYJ1GxNm_Jwg6TkP0Lf7ZOuvkVctc_96EV_uzVnI");
                request.Content(HttpStringContent(rpcParams.Stringify(), Windows::Storage::Streams::UnicodeEncoding::Utf8, L"application/json"));

                HttpClient httpClient;
                auto response = co_await httpClient.SendRequestAsync(request);
                auto resultJson = co_await response.Content().ReadAsStringAsync();

                OutputDebugStringW((L"[CreateQuiz] Response JSON: " + std::wstring(resultJson) + L"\n").c_str());

                auto resultObj = Windows::Data::Json::JsonObject::Parse(resultJson);
                bool success = resultObj.GetNamedBoolean(L"success", false);

                if (success)
                {
                    OutputDebugStringW(L"[CreateQuiz] Success\n");
                    DispatcherQueue().TryEnqueue([lifetime]()
                                                 {
                        lifetime->ShowMessage(L"Quiz created successfully", InfoBarSeverity::Success);
                        lifetime->LoadQuizzes(); });
                }
                else
                {
                    auto errorMsg = resultObj.GetNamedString(L"message", L"Failed to create quiz");
                    OutputDebugStringW((L"[CreateQuiz] Failed: " + std::wstring(errorMsg) + L"\n").c_str());
                    ShowMessage(errorMsg, InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                OutputDebugStringW((L"[CreateQuiz] Exception: " + std::wstring(ex.message()) + L"\n").c_str());
                ShowMessage(ex.message(), InfoBarSeverity::Error);
            }
        }
    }

    void QuizManagementPage::RefreshQuizzes_Click(Windows::Foundation::IInspectable const &, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        LoadQuizzes();
    }

    winrt::fire_and_forget QuizManagementPage::EditQuiz_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        auto lifetime = get_strong();

        auto btn = sender.as<Button>();
        auto quizId = unbox_value<hstring>(btn.Tag());

        for (uint32_t i = 0; i < m_quizzes.Size(); ++i)
        {
            auto quiz = m_quizzes.GetAt(i);
            if (quiz.QuizId() == quizId)
            {
                InputTitle().Text(quiz.Title());
                InputTimeLimit().Value(quiz.TimeLimitMinutes());
                InputTotalPoints().Value(quiz.TotalPoints());
                InputMaxAttempts().Text(quiz.MaxAttempts());
                CheckShuffleQuestions().IsChecked(quiz.ShuffleQuestions());
                CheckShuffleAnswers().IsChecked(quiz.ShuffleAnswers());

                auto visibility = quiz.ResultVisibility();
                if (visibility == L"Immediate")
                    ComboResultVisibility().SelectedIndex(0);
                else if (visibility == L"After quiz end")
                    ComboResultVisibility().SelectedIndex(1);
                else
                    ComboResultVisibility().SelectedIndex(2);

                QuizEditorDialog().Title(box_value(L"Edit quiz"));
                QuizEditorDialog().PrimaryButtonText(L"Update");

                auto dialogResult = co_await QuizEditorDialog().ShowAsync();

                if (dialogResult == ContentDialogResult::Primary)
                {
                    auto title = InputTitle().Text();
                    auto timeLimit = static_cast<int32_t>(InputTimeLimit().Value());
                    auto totalPoints = static_cast<int32_t>(InputTotalPoints().Value());
                    auto maxAttempts = InputMaxAttempts().Text();
                    auto shuffleQuestions = CheckShuffleQuestions().IsChecked().GetBoolean();
                    auto shuffleAnswers = CheckShuffleAnswers().IsChecked().GetBoolean();

                    auto resultVisIdx = ComboResultVisibility().SelectedIndex();
                    hstring resultVisibility = resultVisIdx == 0 ? L"Immediate" : resultVisIdx == 1 ? L"After quiz end"
                                                                                                    : L"Manual release";

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

                    title = trimString(title);
                    maxAttempts = trimString(maxAttempts);

                    // Ensure maxAttempts uses correct case-sensitive value
                    if (maxAttempts.empty() || maxAttempts == L"unlimited" || maxAttempts == L"Unlimited")
                    {
                        maxAttempts = L"Unlimited";
                    }

                    if (title.empty())
                    {
                        ShowMessage(L"Quiz title is required", InfoBarSeverity::Warning);
                        co_return;
                    }

                    if (timeLimit <= 0 || totalPoints <= 0)
                    {
                        ShowMessage(L"Time limit and points must be greater than 0", InfoBarSeverity::Warning);
                        co_return;
                    }

                    OutputDebugStringW((L"[UpdateQuiz] Starting update for ID: " + std::wstring(quizId) + L"\n").c_str());

                    try
                    {
                        auto resultJson = co_await m_client->UpdateQuizAsync(
                            quizId, title, timeLimit, totalPoints,
                            maxAttempts, resultVisibility, shuffleQuestions, shuffleAnswers);

                        auto resultObj = Windows::Data::Json::JsonObject::Parse(resultJson);
                        bool success = resultObj.GetNamedBoolean(L"success", false);

                        if (success)
                        {
                            DispatcherQueue().TryEnqueue([lifetime]()
                                                         {
                                lifetime->ShowMessage(L"Quiz updated successfully", InfoBarSeverity::Success);
                                lifetime->LoadQuizzes(); });
                        }
                        else
                        {
                            auto errorMsg = resultObj.GetNamedString(L"message", L"Failed to update quiz");
                            ShowMessage(errorMsg, InfoBarSeverity::Error);
                        }
                    }
                    catch (hresult_error const &ex)
                    {
                        OutputDebugStringW((L"[UpdateQuiz] Exception: " + std::wstring(ex.message()) + L"\n").c_str());
                        ShowMessage(ex.message(), InfoBarSeverity::Error);
                    }
                }

                QuizEditorDialog().Title(box_value(L"Create quiz"));
                QuizEditorDialog().PrimaryButtonText(L"Save");
                break;
            }
        }
    }

    winrt::fire_and_forget QuizManagementPage::DeleteQuiz_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &)
    {
        auto lifetime = get_strong();

        auto btn = sender.as<Button>();
        auto quizId = unbox_value<hstring>(btn.Tag());

        ContentDialog confirmDialog;
        confirmDialog.XamlRoot(this->XamlRoot());
        confirmDialog.Title(box_value(L"Confirm deletion"));
        confirmDialog.Content(box_value(hstring(L"Are you sure you want to delete this quiz? This action cannot be undone if students have already taken this quiz.")));
        confirmDialog.PrimaryButtonText(L"Delete");
        confirmDialog.CloseButtonText(L"Cancel");
        confirmDialog.DefaultButton(ContentDialogButton::Close);

        auto result = co_await confirmDialog.ShowAsync();

        if (result == ContentDialogResult::Primary)
        {
            OutputDebugStringW((L"[DeleteQuiz] Starting deletion for ID: " + std::wstring(quizId) + L"\n").c_str());

            try
            {
                auto resultJson = co_await m_client->DeleteQuizSafeAsync(quizId);
                auto resultObj = Windows::Data::Json::JsonObject::Parse(resultJson);
                bool success = resultObj.GetNamedBoolean(L"success", false);

                if (success)
                {
                    DispatcherQueue().TryEnqueue([lifetime]()
                                                 {
                        lifetime->ShowMessage(L"Quiz deleted successfully", InfoBarSeverity::Success);
                        lifetime->LoadQuizzes(); });
                }
                else
                {
                    auto errorMsg = resultObj.GetNamedString(L"message", L"Failed to delete quiz");
                    ShowMessage(errorMsg, InfoBarSeverity::Error);
                }
            }
            catch (hresult_error const &ex)
            {
                OutputDebugStringW((L"[DeleteQuiz] Exception: " + std::wstring(ex.message()) + L"\n").c_str());
                ShowMessage(ex.message(), InfoBarSeverity::Error);
            }
        }
    }

    void QuizManagementPage::ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        ::quiz_examination_system::PageHelper::ShowInfoBar(ActionMessage(), message, severity);
    }

    void QuizManagementPage::QuestionSearchBox_TextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const &sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const &)
    {
        auto searchText = sender.Text();
        m_availableQuestions.Clear();

        if (searchText.empty())
        {
            for (auto const &q : m_allQuestions)
            {
                m_availableQuestions.Append(q);
            }
        }
        else
        {
            std::wstring searchLower(searchText);
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::towlower);

            for (auto const &q : m_allQuestions)
            {
                std::wstring questionText(q.QuestionText());
                std::wstring topic(q.Topic());
                std::wstring difficulty(q.DifficultyLevel());

                std::transform(questionText.begin(), questionText.end(), questionText.begin(), ::towlower);
                std::transform(topic.begin(), topic.end(), topic.begin(), ::towlower);
                std::transform(difficulty.begin(), difficulty.end(), difficulty.begin(), ::towlower);

                if (questionText.find(searchLower) != std::wstring::npos ||
                    topic.find(searchLower) != std::wstring::npos ||
                    difficulty.find(searchLower) != std::wstring::npos)
                {
                    m_availableQuestions.Append(q);
                }
            }
        }
    }
}

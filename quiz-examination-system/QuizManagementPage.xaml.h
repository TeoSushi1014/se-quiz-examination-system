#pragma once

#include "QuizManagementPage.g.h"
#include "QuizItem.h"
#include "QuestionItem.h"
#include "SupabaseClientAsync.h"
#include "SupabaseClientManager.h"
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::quiz_examination_system::implementation
{
    // Simple struct for student selection
    struct StudentInfo
    {
        hstring Id;
        hstring Username;
    };

    struct QuizManagementPage : QuizManagementPageT<QuizManagementPage>
    {
        QuizManagementPage();

        void Page_Loaded(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void CreateQuiz_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void RefreshQuizzes_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget EditQuiz_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget DeleteQuiz_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget AssignQuiz_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void SelectAllStudents_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void ClearAllStudents_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void QuestionSearchBox_TextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const &sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const &args);

        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuestionItem> AvailableQuestions() { return m_availableQuestions; }

    private:
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuizItem> m_quizzes;
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuestionItem> m_availableQuestions;
        std::vector<quiz_examination_system::QuestionItem> m_allQuestions;
        std::vector<StudentInfo> m_students;
        hstring m_currentAssignQuizId;
        std::unique_ptr<::quiz_examination_system::SupabaseClientAsync> m_client;

        winrt::fire_and_forget LoadQuizzes();
        winrt::fire_and_forget LoadAvailableQuestions();
        Windows::Foundation::IAsyncAction LoadStudentsAsync();
        winrt::fire_and_forget ShowQuizEditorDialog();
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct QuizManagementPage : QuizManagementPageT<QuizManagementPage, implementation::QuizManagementPage>
    {
    };
}

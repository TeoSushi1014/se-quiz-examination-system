#pragma once

#include "QuestionBankPage.g.h"
#include "QuestionItem.h"
#include "SupabaseClientAsync.h"
#include "SupabaseClientManager.h"
#include <winrt/Windows.Foundation.Collections.h>
#include <set>

namespace winrt::quiz_examination_system::implementation
{
    struct QuestionBankPage : QuestionBankPageT<QuestionBankPage>
    {
        QuestionBankPage();

        void Page_Loaded(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void AddQuestion_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void ImportQuestions_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        void RefreshQuestions_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget EditQuestion_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);
        winrt::fire_and_forget DeleteQuestion_Click(Windows::Foundation::IInspectable const &sender, Microsoft::UI::Xaml::RoutedEventArgs const &e);

    private:
        Windows::Foundation::Collections::IObservableVector<quiz_examination_system::QuestionItem> m_questions;
        std::unique_ptr<::quiz_examination_system::SupabaseClientAsync> m_client;

        winrt::fire_and_forget LoadQuestions();
        winrt::fire_and_forget ShowAddQuestionDialog();
        void ShowMessage(hstring const &message, Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::quiz_examination_system::factory_implementation
{
    struct QuestionBankPage : QuestionBankPageT<QuestionBankPage, implementation::QuestionBankPage>
    {
    };
}

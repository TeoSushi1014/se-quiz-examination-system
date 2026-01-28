#pragma once

#include <winrt/base.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <functional>

namespace quiz_examination_system
{
    class PageHelper
    {
    public:
        static void ShowInfoBar(
            winrt::Microsoft::UI::Xaml::Controls::InfoBar const &infoBar,
            winrt::hstring const &message,
            winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
        {
            infoBar.Message(message);
            infoBar.Severity(severity);
            infoBar.IsOpen(true);
        }

        static void ShowInfoBar(
            winrt::Microsoft::UI::Xaml::Controls::InfoBar const &infoBar,
            winrt::hstring const &title,
            winrt::hstring const &message,
            winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
        {
            infoBar.Title(title);
            infoBar.Message(message);
            infoBar.Severity(severity);
            infoBar.IsOpen(true);
        }

        static winrt::Windows::Foundation::IAsyncOperation<bool> ShowConfirmDialog(
            winrt::Microsoft::UI::Xaml::XamlRoot const &xamlRoot,
            winrt::hstring const &title,
            winrt::hstring const &content,
            winrt::hstring const &primaryButtonText = L"Confirm",
            winrt::hstring const &closeButtonText = L"Cancel")
        {
            using namespace winrt::Microsoft::UI::Xaml::Controls;

            ContentDialog dialog;
            dialog.XamlRoot(xamlRoot);
            dialog.Title(winrt::box_value(title));
            dialog.Content(winrt::box_value(content));
            dialog.PrimaryButtonText(primaryButtonText);
            dialog.CloseButtonText(closeButtonText);
            dialog.DefaultButton(ContentDialogButton::Close);

            auto result = co_await dialog.ShowAsync();
            co_return result == ContentDialogResult::Primary;
        }

        template <typename TPage>
        static winrt::fire_and_forget ExecuteWithLoading(
            TPage *page,
            winrt::Microsoft::UI::Xaml::Controls::ProgressRing const &loadingRing,
            winrt::Microsoft::UI::Xaml::Controls::InfoBar const &infoBar,
            std::function<winrt::Windows::Foundation::IAsyncAction()> action)
        {
            auto lifetime = page->get_strong();

            loadingRing.IsActive(true);

            try
            {
                co_await action();
            }
            catch (winrt::hresult_error const &ex)
            {
                ShowInfoBar(infoBar, ex.message(), winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            }

            loadingRing.IsActive(false);
        }
    };

    class ValidationHelper
    {
    public:
        static constexpr int MIN_PASSWORD_LENGTH = 8;

        static bool ValidatePassword(winrt::hstring const &password, winrt::hstring &errorMessage)
        {
            if (password.empty())
            {
                errorMessage = L"Password is required";
                return false;
            }

            if (password.size() < MIN_PASSWORD_LENGTH)
            {
                errorMessage = L"Password must be at least 8 characters";
                return false;
            }

            return true;
        }

        static bool ValidateUsername(winrt::hstring const &username, winrt::hstring &errorMessage)
        {
            if (username.empty())
            {
                errorMessage = L"Username is required";
                return false;
            }

            return true;
        }
    };
}

#include "pch.h"
#include "SystemLogsPage.xaml.h"
#if __has_include("SystemLogsPage.g.cpp")
#include "SystemLogsPage.g.cpp"
#endif
#include "LogItem.h"
#include "HttpHelper.h"
#include "SupabaseConfig.h"
#include "PageHelper.h"
#include <algorithm>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Windows::Data::Json;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Headers;

namespace winrt::quiz_examination_system::implementation
{
    SystemLogsPage::SystemLogsPage()
        : m_logs(single_threaded_observable_vector<quiz_examination_system::LogItem>())
    {
        InitializeComponent();
        LoadLogs();
    }

    winrt::fire_and_forget SystemLogsPage::LoadLogs()
    {
        auto lifetime = get_strong();

        OutputDebugStringW(L"[SystemLogsPage] Loading audit logs\n");

        if (LoadingRing())
        {
            LoadingRing().IsActive(true);
        }

        if (EmptyStateText())
        {
            EmptyStateText().Visibility(Visibility::Collapsed);
        }

        try
        {
            hstring query = L"select=id,action,actor_id,target_table,target_id,details,created_at,users!audit_logs_actor_id_fkey(username)&order=created_at.desc&limit=200";
            hstring endpoint = ::quiz_examination_system::SupabaseConfig::GetRestEndpoint(L"audit_logs?" + query);

            OutputDebugStringW((L"[SystemLogsPage] Request URI: " + std::wstring(endpoint) + L"\n").c_str());

            hstring responseBody = co_await ::quiz_examination_system::HttpHelper::SendSupabaseRequest(endpoint, L"", HttpMethod::Get());

            if (!lifetime)
            {
                OutputDebugStringW(L"[SystemLogsPage] Page destroyed during HTTP request\n");
                co_return;
            }

            OutputDebugStringW((L"[SystemLogsPage] Response Body Length: " + std::to_wstring(responseBody.size()) + L"\n").c_str());
            OutputDebugStringW((L"[SystemLogsPage] Full Response Body: " + std::wstring(responseBody) + L"\n").c_str());

            OutputDebugStringW(L"[SystemLogsPage] Attempting to parse JSON...\n");
            auto logsArray = JsonArray::Parse(responseBody);
            OutputDebugStringW((L"[SystemLogsPage] Parsed " + std::to_wstring(logsArray.Size()) + L" logs\n").c_str());

            m_allLogs.clear();
            m_logs.Clear();

            for (uint32_t i = 0; i < logsArray.Size(); i++)
            {
                try
                {
                    OutputDebugStringW((L"[SystemLogsPage] Processing log item " + std::to_wstring(i) + L"\n").c_str());
                    auto logObj = logsArray.GetObjectAt(i);

                    OutputDebugStringW(L"[SystemLogsPage] Getting id field...\n");
                    auto id = logObj.GetNamedString(L"id");
                    OutputDebugStringW((L"[SystemLogsPage] id=" + std::wstring(id) + L"\n").c_str());

                    OutputDebugStringW(L"[SystemLogsPage] Getting action field...\n");
                    auto action = logObj.GetNamedString(L"action");
                    OutputDebugStringW((L"[SystemLogsPage] action=" + std::wstring(action) + L"\n").c_str());

                    OutputDebugStringW(L"[SystemLogsPage] Getting actor_id field...\n");
                    auto actorId = logObj.GetNamedString(L"actor_id");
                    OutputDebugStringW((L"[SystemLogsPage] actor_id=" + std::wstring(actorId) + L"\n").c_str());

                    OutputDebugStringW(L"[SystemLogsPage] Getting target_table field...\n");
                    auto targetTable = logObj.GetNamedString(L"target_table", L"");
                    OutputDebugStringW((L"[SystemLogsPage] target_table=" + std::wstring(targetTable) + L"\n").c_str());

                    OutputDebugStringW(L"[SystemLogsPage] Getting target_id field...\n");
                    auto targetId = logObj.GetNamedString(L"target_id", L"");
                    OutputDebugStringW((L"[SystemLogsPage] target_id=" + std::wstring(targetId) + L"\n").c_str());

                    OutputDebugStringW(L"[SystemLogsPage] Getting created_at field...\n");
                    auto timestamp = logObj.GetNamedString(L"created_at");
                    OutputDebugStringW((L"[SystemLogsPage] created_at=" + std::wstring(timestamp) + L"\n").c_str());

                    OutputDebugStringW(L"[SystemLogsPage] Checking users field...\n");
                    hstring actorUsername = L"Unknown";
                    if (logObj.HasKey(L"users"))
                    {
                        OutputDebugStringW(L"[SystemLogsPage] users field exists\n");
                        auto usersValue = logObj.GetNamedValue(L"users");
                        OutputDebugStringW((L"[SystemLogsPage] users ValueType: " + std::to_wstring(static_cast<int>(usersValue.ValueType())) + L"\n").c_str());

                        if (usersValue.ValueType() == JsonValueType::Object)
                        {
                            OutputDebugStringW(L"[SystemLogsPage] Parsing users object...\n");
                            auto userObj = logObj.GetNamedObject(L"users");
                            actorUsername = userObj.GetNamedString(L"username", L"Unknown");
                            OutputDebugStringW((L"[SystemLogsPage] actorUsername=" + std::wstring(actorUsername) + L"\n").c_str());
                        }
                        else
                        {
                            OutputDebugStringW(L"[SystemLogsPage] users field is not an object\n");
                        }
                    }
                    else
                    {
                        OutputDebugStringW(L"[SystemLogsPage] users field does not exist, using Unknown\n");
                    }

                    OutputDebugStringW(L"[SystemLogsPage] Checking details field...\n");
                    hstring details = L"";
                    if (logObj.HasKey(L"details"))
                    {
                        OutputDebugStringW(L"[SystemLogsPage] details field exists\n");
                        auto detailsValue = logObj.GetNamedValue(L"details");
                        OutputDebugStringW((L"[SystemLogsPage] details ValueType: " + std::to_wstring(static_cast<int>(detailsValue.ValueType())) + L"\n").c_str());

                        if (detailsValue.ValueType() == JsonValueType::Object)
                        {
                            OutputDebugStringW(L"[SystemLogsPage] Stringifying details object...\n");
                            auto detailsObj = logObj.GetNamedObject(L"details");
                            details = detailsObj.Stringify();
                            OutputDebugStringW((L"[SystemLogsPage] details JSON: " + std::wstring(details).substr(0, 100) + L"...\n").c_str());
                        }
                        else if (detailsValue.ValueType() == JsonValueType::String)
                        {
                            OutputDebugStringW(L"[SystemLogsPage] Getting details as string...\n");
                            details = detailsValue.GetString();
                            OutputDebugStringW((L"[SystemLogsPage] details string: " + std::wstring(details).substr(0, 100) + L"...\n").c_str());
                        }
                        else
                        {
                            OutputDebugStringW(L"[SystemLogsPage] details field has unexpected type\n");
                        }
                    }
                    else
                    {
                        OutputDebugStringW(L"[SystemLogsPage] details field does not exist\n");
                    }

                    OutputDebugStringW(L"[SystemLogsPage] Creating LogItem object...\n");
                    auto logItem = make<LogItem>(id, action, actorId, actorUsername,
                                                 targetTable, targetId, details, timestamp);
                    OutputDebugStringW(L"[SystemLogsPage] LogItem created successfully\n");

                    m_allLogs.push_back(logItem);
                    OutputDebugStringW(L"[SystemLogsPage] Added to m_allLogs\n");

                    m_logs.Append(logItem);
                    OutputDebugStringW(L"[SystemLogsPage] Appended to m_logs\n");
                }
                catch (hresult_error const &ex)
                {
                    OutputDebugStringW((L"[SystemLogsPage] Error processing log item " + std::to_wstring(i) + L": " + ex.message() + L"\n").c_str());
                }
                catch (...)
                {
                    OutputDebugStringW((L"[SystemLogsPage] Unknown error processing log item " + std::to_wstring(i) + L"\n").c_str());
                }
            }

            OutputDebugStringW((L"[SystemLogsPage] Successfully loaded " + std::to_wstring(m_logs.Size()) + L" logs\n").c_str());

            if (lifetime && EmptyStateText() && m_logs.Size() == 0)
            {
                EmptyStateText().Visibility(Visibility::Visible);
            }
        }
        catch (hresult_error const &ex)
        {
            OutputDebugStringW((L"[SystemLogsPage] ERROR: " + ex.message() + L"\n").c_str());
            if (lifetime && MessageBar())
            {
                ShowMessage(L"Failed to load audit logs: " + ex.message(), InfoBarSeverity::Error);
            }
        }
        catch (...)
        {
            OutputDebugStringW(L"[SystemLogsPage] Unknown error\n");
            if (lifetime && MessageBar())
            {
                ShowMessage(L"Failed to load audit logs", InfoBarSeverity::Error);
            }
        }

        if (lifetime && LoadingRing())
        {
            LoadingRing().IsActive(false);
        }
    }

    void SystemLogsPage::OnSearchTextChanged(AutoSuggestBox const &sender, AutoSuggestBoxTextChangedEventArgs const &)
    {
        m_currentSearchText = sender.Text();
        FilterLogs();
    }

    void SystemLogsPage::OnActionFilterChanged(IInspectable const &sender, SelectionChangedEventArgs const &)
    {
        auto comboBox = sender.as<ComboBox>();
        auto selectedItem = comboBox.SelectedItem();

        if (selectedItem)
        {
            auto item = selectedItem.as<ComboBoxItem>();
            m_currentActionFilter = unbox_value<hstring>(item.Content());
            FilterLogs();
        }
    }

    void SystemLogsPage::OnRefreshClicked(IInspectable const &, RoutedEventArgs const &)
    {
        m_currentActionFilter = L"All";
        m_currentSearchText = L"";

        if (SearchBox())
        {
            SearchBox().Text(L"");
        }

        if (ActionFilterComboBox())
        {
            ActionFilterComboBox().SelectedIndex(0);
        }

        LoadLogs();
    }

    void SystemLogsPage::FilterLogs()
    {
        m_logs.Clear();

        auto lowerSearch = to_string(m_currentSearchText);
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

        for (auto const &log : m_allLogs)
        {
            bool matchesSearch = true;
            bool matchesAction = true;

            if (!m_currentSearchText.empty())
            {
                auto actor = to_string(log.ActorUsername());
                auto action = to_string(log.Action());
                auto details = to_string(log.Details());

                std::transform(actor.begin(), actor.end(), actor.begin(), ::tolower);
                std::transform(action.begin(), action.end(), action.begin(), ::tolower);
                std::transform(details.begin(), details.end(), details.begin(), ::tolower);

                matchesSearch = (actor.find(lowerSearch) != std::string::npos ||
                                 action.find(lowerSearch) != std::string::npos ||
                                 details.find(lowerSearch) != std::string::npos);
            }

            if (m_currentActionFilter != L"All")
            {
                matchesAction = (log.Action() == m_currentActionFilter);
            }

            if (matchesSearch && matchesAction)
            {
                m_logs.Append(log);
            }
        }

        if (EmptyStateText())
        {
            EmptyStateText().Visibility(m_logs.Size() == 0 ? Visibility::Visible : Visibility::Collapsed);
        }
    }

    void SystemLogsPage::ShowMessage(hstring const &message, InfoBarSeverity severity)
    {
        if (MessageBar())
        {
            ::quiz_examination_system::PageHelper::ShowInfoBar(MessageBar(), message, severity);
        }
    }
}

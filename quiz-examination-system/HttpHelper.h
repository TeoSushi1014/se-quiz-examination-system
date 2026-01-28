#pragma once

#include <winrt/base.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Storage.Streams.h>
#include <chrono>
#include "SupabaseConfig.h"

namespace quiz_examination_system
{
    class HttpHelper
    {
    public:
        static void SetSupabaseHeaders(winrt::Windows::Web::Http::HttpRequestMessage const &request)
        {
            auto headers = request.Headers();
            headers.Insert(L"apikey", SupabaseConfig::ANON_KEY);
            headers.Insert(L"Authorization", SupabaseConfig::GetAuthorizationHeader());

            // Force no-cache to prevent stale data
            headers.Insert(L"Cache-Control", L"no-cache, no-store, must-revalidate");
            headers.Insert(L"Pragma", L"no-cache");
        }

        static winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> SendSupabaseRequest(
            winrt::hstring const &url,
            winrt::hstring const &jsonBody = L"",
            winrt::Windows::Web::Http::HttpMethod method = winrt::Windows::Web::Http::HttpMethod::Post())
        {
            using namespace winrt;
            using namespace Windows::Web::Http;
            using namespace Windows::Storage::Streams;

            // Add cache-busting timestamp for GET requests
            hstring finalUrl = url;
            if (method == HttpMethod::Get())
            {
                auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                std::wstring urlStr(url);
                auto separator = urlStr.find(L'?') == std::wstring::npos ? L"?" : L"&";
                finalUrl = url + hstring(separator) + L"_t=" + to_hstring(timestamp);
            }

            HttpClient client;
            Windows::Foundation::Uri uri(finalUrl);
            HttpRequestMessage request(method, uri);

            SetSupabaseHeaders(request);

            if (!jsonBody.empty())
            {
                auto bodyStream = InMemoryRandomAccessStream();
                auto writer = DataWriter(bodyStream);
                writer.WriteString(jsonBody);
                co_await writer.StoreAsync();
                writer.DetachStream();
                bodyStream.Seek(0);

                auto content = HttpStreamContent(bodyStream);
                content.Headers().ContentType(Headers::HttpMediaTypeHeaderValue(L"application/json"));
                request.Content(content);
            }

            auto response = co_await client.SendRequestAsync(request);
            auto responseText = co_await response.Content().ReadAsStringAsync();

            co_return responseText;
        }
    };
}

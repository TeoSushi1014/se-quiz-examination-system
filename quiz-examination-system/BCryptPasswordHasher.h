#pragma once

#include <string>
#include <algorithm>
#include <winrt/Windows.Security.Cryptography.h>
#include <winrt/Windows.Security.Cryptography.Core.h>
#include <winrt/Windows.Storage.Streams.h>

namespace quiz_examination_system
{
    class BCryptPasswordHasher
    {
    private:
        static constexpr int WORK_FACTOR = 12;
        static constexpr int SALT_LENGTH = 16;

        static std::string GenerateSalt()
        {
            auto buffer = winrt::Windows::Security::Cryptography::CryptographicBuffer::GenerateRandom(SALT_LENGTH);
            auto saltHex = winrt::Windows::Security::Cryptography::CryptographicBuffer::EncodeToHexString(buffer);

            std::wstring upperSalt = saltHex.c_str();
            std::transform(upperSalt.begin(), upperSalt.end(), upperSalt.begin(), ::towupper);

            return winrt::to_string(winrt::hstring(upperSalt));
        }

        static std::string PBKDF2Hash(const std::string &password, const std::string &saltHex, int iterations)
        {
            try
            {
                auto provider = winrt::Windows::Security::Cryptography::Core::KeyDerivationAlgorithmProvider::OpenAlgorithm(
                    winrt::Windows::Security::Cryptography::Core::KeyDerivationAlgorithmNames::Pbkdf2Sha256());

                auto passwordBuffer = winrt::Windows::Security::Cryptography::CryptographicBuffer::ConvertStringToBinary(
                    winrt::to_hstring(password),
                    winrt::Windows::Security::Cryptography::BinaryStringEncoding::Utf8);

                auto saltBuffer = winrt::Windows::Security::Cryptography::CryptographicBuffer::DecodeFromHexString(
                    winrt::to_hstring(saltHex));

                auto key = provider.CreateKey(passwordBuffer);

                winrt::Windows::Security::Cryptography::Core::KeyDerivationParameters params =
                    winrt::Windows::Security::Cryptography::Core::KeyDerivationParameters::BuildForPbkdf2(saltBuffer, iterations);

                auto derivedKey = winrt::Windows::Security::Cryptography::Core::CryptographicEngine::DeriveKeyMaterial(
                    key,
                    params,
                    32);

                auto hashString = winrt::Windows::Security::Cryptography::CryptographicBuffer::EncodeToHexString(derivedKey);

                std::wstring upperHash = hashString.c_str();
                std::transform(upperHash.begin(), upperHash.end(), upperHash.begin(), ::towupper);

                return winrt::to_string(winrt::hstring(upperHash));
            }
            catch (...)
            {
                return "";
            }
        }

    public:
        static hstring HashPassword(hstring const &password)
        {
            try
            {
                OutputDebugStringW(L"[BCrypt] HashPassword started\n");
                std::string pwd = winrt::to_string(password);
                OutputDebugStringW((L"[BCrypt] Password converted, length: " + std::to_wstring(pwd.length()) + L"\n").c_str());

                std::string salt = GenerateSalt();
                OutputDebugStringW((L"[BCrypt] Salt generated: " + std::to_wstring(salt.length()) + L" chars\n").c_str());

                int iterations = 1 << WORK_FACTOR;
                OutputDebugStringW((L"[BCrypt] Iterations: " + std::to_wstring(iterations) + L"\n").c_str());

                std::string hash = PBKDF2Hash(pwd, salt, iterations);
                OutputDebugStringW((L"[BCrypt] Hash computed: " + std::to_wstring(hash.length()) + L" chars\n").c_str());

                if (hash.empty())
                {
                    OutputDebugStringW(L"[BCrypt] ERROR: Hash is empty!\n");
                    return L"";
                }

                std::string result = "$pbkdf2-sha256$i=" + std::to_string(iterations) + "$" + salt + "$" + hash;
                OutputDebugStringW((L"[BCrypt] Final result: " + std::to_wstring(result.length()) + L" chars\n").c_str());
                return winrt::to_hstring(result);
            }
            catch (winrt::hresult_error const &ex)
            {
                OutputDebugStringW((L"[BCrypt] hresult_error: " + ex.message() + L"\n").c_str());
                return hstring(L"");
            }
            catch (std::exception const &ex)
            {
                std::string msg = ex.what();
                OutputDebugStringW((L"[BCrypt] std::exception: " + winrt::to_hstring(msg) + L"\n").c_str());
                return hstring(L"");
            }
            catch (...)
            {
                OutputDebugStringW(L"[BCrypt] Unknown exception in HashPassword\n");
                return hstring(L"");
            }
        }

        static bool VerifyPassword(hstring const &password, hstring const &storedHash)
        {
            try
            {
                std::string pwd = winrt::to_string(password);
                std::string stored = winrt::to_string(storedHash);

                if (stored.find("$pbkdf2-sha256$i=") != 0)
                {
                    return false;
                }

                size_t iterPos = stored.find("$i=") + 3;
                size_t saltPos = stored.find("$", iterPos);
                size_t hashPos = stored.find("$", saltPos + 1);

                if (saltPos == std::string::npos || hashPos == std::string::npos)
                {
                    return false;
                }

                int iterations = std::stoi(stored.substr(iterPos, saltPos - iterPos));
                std::string salt = stored.substr(saltPos + 1, hashPos - saltPos - 1);
                std::string storedHashPart = stored.substr(hashPos + 1);

                std::string computedHash = PBKDF2Hash(pwd, salt, iterations);

                return computedHash == storedHashPart;
            }
            catch (...)
            {
                return false;
            }
        }
    };
}

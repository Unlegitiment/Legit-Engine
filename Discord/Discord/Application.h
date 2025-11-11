#pragma once
#define DISCORDPP_IMPLEMENTATION
#include <Discord/discord_social_sdk/include/discordpp.h>
#include <iostream>
using namespace discordpp;
class CDiscordApplication {
public:
	static constexpr auto APP_ID = 1435161339541848164;
	Client* m_pClient = nullptr;
	void Init() {
		m_pClient = new Client();

        m_pClient->AddLogCallback([](auto message, auto severity) {
            std::cout << "[" << EnumToString(severity) << "] " << message << std::endl;
            }, discordpp::LoggingSeverity::Info);

        // Set up status callback to monitor client connection
        m_pClient->SetStatusChangedCallback([this](discordpp::Client::Status status, discordpp::Client::Error error, int32_t errorDetail) {
            std::cout << "Status changed: " << discordpp::Client::StatusToString(status) << std::endl;
            if (status == discordpp::Client::Status::Ready) {
                std::cout << "Client is ready! You can now call SDK functions.\n";

                // Access initial relationships data
                std::cout << " Friends Count: " << m_pClient->GetRelationships().size() << std::endl;

                // Configure rich presence details
                discordpp::Activity activity;
                activity.SetName("Grand Theft Auto V");
                activity.SetType(discordpp::ActivityTypes::Playing);
                activity.SetState("Exploring Los Santos.");
                // Update rich presence
                m_pClient->UpdateRichPresence(activity, [](discordpp::ClientResult result) {
                    if (result.Successful()) {
                        std::cout << " Rich Presence updated successfully!\n";
                    }
                    else {
                        std::cerr << " Rich Presence update failed";
                    }
                    });

            }
            else if (error != discordpp::Client::Error::None) {
                std::cerr << " Connection Error: " << discordpp::Client::ErrorToString(error) << " - Details: " << errorDetail << std::endl;
            }
            });

        this->Authorize();
	}
	void Update() {
		discordpp::RunCallbacks();
	}
	void Shutdown() {

	}
private:
    void Authorize() {
        AuthorizationArgs args{};
        args.SetClientId(APP_ID);
        args.SetScopes(Client::GetDefaultPresenceScopes());
        auto codeVerif = m_pClient->CreateAuthorizationCodeVerifier();
        args.SetCodeChallenge(codeVerif.Challenge());
        m_pClient->Authorize(args, [this, codeVerif](auto result, auto code, auto redirectUri) {
            if (!result.Successful()) {
                std::cerr << "Authentication Error: " << result.Error() << std::endl;
                return;
            }
            else {
                std::cout << "Authorization successful! Getting access token...\n";

                // Exchange auth code for access token
                m_pClient->GetToken(APP_ID, code, codeVerif.Verifier(), redirectUri,
                    [this](discordpp::ClientResult result,
                        std::string accessToken,
                        std::string refreshToken,
                        discordpp::AuthorizationTokenType tokenType,
                        int32_t expiresIn,
                        std::string scope) {
                            std::cout << "Access token received! Establishing connection...\n";
                            // Next Step: Update the token and connect
                            m_pClient->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken, [this](discordpp::ClientResult result) {
                                if (result.Successful()) {
                                    std::cout << "Token updated, connecting to Discord...\n";
                                    m_pClient->Connect();
                                }
                                });
                    });
            }
            });
    }
	bool m_bIsRunning = true;
};
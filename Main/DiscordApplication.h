#pragma once
#include <thirdparty/discord_social_sdk/include/discordpp.h>
using richDiscordLogCallback = void(*)(const char*);
using richDiscordOnReady = void(*)(discordpp::Client*);
using richDiscordId = long long;
#include <queue>
using RichPresenceDebugCallback = void(*)(const char*);

class richPresenceDebug {
public:
	static void Init(RichPresenceDebugCallback dbg = OutputDebugStringA) {
		CB = dbg;
	}
	template<typename... T>
	static void Printf(const char* fmt, T&&... args) {
		char buff[1028]{0};
		sprintf_s(buff, fmt, args...);
		CB(buff);
	}
private:
	static inline RichPresenceDebugCallback CB = OutputDebugStringA;
};
#define richPrintf(x, ...) richPresenceDebug::Printf("[INFO][%s] " x, __FUNCTION__, __VA_ARGS__);

#define BIT(x) 1llu << x
enum richDiscordScopeFlag : unsigned long long {
	identify = BIT(0),
	email = BIT(1),
	connections = BIT(2),
	guilds = BIT(3),
	guilds_join = BIT(4),
	guilds_members_read = BIT(5),
	guilds_channels_read = BIT(6),
	gdm_join = BIT(7),
	bot = BIT(8),
	rpc = BIT(9),
	rpc_notifications_read = BIT(10),
	rpc_voice_read = BIT(11),
	rpc_voice_write = BIT(12),
	rpc_video_read = BIT(13),
	rpc_video_write = BIT(14),
	rpc_screenshare_read = BIT(15),
	rpc_screenshare_write = BIT(16),
	rpc_activities_write = BIT(17),
	webhook_incoming = BIT(18),
	messages_read = BIT(19),
	applications_builds_upload = BIT(20),
	applications_builds_read = BIT(21),
	applications_commands = BIT(22),
	applications_store_update = BIT(23),
	applications_entitlements = BIT(24),
	activities_read = BIT(25),
	activities_write = BIT(26),
	activities_invites_write = BIT(27),
	relationships_read = BIT(28),
	relationships_write = BIT(29),
	voice = BIT(30),
	dm_channels_read = BIT(31),
	role_connections_write = BIT(32),
	presences_read = BIT(33),
	presences_write = BIT(34),
	openid = BIT(35),
	dm_channels_messages_read = BIT(36),
	dm_channels_messages_write = BIT(37),
	gateway_connect = BIT(38),
	account_global_name_update = BIT(39),
	payment_sources_country_code = BIT(40),
	sdk_social_layer_presence = BIT(41),
	lobbies_write = BIT(42),
	application_identities_write = BIT(43),
	sdk_social_layer = BIT(44),
	application_commands_permission_update = BIT(45),
	MAX_BIT_COUNT = 46
};
#include <array>
#include <sstream>
struct richDiscordScopes {
	static richDiscordScopes GetDefaultPresence() {
		richDiscordScopes scope{};
		scope.Scopes |= richDiscordScopeFlag::sdk_social_layer_presence;
		return scope;
	}
	static richDiscordScopes GetDefaultCommunication() {
		richDiscordScopes scope{};
		scope.Scopes |= richDiscordScopeFlag::sdk_social_layer;
		return scope;
	}
	static constexpr int CountOfScopes = richDiscordScopeFlag::MAX_BIT_COUNT;
	long long Scopes : CountOfScopes;
};
class richDiscordScopeResolver {
public:
	richDiscordScopeResolver(richDiscordScopes& ScopeFlags) : Scopes(ScopeFlags) {}
	std::string GetResolvedResource() {
		std::stringstream str{};
		for (int i = 0; i < richDiscordScopes::CountOfScopes; i++) {
			bool Flag = Scopes.Scopes & (1ll << i);
			if (Flag) {
				str << ScopesString[i] << "+";
			}
		}
		auto res = str.str();
		if (!res.empty()) {
			res.pop_back();
		}
		richPrintf("%s\n", res.c_str());
		return res;
	}
private:
	static inline std::array<std::string, richDiscordScopes::CountOfScopes> ScopesString{
		"identify",
		"email",
		"connections",
		"guilds",
		"guilds.join",
		"guilds.members.read",
		"guilds.channels.read",
		"gdm.join",
		"bot",
		"rpc",
		"rpc.notifications.read",
		"rpc.voice.read",
		"rpc.voice.write",
		"rpc.video.read",
		"rpc.video.write",
		"rpc.screenshare.read",
		"rpc.screenshare.write",
		"rpc.activities.write",
		"webhook.incoming",
		"messages.read",
		"applications.builds.upload",
		"applications.builds.read",
		"applications.commands",
		"applications.store.update",
		"applications.entitlements",
		"activities.read",
		"activities.write",
		"activities.invites.write",
		"relationships.read",
		"relationships.write",
		"voice",
		"dm_channels.read",
		"role_connections.write",
		"presences.read",
		"presences.write",
		"openid",
		"dm_channels.messages.read",
		"dm_channels.messages.write",
		"gateway.connect",
		"account.global_name.update",
		"payment_sources.country_code",
		"sdk.social_layer_presence",
		"lobbies.write",
		"application_identities.write",
		"sdk.social_layer",
		"applications.commands.permission.update",
	};
	richDiscordScopes& Scopes;
};
struct richDiscordConfig {
public:
	richDiscordId ApplicationId{};
	richDiscordScopes Scopes{};
};
class richDiscordAuth {
public:
	/*
		Returns: Has Client Properly Authenticated. 
	*/
	static bool Authorize(discordpp::Client* Client, richDiscordConfig& config) {
				// Generate OAuth2 code verifier for authentication
		auto codeVerifier = Client->CreateAuthorizationCodeVerifier();
		Client->RegisterAuthorizeRequestCallback([] () {
			OutputDebugStringA("RegisterAuthorizeRequestCallback\n");
			});
		// Set up authentication arguments
		discordpp::AuthorizationArgs args{};
		args.SetClientId(config.ApplicationId);
		args.SetScopes(richDiscordScopeResolver(config.Scopes).GetResolvedResource());
		args.SetCodeChallenge(codeVerifier.Challenge());
		bool ResultOfFunctionCall = true;
		// Begin authentication process
		Client->Authorize(args, [&ResultOfFunctionCall, Client, config, codeVerifier] (auto result, auto code, auto redirectUri) {
			if (!result.Successful()) {
				std::cerr << "Authentication Error: " << result.Error() << std::endl;
				ResultOfFunctionCall = false;
				return;
			}
			else {
				std::cout << "Authorization successful! Getting access token...\n";

				// Exchange auth code for access token
				Client->GetToken(config.ApplicationId, code, codeVerifier.Verifier(), redirectUri,
					[Client](discordpp::ClientResult result,
						std::string accessToken,
						std::string refreshToken,
						discordpp::AuthorizationTokenType tokenType,
						int32_t expiresIn,
						std::string scope) {
							std::cout << "Access token received! Establishing connection...\n";
							Client->UpdateToken(discordpp::AuthorizationTokenType::Bearer, accessToken, [Client] (discordpp::ClientResult result) {
								if (result.Successful()) {
									std::cout << "Token updated, connecting to Discord...\n";
									Client->Connect();
								}
								});
					});
			}
			});
		return ResultOfFunctionCall;
	}
};
using richAction = std::function<void(discordpp::Client*)>;
class richDiscordClient {
public:
	richDiscordClient(richDiscordConfig& config) {
		m_pClient = new discordpp::Client();
		Init(config);
	}
	void EnqueueAction(richAction action) {
		m_Actions.push(action);
	}
	void SetActivity(const discordpp::Activity& Activity) {
		this->EnqueueAction([Activity] (discordpp::Client* p) {
				p->UpdateRichPresence(Activity, [] (auto res) {});
			});
		if (this->m_pCurrentActivity) {
			*m_pCurrentActivity = Activity;
		}
		else {
			m_pCurrentActivity = new discordpp::Activity(Activity);
		}
	}
	discordpp::Activity GetActivity() {
		return m_pCurrentActivity ? *m_pCurrentActivity : discordpp::Activity::nullobj;
	}
	bool IsActivitySet() {
		return m_pCurrentActivity != nullptr;
	}
	void ClearActivity() {
		delete m_pCurrentActivity; m_pCurrentActivity = nullptr;
		this->EnqueueAction([] (discordpp::Client* p) {
			p->ClearRichPresence();
		});
	}
	discordpp::Client* GetClient() {
		return this->m_pClient;
	}
	void Run() {
		while (IsClientReady() && !m_Actions.empty()) {
			m_Actions.front()(m_pClient);
			m_Actions.pop();
		}
		discordpp::RunCallbacks();
	}
	~richDiscordClient() {
		delete m_pCurrentActivity;
		delete m_pClient;
	}
	bool IsClientReady() const {
		return IsClientRunning;
	}
private:
	discordpp::Activity* m_pCurrentActivity = nullptr;

	std::queue<richAction> m_Actions;
	bool HasClientBeenAuthorized = false;
	bool IsClientRunning = false;
	discordpp::Client* m_pClient = nullptr;
	void Init(richDiscordConfig& config) {
		richPrintf("Procedure started\n");
		m_pClient->AddLogCallback([] (auto message, auto sever) {
			richPrintf("[LogCallback: %s]: %s", discordpp::EnumToString(sever), message.c_str());
			}, discordpp::LoggingSeverity::Info);
		m_pClient->SetStatusChangedCallback([this] (discordpp::Client::Status status, discordpp::Client::Error error, int32_t errorDetail) {
			richPrintf("[richPresence::StatusChanged]: %s\n", discordpp::Client::StatusToString(status).c_str());
			if (status == discordpp::Client::Status::Disconnected) {
				IsClientRunning = false;
			}
			if (status == discordpp::Client::Status::Ready) {
				richPrintf("[richPresence::StatusChanged] Client is ready");
				HasClientBeenAuthorized = true;
				IsClientRunning = true;
			}
			else if (error != discordpp::Client::Error::None) {
				richPrintf("[richPresence::StatusChanged] Connection Error: %s - Details: %lu\n", discordpp::Client::ErrorToString(error).c_str(), errorDetail);
			}
			});
		HasClientBeenAuthorized = richDiscordAuth::Authorize(m_pClient, config);
	}
};
#include <chrono>
class engineDiscordStatus {
	static constexpr richDiscordId APPLICATION_ID = 1464925633649049794;
public:
	engineDiscordStatus() {
		m_Stamp.SetStart(GetTime());
		richDiscordConfig conf;
		conf.ApplicationId = APPLICATION_ID;
		conf.Scopes = richDiscordScopes::GetDefaultPresence();
		m_pDiscordClient = new richDiscordClient(conf);
		InitStrings();
		TimeSinceLastUpdate = GetTime();
	}
	void Update() {

	}
	~engineDiscordStatus() {
		delete m_pDiscordClient;
	}
private:
	unsigned long long GetTime() const {
		return std::chrono::high_resolution_clock::now().time_since_epoch().count();
	}
	discordpp::Activity GetDef() {
		discordpp::Activity DiscordActivity{};
		DiscordActivity.SetType(discordpp::ActivityTypes::Playing);
		DiscordActivity.SetName("Legit Engine");
		discordpp::ActivityAssets assets{};
		assets.SetLargeImage("logo_main");
		assets.SetLargeUrl("https://www.rockstargames.com/VI/");
		assets.SetLargeText("Legit Engine");
		DiscordActivity.SetAssets(assets);
		DiscordActivity.SetDetails("Creating Worlds.");
		return DiscordActivity;
	}
	void InitStrings() {
		m_DetailsStrings.push_back("Creating Worlds!");
		m_DetailsStrings.push_back("Pushing boundaries");
		m_DetailsStrings.push_back("Writing C++ :(");
		m_DetailsStrings.push_back("This is a Cyclic List Test.");
	}
	std::string* GetNext() {
		if (m_DetailsStrings.empty()) return nullptr;
		m_Index = (m_Index + 1) % m_DetailsStrings.size();
		return &m_DetailsStrings[m_Index];
	}
	unsigned long long TimeSinceLastUpdate = 0;
	discordpp::ActivityTimestamps m_Stamp{};
	richDiscordClient* m_pDiscordClient;
	std::vector<std::string> m_DetailsStrings;
	std::string DetailsStringCurrent;
	size_t m_Index;
};
class richDiscord {
public:
	static void Init() {
		richDiscordConfig config{};
		config.ApplicationId = APPLICATION_ID;
		config.Scopes = richDiscordScopes::GetDefaultPresence();
		sm_pClient = new richDiscordClient(config);
		sm_Status = new char[1028] {0};
	}
	static discordpp::Activity GetEngineActivity() {

	}
	static richDiscordClient* GetClient() {
		return sm_pClient;
	}
	static inline std::vector<std::string> Participants{};
	static inline bool IsThereAnActiveCall = false;
	static bool IsClientActive() { return sm_pClient->IsClientReady(); }
	static void Update() {
		sm_pClient->Run();
		if (IsClientActive() && ImGui::Begin("Discord Application")) {
			if (ImGui::InputText("Discord Status", sm_Status, 1028)) {

			}
			if (sm_pClient->IsActivitySet() && ImGui::Button("Update Presence")) {
				auto a = sm_pClient->GetActivity();
				
				a.SetDetails(sm_Status);
				sm_pClient->SetActivity(a);
			}
			if (ImGui::Button("Start Presence")) {
				sm_pClient->SetActivity(GetEngineActivity());
			}
			if (ImGui::Button("Clear Presence")) {
				sm_pClient->ClearActivity();
			}
			ImGui::End();
		}
	}
	static void Shutdown() {
		delete sm_pClient;
	}
private:
	static inline char* sm_Status = nullptr;
	static inline richDiscordClient* sm_pClient = nullptr;
};
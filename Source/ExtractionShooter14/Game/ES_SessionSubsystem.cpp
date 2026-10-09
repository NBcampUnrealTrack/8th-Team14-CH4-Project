#include "ES_SessionSubsystem.h"

#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

const FName UES_SessionSubsystem::SessionGameKey =
	TEXT("SESSION_GAME");

const FString UES_SessionSubsystem::SessionGameValue =
	TEXT("ExtractionShooter14");

const FName UES_SessionSubsystem::ServerNameKey =
	TEXT("SERVER_NAME");

void UES_SessionSubsystem::Initialize(
	FSubsystemCollectionBase& Collection
)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* OnlineSubsystem =
		IOnlineSubsystem::Get();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[SessionSubsystem] Initialize "
			"OSS=%s Dedicated=%d"
		),
		OnlineSubsystem
			? *OnlineSubsystem->GetSubsystemName().ToString()
			: TEXT("NULL"),
		IsRunningDedicatedServer()
	);
}

void UES_SessionSubsystem::Deinitialize()
{
	ClearCreateSessionDelegate();
	ClearFindSessionsDelegate();
	ClearJoinSessionDelegate();
	ClearDestroySessionDelegate();

	SessionSearch.Reset();

	bIsConnecting = false;
	bCreateSessionAfterDestroy = false;

	Super::Deinitialize();
}

IOnlineSessionPtr
UES_SessionSubsystem::GetSessionInterface() const
{
	IOnlineSubsystem* OnlineSubsystem =
		IOnlineSubsystem::Get();

	if (OnlineSubsystem == nullptr)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[SessionSubsystem] "
				"OnlineSubsystem is null."
			)
		);

		return nullptr;
	}

	IOnlineSessionPtr SessionInterface =
		OnlineSubsystem->GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[SessionSubsystem] "
				"SessionInterface is invalid. OSS=%s"
			),
			*OnlineSubsystem
				->GetSubsystemName()
				.ToString()
		);
	}

	return SessionInterface;
}

bool UES_SessionSubsystem::HasExistingSession() const
{
	const IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		return false;
	}

	return SessionInterface->GetNamedSession(
		NAME_GameSession
	) != nullptr;
}

void UES_SessionSubsystem::CreateDedicatedSession(
	int32 MaxPlayers,
	const FString& ServerName
)
{
	UE_LOG(LogTemp, Warning,
		TEXT("CreateSession BuildId=%d"),
		GetBuildUniqueId());

	if (!IsRunningDedicatedServer())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[CreateDedicatedSession] "
				"This process is not a Dedicated Server."
			)
		);

		OnDedicatedSessionCreated.Broadcast(false);
		return;
	}

	IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		OnDedicatedSessionCreated.Broadcast(false);
		return;
	}

	PendingMaxPlayers =
		FMath::Max(1, MaxPlayers);

	PendingServerName =
		ServerName.IsEmpty()
			? TEXT("Extraction Shooter Server")
			: ServerName;

	/*
	 * 동일 프로세스에서 세션이 이미 존재하면
	 * 먼저 파괴한 뒤 다시 생성한다.
	 */
	if (SessionInterface->GetNamedSession(
			NAME_GameSession
		) != nullptr)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[CreateDedicatedSession] "
				"Existing session found. Destroy first."
			)
		);

		bCreateSessionAfterDestroy = true;
		DestroyCurrentSession();
		return;
	}

	FOnlineSessionSettings SessionSettings;

	/*
	 * Dedicated Server Session
	 */
	SessionSettings.bIsDedicated = true;
	SessionSettings.bIsLANMatch = false;

	SessionSettings.NumPublicConnections =
		PendingMaxPlayers;

	SessionSettings.NumPrivateConnections = 0;

	/*
	 * 검색 가능 및 게임 도중 참가 가능
	 */
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;

	/*
	 * Dedicated Server 브라우저 방식이므로
	 * Presence Lobby 기능은 사용하지 않는다.
	 */
	SessionSettings.bUsesPresence = false;
	SessionSettings.bAllowJoinViaPresence = false;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings.bUseLobbiesIfAvailable = false;

	SessionSettings.bAllowInvites = false;

	/*
	 * 서버와 클라이언트가 같은 값을 사용해야 한다.
	 */
	SessionSettings.BuildUniqueId = GetBuildUniqueId();
	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[CreateDedicatedSession] "
			"GetBuildUniqueId=%d SessionBuildUniqueId=%d"
		),
		GetBuildUniqueId(),
		SessionSettings.BuildUniqueId
	);
	/*
	 * App ID 480의 다른 프로젝트 세션과 구별하기 위한 값.
	 */
	SessionSettings.Set(
		SessionGameKey,
		SessionGameValue,
		EOnlineDataAdvertisementType::
			ViaOnlineServiceAndPing
	);

	SessionSettings.Set(
		ServerNameKey,
		PendingServerName,
		EOnlineDataAdvertisementType::
			ViaOnlineServiceAndPing
	);

	CreateSessionDelegateHandle =
		SessionInterface
			->AddOnCreateSessionCompleteDelegate_Handle(
				FOnCreateSessionCompleteDelegate::
					CreateUObject(
						this,
						&ThisClass::
							OnCreateSessionComplete
					)
			);

	const bool bRequestStarted =
		SessionInterface->CreateSession(
			0,
			NAME_GameSession,
			SessionSettings
		);

	if (!bRequestStarted)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"[CreateDedicatedSession] "
				"CreateSession request failed to start."
			)
		);

		ClearCreateSessionDelegate();

		OnDedicatedSessionCreated.Broadcast(false);
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[CreateDedicatedSession] "
			"Request started. MaxPlayers=%d Server=%s"
		),
		PendingMaxPlayers,
		*PendingServerName
	);
	UE_LOG(LogTemp, Warning,
		TEXT("SessionSettings.BuildUniqueId=%d"),
		SessionSettings.BuildUniqueId);
}

void UES_SessionSubsystem::OnCreateSessionComplete(
	FName SessionName,
	bool bWasSuccessful
)
{
	ClearCreateSessionDelegate();

	UE_LOG(
		LogTemp,
			Error,
		TEXT(
			"[OnCreateSessionComplete] "
			"Session=%s Success=%d"
		),
		*SessionName.ToString(),
		bWasSuccessful
	);

	OnDedicatedSessionCreated.Broadcast(
		bWasSuccessful
	);
}

void UES_SessionSubsystem::
FindAndJoinDedicatedServer()
{
	if (IsRunningDedicatedServer())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[FindAndJoinDedicatedServer] "
				"Dedicated Server cannot join another server."
			)
		);

		return;
	}

	if (bIsConnecting)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[FindAndJoinDedicatedServer] "
				"Connection attempt already in progress."
			)
		);

		return;
	}

	bIsConnecting = true;

	OnConnectionStateChanged.Broadcast(
		TEXT("서버를 검색하고 있습니다...")
	);

	FindDedicatedServer();
}

void UES_SessionSubsystem::FindDedicatedServer()
{
	IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		FinishConnectionAttempt(
			false,
			TEXT(
				"Steam 세션 인터페이스가 "
				"초기화되지 않았습니다."
			)
		);

		return;
	}

	/*
	 * 이전 검색 Delegate가 남아 있지 않도록 정리.
	 */
	ClearFindSessionsDelegate();

	SessionSearch =
		MakeShared<FOnlineSessionSearch>();

	SessionSearch->MaxSearchResults = 100;
	SessionSearch->bIsLanQuery = false;

	/*
	 * Dedicated Server Session만 검색한다.
	 */
	//SessionSearch->QuerySettings.Set(
	//	SEARCH_DEDICATED_ONLY,
	//	true,
	//	EOnlineComparisonOp::Equals
	//);

	FindSessionsDelegateHandle =
		SessionInterface
			->AddOnFindSessionsCompleteDelegate_Handle(
				FOnFindSessionsCompleteDelegate::
					CreateUObject(
						this,
						&ThisClass::
							OnFindSessionsComplete
					)
			);

	const bool bRequestStarted =
		SessionInterface->FindSessions(
			0,
			SessionSearch.ToSharedRef()
		);

	if (!bRequestStarted)
	{
		ClearFindSessionsDelegate();

		FinishConnectionAttempt(
			false,
			TEXT(
				"서버 검색 요청을 "
				"시작하지 못했습니다."
			)
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"[FindDedicatedServer] "
			"FindSessions request started."
		)
	);
}

void UES_SessionSubsystem::OnFindSessionsComplete(
	bool bWasSuccessful
)
{
	ClearFindSessionsDelegate();

	if (!bWasSuccessful)
	{
		FinishConnectionAttempt(
			false,
			TEXT("서버 검색에 실패했습니다.")
		);

		return;
	}

	if (!SessionSearch.IsValid())
	{
		FinishConnectionAttempt(
			false,
			TEXT("서버 검색 결과가 유효하지 않습니다.")
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"[OnFindSessionsComplete] "
			"Success=%d ResultCount=%d"
		),
		bWasSuccessful,
		SessionSearch->SearchResults.Num()
	);

	for (int32 Index = 0;
		 Index < SessionSearch->SearchResults.Num();
		 ++Index)
	{
		const FOnlineSessionSearchResult& Result =
			SessionSearch->SearchResults[Index];

		FString FoundGameName;
		FString FoundServerName;

		Result.Session.SessionSettings.Get(
			SessionGameKey,
			FoundGameName
		);

		Result.Session.SessionSettings.Get(
			ServerNameKey,
			FoundServerName
		);

		UE_LOG(
			LogTemp,
			Log,
			TEXT(
				"[SearchResult %d] "
				"Valid=%d Game=%s Server=%s "
				"Owner=%s Ping=%d Open=%d/%d"
			),
			Index,
			Result.IsValid(),
			*FoundGameName,
			*FoundServerName,
			*Result.Session.OwningUserName,
			Result.PingInMs,
			Result.Session.NumOpenPublicConnections,
			Result.Session
				.SessionSettings
				.NumPublicConnections
		);
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[SearchResult %d] BuildId=%d AppId=%d Server=%s"
			),
			Index,
			Result.Session.SessionSettings.BuildUniqueId,
			Result.Session.SessionSettings.BuildUniqueId,
			*FoundServerName
		);
	}

	const int32 TargetServerIndex =
		FindTargetServerIndex();
	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[OnFindSessionsComplete] "
			"TargetServerIndex=%d"
		),
		TargetServerIndex
	);
	if (TargetServerIndex == INDEX_NONE)
	{
		const int32 ResultCount =
			SessionSearch.IsValid()
			? SessionSearch->SearchResults.Num()
			: -1;

		FinishConnectionAttempt(
			false,
			FString::Printf(
				TEXT(
					"접속 가능한 전용 서버가 없습니다. "
					"(검색 결과: %d개)"
				),
				ResultCount
			)
		);

		return;
	}

	OnConnectionStateChanged.Broadcast(
		TEXT("서버에 접속하고 있습니다...")
	);
	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[OnFindSessionsComplete] "
			"Calling JoinDedicatedServer. Index=%d"
		),
		TargetServerIndex
	);
	JoinDedicatedServer(
		SessionSearch
			->SearchResults[TargetServerIndex]
	);
}

int32 UES_SessionSubsystem::
FindTargetServerIndex() const
{
	if (!SessionSearch.IsValid())
	{
		return INDEX_NONE;
	}

	int32 BestServerIndex = INDEX_NONE;
	int32 BestPing = MAX_int32;

	for (int32 Index = 0;
		 Index < SessionSearch->SearchResults.Num();
		 ++Index)
	{
		const FOnlineSessionSearchResult& Result =
			SessionSearch->SearchResults[Index];

		if (!Result.IsValid())
		{
			continue;
		}

		/*
		 * 공개 슬롯이 없는 서버 제외.
		 */
		if (Result.Session
				.NumOpenPublicConnections <= 0)
		{
			continue;
		}

		FString FoundGameValue;

		const bool bHasGameKey =
			Result.Session.SessionSettings.Get(
				SessionGameKey,
				FoundGameValue
			);

		if (!bHasGameKey)
		{
			continue;
		}

		if (FoundGameValue != SessionGameValue)
		{
			continue;
		}

		/*
		 * 현재는 서버가 하나지만,
		 * 여러 개가 잡힐 경우 Ping이 가장 낮은 서버 선택.
		 */
		const int32 Ping =
			Result.PingInMs >= 0
				? Result.PingInMs
				: MAX_int32 - 1;

		if (BestServerIndex == INDEX_NONE ||
			Ping < BestPing)
		{
			BestServerIndex = Index;
			BestPing = Ping;
		}
	}

	return BestServerIndex;

	/*if (!SessionSearch.IsValid())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[FindTargetServerIndex] SessionSearch is invalid.")
		);

		return INDEX_NONE;
	}

	for (int32 Index = 0;
		Index < SessionSearch->SearchResults.Num();
		++Index)
	{
		const FOnlineSessionSearchResult& Result =
			SessionSearch->SearchResults[Index];

		if (!Result.IsValid())
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT(
					"[FindTargetServerIndex] "
					"Index=%d rejected: invalid result"
				),
				Index
			);

			continue;
		}

		if (Result.Session.NumOpenPublicConnections <= 0)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT(
					"[FindTargetServerIndex] "
					"Index=%d rejected: no open slot"
				),
				Index
			);

			continue;
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[FindTargetServerIndex] "
				"Temporary selected Index=%d "
				"Ping=%d Open=%d/%d"
			),
			Index,
			Result.PingInMs,
			Result.Session.NumOpenPublicConnections,
			Result.Session.SessionSettings.NumPublicConnections
		);

		return Index;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[FindTargetServerIndex] No valid result found.")
	);

	return INDEX_NONE;*/

}

void UES_SessionSubsystem::JoinDedicatedServer(
	const FOnlineSessionSearchResult& SearchResult
)
{
	IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		FinishConnectionAttempt(
			false,
			TEXT(
				"Steam 세션 인터페이스가 "
				"유효하지 않습니다."
			)
		);

		return;
	}

	if (!SearchResult.IsValid())
	{
		FinishConnectionAttempt(
			false,
			TEXT("검색된 서버 정보가 유효하지 않습니다.")
		);

		return;
	}

	/*
	 * 이전 Join Delegate가 남아 있지 않도록 정리.
	 */
	ClearJoinSessionDelegate();

	/*
	 * 클라이언트에 같은 이름의 세션이 남아 있다면
	 * JoinSession이 AlreadyInSession으로 실패할 수 있다.
	 */
	if (SessionInterface->GetNamedSession(
			NAME_GameSession
		) != nullptr)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[JoinDedicatedServer] "
				"Client already has GameSession."
			)
		);

		FinishConnectionAttempt(
			false,
			TEXT(
				"기존 세션 정보가 남아 있습니다. "
				"세션을 정리한 뒤 다시 시도하세요."
			)
		);

		return;
	}

	JoinSessionDelegateHandle =
		SessionInterface
			->AddOnJoinSessionCompleteDelegate_Handle(
				FOnJoinSessionCompleteDelegate::
					CreateUObject(
						this,
						&ThisClass::
							OnJoinSessionComplete
					)
			);

	const bool bRequestStarted =
		SessionInterface->JoinSession(
			0,
			NAME_GameSession,
			SearchResult
		);

	if (!bRequestStarted)
	{
		ClearJoinSessionDelegate();

		FinishConnectionAttempt(
			false,
			TEXT(
				"서버 참가 요청을 "
				"시작하지 못했습니다."
			)
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"[JoinDedicatedServer] "
			"JoinSession request started."
		)
	);
}

void UES_SessionSubsystem::OnJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type Result
)
{
	ClearJoinSessionDelegate();

	UE_LOG(
		LogTemp,
		Error,
		TEXT(
			"[OnJoinSessionComplete] "
			"Session=%s Result=%d"
		),
		*SessionName.ToString(),
		static_cast<int32>(Result)
	);

	if (Result !=
		EOnJoinSessionCompleteResult::Success)
	{
		FString FailureMessage;

		switch (Result)
		{
		case EOnJoinSessionCompleteResult::
			SessionIsFull:
			FailureMessage =
				TEXT("서버의 접속 인원이 가득 찼습니다.");
			break;

		case EOnJoinSessionCompleteResult::
			SessionDoesNotExist:
			FailureMessage =
				TEXT("서버 세션이 더 이상 존재하지 않습니다.");
			break;

		case EOnJoinSessionCompleteResult::
			CouldNotRetrieveAddress:
			FailureMessage =
				TEXT("서버의 IP 주소를 가져오지 못했습니다.");
			break;

		case EOnJoinSessionCompleteResult::
			AlreadyInSession:
			FailureMessage =
				TEXT("이미 해당 세션에 참가 중입니다.");
			break;

		default:
			FailureMessage =
				TEXT("서버 세션 참가에 실패했습니다.");
			break;
		}

		FinishConnectionAttempt(
			false,
			FailureMessage
		);

		return;
	}

	IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		FinishConnectionAttempt(
			false,
			TEXT("세션 인터페이스가 유효하지 않습니다.")
		);

		return;
	}

	FString ConnectString;

	const bool bResolved =
		SessionInterface->GetResolvedConnectString(
			SessionName,
			ConnectString
		);

	if (!bResolved || ConnectString.IsEmpty())
	{
		FinishConnectionAttempt(
			false,
			TEXT("서버 접속 주소를 얻지 못했습니다.")
		);

		return;
	}

	UGameInstance* GameInstance =
		GetGameInstance();

	if (!IsValid(GameInstance))
	{
		FinishConnectionAttempt(
			false,
			TEXT("GameInstance가 유효하지 않습니다.")
		);

		return;
	}

	APlayerController* LocalPlayerController =
		GameInstance
			->GetFirstLocalPlayerController();

	if (!IsValid(LocalPlayerController))
	{
		FinishConnectionAttempt(
			false,
			TEXT(
				"로컬 PlayerController를 "
				"찾지 못했습니다."
			)
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[OnJoinSessionComplete] "
			"ResolvedConnectString=%s"
		),
		*ConnectString
	);

	OnConnectionStateChanged.Broadcast(
		TEXT("로비로 이동합니다...")
	);

	/*
	 * 성공 결과를 UI에 알린다.
	 * 이후 Title World가 ClientTravel로 정리된다.
	 */
	OnConnectionCompleted.Broadcast(true);

	bIsConnecting = false;

	/*
	 * bUseSteamNetworking=false이고
	 * IpNetDriver를 사용하면 일반적으로
	 * ConnectString은 IP:Port 형태다.
	 */
	LocalPlayerController->ClientTravel(
		ConnectString,
		ETravelType::TRAVEL_Absolute
	);
}

void UES_SessionSubsystem::FinishConnectionAttempt(
	bool bWasSuccessful,
	const FString& Message
)
{
	bIsConnecting = false;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[FinishConnectionAttempt] "
			"Success=%d Message=%s"
		),
		bWasSuccessful,
		*Message
	);

	OnConnectionStateChanged.Broadcast(Message);
	OnConnectionCompleted.Broadcast(
		bWasSuccessful
	);
}

void UES_SessionSubsystem::DestroyCurrentSession()
{
	IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (!SessionInterface.IsValid())
	{
		if (bCreateSessionAfterDestroy)
		{
			bCreateSessionAfterDestroy = false;

			OnDedicatedSessionCreated.Broadcast(false);
		}

		return;
	}

	if (SessionInterface->GetNamedSession(
			NAME_GameSession
		) == nullptr)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT(
				"[DestroyCurrentSession] "
				"No session to destroy."
			)
		);

		if (bCreateSessionAfterDestroy)
		{
			bCreateSessionAfterDestroy = false;

			CreateDedicatedSession(
				PendingMaxPlayers,
				PendingServerName
			);
		}

		return;
	}

	ClearDestroySessionDelegate();

	DestroySessionDelegateHandle =
		SessionInterface
			->AddOnDestroySessionCompleteDelegate_Handle(
				FOnDestroySessionCompleteDelegate::
					CreateUObject(
						this,
						&ThisClass::
							OnDestroySessionComplete
					)
			);

	const bool bRequestStarted =
		SessionInterface->DestroySession(
			NAME_GameSession
		);

	if (!bRequestStarted)
	{
		ClearDestroySessionDelegate();

		const bool bShouldRecreate =
			bCreateSessionAfterDestroy;

		bCreateSessionAfterDestroy = false;

		if (bShouldRecreate)
		{
			OnDedicatedSessionCreated.Broadcast(false);
		}
	}
}

void UES_SessionSubsystem::OnDestroySessionComplete(
	FName SessionName,
	bool bWasSuccessful
)
{
	ClearDestroySessionDelegate();

	UE_LOG(
		LogTemp,
		Error,
		TEXT(
			"[OnDestroySessionComplete] "
			"Session=%s Success=%d"
		),
		*SessionName.ToString(),
		bWasSuccessful
	);

	const bool bShouldRecreate =
		bCreateSessionAfterDestroy;

	bCreateSessionAfterDestroy = false;

	if (bShouldRecreate)
	{
		if (!bWasSuccessful)
		{
			OnDedicatedSessionCreated.Broadcast(false);
			return;
		}

		CreateDedicatedSession(
			PendingMaxPlayers,
			PendingServerName
		);
	}
}

void UES_SessionSubsystem::
ClearCreateSessionDelegate()
{
	IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (SessionInterface.IsValid() &&
		CreateSessionDelegateHandle.IsValid())
	{
		SessionInterface
			->ClearOnCreateSessionCompleteDelegate_Handle(
				CreateSessionDelegateHandle
			);
	}

	CreateSessionDelegateHandle.Reset();
}

void UES_SessionSubsystem::
ClearFindSessionsDelegate()
{
	IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (SessionInterface.IsValid() &&
		FindSessionsDelegateHandle.IsValid())
	{
		SessionInterface
			->ClearOnFindSessionsCompleteDelegate_Handle(
				FindSessionsDelegateHandle
			);
	}

	FindSessionsDelegateHandle.Reset();
}

void UES_SessionSubsystem::
ClearJoinSessionDelegate()
{
	IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (SessionInterface.IsValid() &&
		JoinSessionDelegateHandle.IsValid())
	{
		SessionInterface
			->ClearOnJoinSessionCompleteDelegate_Handle(
				JoinSessionDelegateHandle
			);
	}

	JoinSessionDelegateHandle.Reset();
}

void UES_SessionSubsystem::
ClearDestroySessionDelegate()
{
	IOnlineSessionPtr SessionInterface =
		GetSessionInterface();

	if (SessionInterface.IsValid() &&
		DestroySessionDelegateHandle.IsValid())
	{
		SessionInterface
			->ClearOnDestroySessionCompleteDelegate_Handle(
				DestroySessionDelegateHandle
			);
	}

	DestroySessionDelegateHandle.Reset();
}
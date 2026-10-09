#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ES_SessionSubsystem.generated.h"

/**
 * 접속 과정의 상태 문자열을 UI에 전달한다.
 *
 * 예:
 * - 서버 검색 중...
 * - 서버 접속 중...
 * - 접속 가능한 서버가 없습니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSessionConnectionStateChanged,
	const FString&,
	StateMessage
);

/**
 * 자동 접속 시도의 최종 결과.
 *
 * 성공하면 곧 ClientTravel이 발생한다.
 * 실패하면 Title UI의 버튼을 다시 활성화하는 데 사용한다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSessionConnectionCompleted,
	bool,
	bWasSuccessful
);

/**
 * Dedicated Server의 Session 생성 결과.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnDedicatedSessionCreated,
	bool,
	bWasSuccessful
);

UCLASS()
class EXTRACTIONSHOOTER14_API UES_SessionSubsystem
	: public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(
		FSubsystemCollectionBase& Collection
	) override;

	virtual void Deinitialize() override;

public:
	/**
	 * Dedicated Server에서 호출한다.
	 *
	 * 서버 프로세스가 LobbyMap을 연 뒤
	 * Steam에 검색 가능한 Dedicated Session을 등록한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void CreateDedicatedSession(
		int32 MaxPlayers = 3,
		const FString& ServerName = TEXT("Extraction Shooter Server")
	);

	/**
	 * Title UI의 GameStart 버튼에서 호출한다.
	 *
	 * 서버 검색
	 * → 서버 자동 선택
	 * → JoinSession
	 * → ClientTravel
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void FindAndJoinDedicatedServer();

	/**
	 * 현재 검색 또는 접속이 진행 중인지 반환한다.
	 */
	UFUNCTION(BlueprintPure, Category = "Online|Session")
	bool IsConnecting() const
	{
		return bIsConnecting;
	}


	/**
	 * 서버 종료 또는 수동 정리 시 사용한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Online|Session")
	void DestroyCurrentSession();

public:
	UPROPERTY(BlueprintAssignable, Category = "Online|Session")
	FOnSessionConnectionStateChanged OnConnectionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Online|Session")
	FOnSessionConnectionCompleted OnConnectionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Online|Session")
	FOnDedicatedSessionCreated OnDedicatedSessionCreated;

private:
	/**
	 * 현재 OnlineSubsystem의 SessionInterface를 가져온다.
	 */
	IOnlineSessionPtr GetSessionInterface() const;

	/**
	 * 실제 FindSessions를 시작한다.
	 */
	void FindDedicatedServer();

	/**
	 * 검색 결과 중 우리 게임의 Dedicated Server를 찾는다.
	 */
	int32 FindTargetServerIndex() const;

	/**
	 * 선택된 검색 결과에 JoinSession을 요청한다.
	 */
	void JoinDedicatedServer(
		const FOnlineSessionSearchResult& SearchResult
	);

	/**
	 * 접속 시도를 실패 상태로 종료한다.
	 */
	void FinishConnectionAttempt(
		bool bWasSuccessful,
		const FString& Message
	);

	void ClearCreateSessionDelegate();
	void ClearFindSessionsDelegate();
	void ClearJoinSessionDelegate();
	void ClearDestroySessionDelegate();

private:
	void OnCreateSessionComplete(
		FName SessionName,
		bool bWasSuccessful
	);

	void OnFindSessionsComplete(
		bool bWasSuccessful
	);

	void OnJoinSessionComplete(
		FName SessionName,
		EOnJoinSessionCompleteResult::Type Result
	);

	void OnDestroySessionComplete(
		FName SessionName,
		bool bWasSuccessful
	);

private:
	/**
	 * FindSessions의 검색 결과를 비동기 완료 시점까지 보관한다.
	 */
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FDelegateHandle CreateSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;

	/**
	 * GameStart 버튼 중복 입력 방지.
	 */
	bool bIsConnecting = false;

	/**
	 * 기존 세션 파괴 후 다시 생성할 때 사용할 값.
	 */
	bool bCreateSessionAfterDestroy = false;

	int32 PendingMaxPlayers = 3;
	FString PendingServerName = TEXT("Extraction Shooter Server");

private:
	/**
	 * App ID 480을 사용할 경우 다른 프로젝트 세션과 구별하기 위한 키.
	 */
	static const FName SessionGameKey;
	static const FString SessionGameValue;

	static const FName ServerNameKey;
};
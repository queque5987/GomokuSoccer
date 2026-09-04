#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "GameSessionData.generated.h"

UCLASS(BlueprintType)
class GOSOCCER_API UGameSessionData : public UObject
{
	GENERATED_BODY()
private:
	FString SessionName;
	FString HostName;
	int32 CurrentPlayers;
	int32 MaxPlayers;
	int32 Ping;

	FOnlineSessionSearchResult OnlineSessionSearchResult;
	FSessionDisplayData SessionDisplayData;
public:
	void GetOnlineSessionSearchResult(FOnlineSessionSearchResult& OutOnlineSessionSearchResult) { OutOnlineSessionSearchResult = OnlineSessionSearchResult; };
	void SetOnlineSessionSearchResult(const FOnlineSessionSearchResult& InOnlineSessionSearchResult) { OnlineSessionSearchResult = InOnlineSessionSearchResult; };
	UFUNCTION(BlueprintCallable)
	void GetSessionName(FString& OutString) { OutString = SessionName; }
	void SetSessionName(FString InString) { SessionName = InString; };

	void SetSessionDisplayData(const FSessionDisplayData& InSessionDisplayData) { SessionDisplayData = InSessionDisplayData; };
	UFUNCTION(BlueprintCallable)
	void GetSessionDisplayData(FSessionDisplayData& OutSessionDisplayData) { OutSessionDisplayData = SessionDisplayData; };
};

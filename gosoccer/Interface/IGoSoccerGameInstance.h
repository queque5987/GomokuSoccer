#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "IGoSoccerGameInstance.generated.h"

UINTERFACE(MinimalAPI)
class UIGoSoccerGameInstance : public UInterface
{
	GENERATED_BODY()
};

class GOSOCCER_API IIGoSoccerGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetCurrentMainUIState(const EMainUIState& NewMainUIState);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetCurrentMainUIState(EMainUIState& OutMainUIState);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetMaxFPS(float NewMaxFPS);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetbUseLowTexture(bool NewbUseLowTexture);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetfShadowLevel(float NewfShadowLevel);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetAAType(int32 NewAAType);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetSettingFloat(const ESettingConfigParamType SettingConfigParamType, float fNewLevel);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetScreenResolution(FIntPoint NewResolution);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetWindowMode(EWindowMode::Type NewWindowMode);
	/** 엔진 표시 언어(Culture)를 바꾸고 UGoSoccerGameUserSettings 에도 반영해서 저장한다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetCultureSetting(const FString& NewCulture);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ApplySavedSettings();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetWinDollCircularEffect(bool bOnEffect);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetPlayGameMode(EPlayGameMode& OutPlayGameMode);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPlayGameMode(EPlayGameMode InPlayGameMode);

	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	//bool HostGame(const FUniqueNetIdRepl& UniqueNetIdRepl, const FName& CreateSessionName);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool HostGame(const FUniqueNetIdRepl& UniqueNetIdRepl, const FSessionCreateData& SessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool JoinSelectedSession(UObject* SelectedOnlineSessionData, const FUniqueNetIdRepl& UniqueNetIdRepl);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool FindSession(const FUniqueNetIdRepl& UniqueNetIdRepl);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool LeaveSession();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Steam_CreateSession();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool BindDelegate_Widget(UObject* WidgetDelegate);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetIdentityPlayerNickName(FString& OutPlayerNickName);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int64 GetCSteamID();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetSteamOSSOnline();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetSessionCreateData_FromSessionInterface(FSessionCreateData& OutSessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetSessionCreateData(const FSessionCreateData& InSessionCreateData);
	/**
	 * 호스트가 세션을 만들 때 사용한 설정.
	 * PlayerController / PlayerState 는 트래블 때 파괴되지만 GameInstance 는 유지되므로,
	 * 리슨 서버로 넘어간 뒤에도 여기서 원본 설정을 꺼내올 수 있다.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetHostingSessionCreateData(FSessionCreateData& OutHostingSessionCreateData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetPlayerSessionHost(bool e);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetPlayerSessionHost(bool& OutResult);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool MarkUIState_ClientTravelCallback(EMainUIState InSucceededUIState, EMainUIState InFailededUIState);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetClientTravelCallbackUIState(EMainUIState& OutSucceededUIState, EMainUIState& OutFailededUIState);

	/**
	 * 세션 로딩 진행 상태를 GameInstance 에 기록한다.
	 * PlayerController 는 트래블 때 파괴되므로, 로딩 UI 를 다시 세워야 하는지 판단할 근거를 여기에 남긴다.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool MarkSessionLoadingPhase(ESessionLoadingPhase InLoadingPhase);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetSessionLoadingPhase(ESessionLoadingPhase& OutLoadingPhase);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool StackChattingMessage(UObject* ChattingBoxObject);


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Steam_InviteFriend(int64 FriendSteamID);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Steam_GetInvitableFriends(TArray<FSteamFriendData>& InInvitableArr);
};

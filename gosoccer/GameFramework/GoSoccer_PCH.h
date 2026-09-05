#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "GoSoccer_PCH.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSwitchFingerMode, uint8);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimFlicker, FVector);
DECLARE_MULTICAST_DELEGATE(FOnClientLoadingCompleted);
DECLARE_MULTICAST_DELEGATE(FOnClearOnlineSessionList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContinueMatch);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStarted);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateOnlineSessionList, const TArray<FSessionDisplayData>& /*SessionResults*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStartResponse, bool /*bWasSuccessful*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStartHostGame, const FSessionCreateData& /*SessionCreateData*/);

DECLARE_DELEGATE_TwoParams(FOnCreateSessionCompleteDelegate, FName /*SessionName*/, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnFindSessionsCompleteDelegate, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnCutomButtonValueChangedDelegate, float /*NewValue*/);
DECLARE_DELEGATE_TwoParams(FOnJoinSessionCompleteDelegate, FName /*SessionName*/, EOnJoinSessionCompleteResult::Type /*Result*/);
DECLARE_DELEGATE_TwoParams(FOnWidgetSelected, UObject* /*SelectedWidgetItSelf*/, UObject* /*SelectedItem*/);

DECLARE_DELEGATE_TwoParams(FOnSetOminousHUDPercent, float /*Current Time Sec*/, float /*Max Time Sec*/);
DECLARE_DELEGATE_OneParam(FOnAddRealtimeColldownBonus, float /*CoolDown Bonus Portion(Max Cooldown)*/);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSteamAvatarLoaded, UObject*, AvatarTexture2DObject);
DECLARE_DYNAMIC_DELEGATE(FOnMatchScoreWidgetEnd);

constexpr float HIT_COOLDOWN_ADVANTAGE = 0.1f;

const FLinearColor NORMAL_DOLL_CYLINDER_CHARGE_COLOR = FLinearColor{ 0.12f, 0.12f, 0.1f, 1.f };

UENUM(BlueprintType)
enum class ELanguageSetting : uint8
{
	ELS_Default,
	ELS_English,
	ELS_Korean
};
UENUM(BlueprintType)
enum class EDollType : uint8
{
	EDT_Default,
	EDT_Explosive,
	EDT_Beam
};

UENUM(BlueprintType)
enum class ESettingConfigParamType : uint8
{
	ESCPT_Default,
// Sound
	ESCPT_MasterVolume,
	ESCPT_BGMVolume,
	ESCPT_FXVolume,
// Graphic
	ESCPT_Graphic_Total,
	ESCPT_Graphic_Shadow,
	ESCPT_Graphic_GlobalIllumination,
	ESCPT_Graphic_Reflection,
	ESCPT_Graphic_PostProcess,
	ESCPT_Graphic_Texture,
	ESCPT_Graphic_Effect
};

UENUM(BlueprintType)
enum class EPlayTurnRule : uint8
{
	EPTR_Default,
	EPTR_TakeTurn,
	EPTR_RealTime
};

UENUM(BlueprintType)
enum class EPlayGameRule : uint8
{
	EPGR_Default,
	EPGR_5or5Mode,
	EPGR_5plus5Mode
};

UENUM(BlueprintType)
enum class ECameraPosition : uint8
{
	ECP_Default,
	ECP_Lobby,
	ECP_InPlay,
	ECP_Session,
	ECP_VeryFirst
};

UENUM(BlueprintType)
enum class ETemporalMessageType : uint8
{
	ETMT_Default,
	ETMT_Warning,
	ETMT_Error,
	ETMT_Notice
};

UENUM(BlueprintType)
enum class EBoardPreset : uint8
{
	EBP_Default,
	EBP_Free,
	EBP_China,
	EBP_Korea,
	EBP_Tibet
};

UENUM(BlueprintType)
enum class EMatchScoreLampState : uint8
{
	EMSLS_Default,
	EMSLS_Green,
	EMSLS_Broken
};

UENUM(BlueprintType)
enum class EMatchRule : uint8
{
	EMR_Default,
	EMR_BO1,
	EMR_BO3,
	EMR_BO5,
	EMR_BO7
};

UENUM(BlueprintType)
enum class EPlayDollType : uint8
{
	EPDT_Default,
	EPDT_AntiFragile,
	EPDT_Fragile
};

UENUM(BlueprintType)
enum class EPlayGameMode : uint8
{
	EPGM_Default,
	EPGM_HotSeatGame,
	EPGM_OnlineSession,
	EPGM_SingleAIPlay,
	EPGM_Debug
};

UENUM(BlueprintType)
enum class EMainUIState : uint8
{
	EMUIS_Default,
	EMUIS_VeryFirst,
	EMUIS_SessionSearch,
	EMUIS_InLobby,
	EMUIS_GameStarted,
	EMUIS_SessionCreate,
	EMUIS_Settings,
	EMUIS_Loading,
	EMUIS_MultiPlay,
	EMUIS_SinglePlay,
	EMUIS_GameConfigSetting
};

/**
 * 세션 생성/조인 진행 상태.
 * 요청 시점에 세팅되고, 트래블 후 새 PlayerController 의 Load() 가 끝나면 ESLP_None 으로 돌아간다.
 * GameInstance 가 보관하므로 레벨 트래블을 넘어가도 유지된다.
 */
UENUM(BlueprintType)
enum class ESessionLoadingPhase : uint8
{
	/** 로딩 중 아님 (로딩 UI Off) */
	ESLP_None,
	/** CreateSession 요청 ~ 리슨 서버 레벨 로드 완료 */
	ESLP_CreateSession,
	/** JoinSession 요청 ~ 접속 후 로비 진입 완료 */
	ESLP_JoinSession
};


UENUM(BlueprintType)
enum class EDollColor : uint8
{
	EDC_Default,
	EDC_Black,
	EDC_White,
	EDC_Max
};

UENUM(BlueprintType)
enum class EPlayerTaskStatus : uint8
{
	EPTS_Default,
	EPTS_Running,
	EPTS_Finished
};

UENUM(BlueprintType)
enum class EDollStencilValue : uint8
{
	EDSV_Default = 0,
	EDSV_MouseOver = 100,
	EDSV_MouseOver_NotMine = 101
};

UENUM(BlueprintType)
enum class EScoreType : uint8
{
	EST_Default,
	EST_OnBoardScore,
	EST_FallScore,
	EST_BreakScore,
	EST_TotalScore
};

USTRUCT(BlueprintType)
struct FSteamFriendData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	int64 SteamID = 0;

	UPROPERTY(BlueprintReadOnly)
	FString PersonaName;

	UPROPERTY(BlueprintReadOnly)
	bool bIsOnline = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayingThisGame = false;
};

USTRUCT(BlueprintType)
struct FMatchScoreLampStateConfig
{
	GENERATED_BODY()
	FMatchScoreLampStateConfig() 
	{
		MatchScoreLampIndex = -1;
		MatchScoreFillColor = -1;
		TargetLampState = EMatchScoreLampState::EMSLS_Default;
	};
	FMatchScoreLampStateConfig(
		int32 InIndex, int32 FillDollColor, EMatchScoreLampState InMatchScoreLampState) :
		MatchScoreLampIndex(InIndex), MatchScoreFillColor(FillDollColor), TargetLampState(InMatchScoreLampState)
	{};

public:
	UPROPERTY(BlueprintReadOnly)
	int32 MatchScoreLampIndex;
	UPROPERTY(BlueprintReadOnly)
	int32 MatchScoreFillColor;
	UPROPERTY(BlueprintReadOnly)
	EMatchScoreLampState TargetLampState;
};

USTRUCT(BlueprintType)
struct FDollDamageConfig
{
	GENERATED_BODY()
	FDollDamageConfig() {};
	FDollDamageConfig(
		float InDamageAmount, 
		FVector InHitDirection, FVector InHitLocation,
		FVector InImpulseLocation, float InKnockBackAmount
	) : DamageAmount(InDamageAmount),
		HitDirection(InHitDirection), HitLocation(InHitLocation),
		ImpulseLocation(InImpulseLocation), KnockBackAmount(InKnockBackAmount)
	{}

public:
	UPROPERTY(BlueprintReadOnly)
	float DamageAmount;
	UPROPERTY(BlueprintReadOnly)
	FVector HitDirection;
	UPROPERTY(BlueprintReadOnly)
	FVector HitLocation;
	UPROPERTY(BlueprintReadOnly)
	FVector ImpulseLocation;
	UPROPERTY(BlueprintReadOnly)
	float KnockBackAmount;
};

USTRUCT(BlueprintType)
struct FCylinderConfig
{
	GENERATED_BODY()
	FCylinderConfig()
	{
		Clock = 0.f;
		ChargePercent = FVector4::Zero();
		ShadowTextureIndex = FVector4{ -1.f, -1.f, -1.f, -1.f };

		OverlayColor_0 = NORMAL_DOLL_CYLINDER_CHARGE_COLOR;
		OverlayColor_1 = NORMAL_DOLL_CYLINDER_CHARGE_COLOR;
		OverlayColor_2 = NORMAL_DOLL_CYLINDER_CHARGE_COLOR;
		OverlayColor_3 = NORMAL_DOLL_CYLINDER_CHARGE_COLOR;

		CircleColor_0 = FLinearColor::Black;
		CircleColor_1 = FLinearColor::Black;
		CircleColor_2 = FLinearColor::Black;
		CircleColor_3 = FLinearColor::Black;
	}

public:
	UPROPERTY(BlueprintReadWrite)
	float Clock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 ChargePercent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector4 ShadowTextureIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor OverlayColor_0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor OverlayColor_1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor OverlayColor_2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor OverlayColor_3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CircleColor_0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CircleColor_1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CircleColor_2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CircleColor_3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseClockCurve = true;
};

USTRUCT(BlueprintType)
struct FScoreBoard
{
	GENERATED_BODY()
	FScoreBoard()
	{
		OnBoardScore = 0;
		FallScore = 0;
		BreakScore = 0;
	}
	FScoreBoard(uint8 InDollColor, FName InPlayerName) : PlayerName(InPlayerName), PlayerDollColor(InDollColor)
	{
		OnBoardScore = 0;
		FallScore = 0;
		BreakScore = 0;
	}

public:
	UPROPERTY(BlueprintReadOnly)
	FName PlayerName;
	UPROPERTY(BlueprintReadOnly)
	uint8 PlayerDollColor;
	UPROPERTY(BlueprintReadOnly)
	int32 OnBoardScore;
	UPROPERTY(BlueprintReadOnly)
	int32 FallScore;
	UPROPERTY(BlueprintReadOnly)
	int32 BreakScore;
};

USTRUCT(BlueprintType)
struct FStreakDollContainer
{
	GENERATED_BODY()
	FStreakDollContainer()
	{
		DollDataContainer.Reserve(1);
		DollDataContainer.SetNum(1);
	};
	FStreakDollContainer(int32 SizeOfContainer)
	{
		DollDataContainer.Reserve(SizeOfContainer);
		DollDataContainer.SetNum(SizeOfContainer);
	};
public:
	struct FBitProxy
	{
		uint64& Container;
		uint64 Mask;
		void operator=(bool b)
		{
			if (b) Container |= Mask;
			else   Container &= ~Mask;
		}
		operator bool() const { return (Container & Mask) != 0; }
	};
	FORCEINLINE FBitProxy operator[](int32 idx)
	{
		int32 C = idx / 64;
		int32 i = idx % 64;
		check(C < DollDataContainer.Num());
		return FBitProxy{ DollDataContainer[C], 1ULL << i };
	};
	bool operator==(const FStreakDollContainer& Other) const
	{
		if (DollDataContainer.Num() != Other.DollDataContainer.Num()) return false;
		return DollDataContainer == Other.DollDataContainer;
	};
	bool operator!=(const FStreakDollContainer& Other) const
	{
		return !(*this == Other);
	};
	void SetNum(int32 size)
	{
		DollDataContainer.Empty();
		DollDataContainer.Reserve(size);
		DollDataContainer.SetNum(size);
	};
	const TArray<uint64>& GetData() const { return DollDataContainer; };
	int32 GetCount() const
	{
		int32 rtn = 0;
		for (uint64 Iter : DollDataContainer) { rtn += FMath::CountBits(Iter); }
		return rtn;
	};
	bool IsSubsetOf(const FStreakDollContainer& Other) const
	{
		for (int32 i = 0; i < DollDataContainer.Num(); i++)
		{
			if (i >= Other.DollDataContainer.Num()) return false;
			else if (DollDataContainer[i] != (DollDataContainer[i] & Other.DollDataContainer[i]))
			{
				return false;
			}
		}
		return true;
	};
	bool IsValidIndex(int32 idx)
	{
		int32 n = idx / 64;
		return DollDataContainer.Num() > (n);
	};
	void Clear()
	{
		for (uint64& Iter : DollDataContainer)
		{
			Iter = 0;
		}
	}
private:
	TArray<uint64> DollDataContainer;
};

USTRUCT(BlueprintType)
struct FSessionCreateData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	bool IsSecret = false;
	UPROPERTY(BlueprintReadOnly)
	FString SessionName;

	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	UPROPERTY(BlueprintReadOnly)
	EPlayGameRule PlayGameRule = EPlayGameRule::EPGR_Default;

	UPROPERTY(BlueprintReadOnly)
	EPlayGameMode PlayGameMode = EPlayGameMode::EPGM_Default;

	UPROPERTY(BlueprintReadOnly)
	EBoardPreset BoardPreset = EBoardPreset::EBP_Default;

	UPROPERTY(BlueprintReadOnly)
	EPlayTurnRule PlayTurnRule = EPlayTurnRule::EPTR_Default;

	UPROPERTY(BlueprintReadOnly)
	EPlayDollType PlayDollType = EPlayDollType::EPDT_Default;

	UPROPERTY(BlueprintReadOnly)
	EMatchRule MatchRule = EMatchRule::EMR_Default;

	UPROPERTY(BlueprintReadOnly)
	float TimeLimit = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float AngularThreshold = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float DistanceThreshold = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bEnableItem = false;

	/*AI Mode Only, Not Available For Now*/
	//UPROPERTY(BlueprintReadOnly)
	//int32 PlayAsDollColor = -1;

	/**
	 * 실제 세션 설정이 채워진 값인지. (기본 생성 상태와 구분하는 용도)
	 * 호스트의 설정을 아직 못 받은 쪽에서 툴팁을 빈 값으로 덮어쓰지 않도록 검사한다.
	 */
	bool IsValidSessionConfig() const { return PlayGameRule != EPlayGameRule::EPGR_Default; }
};

USTRUCT(BlueprintType)
struct FSessionDisplayData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ServerName;

	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly)
	int32 Ping;

	UPROPERTY(BlueprintReadOnly)
	FBlueprintSessionResult RawResult;

	UPROPERTY(BlueprintReadOnly)
	EPlayGameRule PlayGameRule;
};

// FGameSettings 는 전부 UGameUserSettings(엔진 기본 클래스) / UGoSoccerGameUserSettings 로 이전되어 더 이상 쓰지 않는다.
// - MaxFPS					-> UGameUserSettings::GetFrameRateLimit / SetFrameRateLimit
// - ShadowMaxResolution	-> UGameUserSettings::GetShadowQuality / SetShadowQuality (Graphic_Shadow 와 같은 값을 공유하도록 통합)
// - AAType					-> UGameUserSettings::GetAntiAliasingQuality / SetAntiAliasingQuality
// - Graphic_*				-> UGameUserSettings::Get/Set*Quality
// - ScreenResolution		-> UGameUserSettings::GetScreenResolution / SetScreenResolution
// - WindowMode				-> UGameUserSettings::GetFullscreenMode / SetFullscreenMode
// - MasterVolume/BGMVolume/FXVolume -> UGoSoccerGameUserSettings::Volume_Master/Volume_BGM/Volume_SFX
// - bUseLowTexture			-> 실제로 쓰이지 않아(적용부가 전부 주석 처리됨) 이전하지 않았다.
//USTRUCT(BlueprintType)
//struct FGameSettings
//{
//	GENERATED_BODY()
//
//public:
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	float MaxFPS = 144.f;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	bool bUseLowTexture = false;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	int32 ShadowMaxResolution = 4;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	int32 AAType = 0;
//
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	int32 Graphic_Shadow = 4;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	int32 Graphic_GlobalIllumination = 4;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	int32 Graphic_Reflection = 4;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	int32 Graphic_PostProcess = 4;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	int32 Graphic_Texture = 4;
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	int32 Graphic_Effect = 4;
//
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	FIntPoint ScreenResolution = FIntPoint(1920.f, 1080.f);
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
//	int32 WindowMode = 2;
//};

USTRUCT(BlueprintType)
struct FPlayerCard
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UObject* OwningPlayerControllerObject;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName PlayerName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTexture2D* PlayerAvatar;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UImage> PlayerIcon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 PlayerDollColor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bGameReady;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsMyTurn;
};

class GOSOCCER_API UGoSoccer_PCH
{
	//GENERATED_BODY()

};
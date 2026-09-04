#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GoSoccer_PCH.h"
#include "Widget/SelectButtonWidget.h"
#include "GameConfigSettingWidget.generated.h"

class UCheckButtonWidget;
class UCustomButtonWidget;
class UEditableText;
class UWidgetSwitcher;

UCLASS()
class GOSOCCER_API UGameConfigSettingWidget : public UUserWidget
{
	GENERATED_BODY()

	UGameConfigSettingWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_Secret;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_Mode0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_Mode1;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	//TObjectPtr<UCheckButtonWidget> BP_Radio_BoardPreset_None;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	//TObjectPtr<UCheckButtonWidget> BP_Radio_BoardPreset_China;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_TurnMode0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_TurnMode1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_DollMode_F;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_DollMode_A;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_NoItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckButtonWidget> BP_Radio_YesItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Button_BoardPreset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Button_TimeLimit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UEditableText> EditableText_SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Cancle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_Complete;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Button_MatchRule;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Button_AngularThreshold;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	//TObjectPtr<USelectButtonWidget> BP_Button_DistanceThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Slider_DistanceThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Switch_TimeLimit_DollSelect;

	UPROPERTY(BlueprintReadOnly)
	EPlayGameMode PlayGameMode;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> TimeLimitArr;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> AngularThresholdArr;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> DistanceThresholdArr;

public:
	TObjectPtr<UCustomButtonWidget> GetButtonCancle() { return BP_Button_Cancle; };
	TObjectPtr<UCustomButtonWidget> GetButtonComplete() { return BP_Button_Complete; };
	UFUNCTION(BlueprintNativeEvent)
	void SetPlayGameMode(EPlayGameMode InPlayGameMode);
	EPlayGameMode GetPlayGameMode() { return PlayGameMode; };
	FSessionCreateData GetSessionCreateData();

	UFUNCTION()
	void OnCheckSecret();

	UFUNCTION()
	void OnCheckMode0();
	UFUNCTION()
	void OnCheckMode1();

	UFUNCTION()
	void OnCheckTurnMode0();
	UFUNCTION()
	void OnCheckTurnMode1();

	UFUNCTION()
	void OnCheckDoll_F();
	UFUNCTION()
	void OnCheckDoll_A();

	UFUNCTION()
	void OnCheckNoItem();
	UFUNCTION()
	void OnCheckYesItem();

	void ApplySessionCreateData(const FSessionCreateData& InSessionCreateData);
private:
	void OnUpdateTooltip();
	/**
	 * ApplySessionCreateData 로 위젯을 채우는 동안 참.
	 * 라디오 버튼을 하나씩 세팅하면서 매번 툴팁을 뿌리면 절반만 적용된 설정이 나가므로,
	 * 적용이 끝난 뒤 호출부에서 한 번만 뿌리도록 그동안의 갱신을 막는다.
	 */
	bool bApplyingSessionCreateData = false;
};

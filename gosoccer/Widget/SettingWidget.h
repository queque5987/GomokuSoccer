#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget/CustomButtonWidget.h"
#include "SettingWidget.generated.h"

class UButton;
class UComboBoxString;
class UCheckBox;
class USlider;
class UPanelWidget;
class USelectButtonWidget;
class UGameUserSettings;

UCLASS()
class GOSOCCER_API USettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_SaveOut;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_GGQuitSession;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomButtonWidget> BP_Button_GG;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Graphic)
	TObjectPtr<UComboBoxString> ComboBox_ScreenResolution;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Graphic)
	TObjectPtr<UComboBoxString> ComboBox_WindowMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Graphic)
	TObjectPtr<UComboBoxString> ComboBox_MaxFPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Sound)
	TObjectPtr<UComboBoxString> ComboBox_Antialiasing;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Graphic)
	//TObjectPtr<UCheckBox> CheckBox_UseDollVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Graphic)
	TObjectPtr<USlider> Slider_Shadow;

// Graphic Setting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Graphic_Total;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Graphic_Shadow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Graphic_GlobalIllumination;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Graphic_Reflection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Graphic_PostProcess;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Graphic_Texture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USelectButtonWidget> BP_Graphic_Effect;
// Graphic Setting End

// Sound Setting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Graphic)
	TObjectPtr<USelectButtonWidget> BP_Slider_Master;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Graphic)
	TObjectPtr<USelectButtonWidget> BP_Slider_BGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Graphic)
	TObjectPtr<USelectButtonWidget> BP_Slider_SFX;
// Sound Setting End
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> InGameManuPanel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget), Category = Category_Language)
	TObjectPtr<UComboBoxString> ComboBox_Language;

	virtual void NativeConstruct() override;

	TObjectPtr<UCustomButtonWidget> GetButton_SaveOut() { return BP_Button_SaveOut; };
	TObjectPtr<UCustomButtonWidget> GetBP_Button_GGQuitSession() { return BP_Button_GGQuitSession; };
	TObjectPtr<UCustomButtonWidget> GetBP_Button_GG() { return BP_Button_GG; };
	//TObjectPtr<UCheckBox> GetCheckBox_UseDollVFX() { return CheckBox_UseDollVFX; };
	TObjectPtr<USlider> GetSlider_Shadow() { return Slider_Shadow; };

	//TObjectPtr<USlider> GetSlider_Volume_Master() { return Slider_Volume_Master; };
	//TObjectPtr<USlider> GetSlider_Volume_BGM() { return Slider_Volume_BGM; };
	//TObjectPtr<USlider> GetSlider_Volume_FX() { return Slider_Volume_FX; };

	FOnCutomButtonValueChangedDelegate* GetSlider_Volume_Master_ValueChangedDelegate();
	FOnCutomButtonValueChangedDelegate* GetSlider_Volume_BGM_ValueChangedDelegate();
	FOnCutomButtonValueChangedDelegate* GetSlider_Volume_SFX_ValueChangedDelegate();

	FOnCutomButtonValueChangedDelegate* GetSlider_Graphic_Total_ValueChangedDelegate();
	FOnCutomButtonValueChangedDelegate* GetSlider_Graphic_Shadow_ValueChangedDelegate();
	FOnCutomButtonValueChangedDelegate* GetSlider_Graphic_GlobalIllumination_ValueChangedDelegate();
	FOnCutomButtonValueChangedDelegate* GetSlider_Graphic_Reflection_ValueChangedDelegate();
	FOnCutomButtonValueChangedDelegate* GetSlider_Graphic_PostProcess_ValueChangedDelegate();
	FOnCutomButtonValueChangedDelegate* GetSlider_Graphic_Texture_ValueChangedDelegate();
	FOnCutomButtonValueChangedDelegate* GetSlider_Graphic_Effect_ValueChangedDelegate();


	UFUNCTION()
	void OnLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnMaxFPSChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnAAChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnScreenResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	void SetInGameManuPanelVisibility(bool e);

	void ApplyGameSettings(UGameUserSettings* InUserSettings);
private:
	TMap<FString, int32> AAMap;
	TMap<FString, FIntPoint> ResolutionMap;
	TMap<FString, EWindowMode::Type> WindowModeMap;
	TMap<FString, FString> CultureMap;
};

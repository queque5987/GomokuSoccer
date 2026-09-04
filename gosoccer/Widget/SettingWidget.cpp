#include "Widget/SettingWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/PanelWidget.h"
#include "GoSoccerPlayManager.h"
#include "Widget/SelectButtonWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameUserSettings.h"
#include "Interface/IGoSoccerGameUserSettings.h"

void USettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ComboBox_MaxFPS)			ComboBox_MaxFPS->OnSelectionChanged.AddDynamic			(this, &USettingWidget::OnMaxFPSChanged);
	if (ComboBox_Antialiasing)		ComboBox_Antialiasing->OnSelectionChanged.AddDynamic	(this, &USettingWidget::OnAAChanged);
	if (ComboBox_ScreenResolution)	ComboBox_ScreenResolution->OnSelectionChanged.AddDynamic(this, &USettingWidget::OnScreenResolutionChanged);
	if (ComboBox_WindowMode)		ComboBox_WindowMode->OnSelectionChanged.AddDynamic		(this, &USettingWidget::OnWindowModeChanged);
	if (ComboBox_Language)			ComboBox_Language->OnSelectionChanged.AddDynamic		(this, &USettingWidget::OnLanguageChanged);


	AAMap.Add(TEXT("None"), 0);
	AAMap.Add(TEXT("FXAA"), 1);
	AAMap.Add(TEXT("TAA"), 2);
	AAMap.Add(TEXT("MSAA"), 3);
	AAMap.Add(TEXT("TSR"), 4);

	if (ComboBox_Language)
	{
		CultureMap.Add(TEXT("English"), TEXT("en-US"));
		CultureMap.Add(TEXT("한국어"), TEXT("ko"));
	}

	TArray<FIntPoint> SupportedScreenResolutions;
	if (ComboBox_ScreenResolution && UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedScreenResolutions))
	{
		auto Iter = SupportedScreenResolutions.CreateIterator();
		FString IterString;
		for (; Iter; ++Iter)
		{
			IterString = FString::Printf(TEXT("%dx%d"), Iter->X, Iter->Y);
			ComboBox_ScreenResolution->AddOption(IterString);
			ResolutionMap.Add(IterString, *Iter);
		}
		//ComboBox_ScreenResolution->SetSelectedOption(IterString);
	}

	if (ComboBox_WindowMode)
	{
		ComboBox_WindowMode->AddOption(TEXT("Fullscreen"));
		ComboBox_WindowMode->AddOption(TEXT("WindowedFullscreen"));
		ComboBox_WindowMode->AddOption(TEXT("Windowed"));
		WindowModeMap.Add(TEXT("Fullscreen"), EWindowMode::Fullscreen);
		WindowModeMap.Add(TEXT("WindowedFullscreen"), EWindowMode::WindowedFullscreen);
		WindowModeMap.Add(TEXT("Windowed"), EWindowMode::Windowed);
		//ComboBox_WindowMode->SetSelectedIndex(0);
	}
	if (BP_Graphic_Total && BP_Graphic_Total->GetValueChangedDelegate())
	{
		BP_Graphic_Total->GetValueChangedDelegate()->BindLambda([&](float ChangedValue)->void 
			{
				if (BP_Graphic_Shadow)				BP_Graphic_Shadow->SetSelectedValue(ChangedValue);
				if (BP_Graphic_GlobalIllumination)	BP_Graphic_GlobalIllumination->SetSelectedValue(ChangedValue);
				if (BP_Graphic_Reflection)			BP_Graphic_Reflection->SetSelectedValue(ChangedValue);
				if (BP_Graphic_PostProcess)			BP_Graphic_PostProcess->SetSelectedValue(ChangedValue);
				if (BP_Graphic_Texture)				BP_Graphic_Texture->SetSelectedValue(ChangedValue);
				if (BP_Graphic_Effect)				BP_Graphic_Effect->SetSelectedValue(ChangedValue);
			}
		);
	}
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Volume_Master_ValueChangedDelegate()
{
	return BP_Slider_Master ? BP_Slider_Master->GetValueChangedDelegate() : nullptr;
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Volume_BGM_ValueChangedDelegate()
{
	return BP_Slider_BGM ? BP_Slider_BGM->GetValueChangedDelegate() : nullptr;
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Volume_SFX_ValueChangedDelegate()
{
	return BP_Slider_SFX ? BP_Slider_SFX->GetValueChangedDelegate() : nullptr;
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Graphic_Total_ValueChangedDelegate()
{
	return BP_Graphic_Total ? BP_Graphic_Total->GetValueChangedDelegate() : nullptr;
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Graphic_Shadow_ValueChangedDelegate()
{
	return BP_Graphic_Shadow ? BP_Graphic_Shadow->GetValueChangedDelegate() : nullptr;
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Graphic_GlobalIllumination_ValueChangedDelegate()
{
	return BP_Graphic_GlobalIllumination ? BP_Graphic_GlobalIllumination->GetValueChangedDelegate() : nullptr;
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Graphic_Reflection_ValueChangedDelegate()
{
	return BP_Graphic_Reflection ? BP_Graphic_Reflection->GetValueChangedDelegate() : nullptr;
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Graphic_PostProcess_ValueChangedDelegate()
{
	return BP_Graphic_PostProcess ? BP_Graphic_PostProcess->GetValueChangedDelegate() : nullptr;
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Graphic_Texture_ValueChangedDelegate()
{
	return BP_Graphic_Texture ? BP_Graphic_Texture->GetValueChangedDelegate() : nullptr;
}

FOnCutomButtonValueChangedDelegate* USettingWidget::GetSlider_Graphic_Effect_ValueChangedDelegate()
{
	return BP_Graphic_Effect ? BP_Graphic_Effect->GetValueChangedDelegate() : nullptr;
}

void USettingWidget::OnLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	FString* Found = CultureMap.Find(SelectedItem);
	if (Found == nullptr) return;
	UGoSoccerPlayManager::ChangeSettings_Language(GetGameInstance(), *Found);
}

void USettingWidget::OnMaxFPSChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr) return;

#if !UE_BUILD_SHIPPING
	if (GetWorld() != nullptr)
	{
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnMaxFPSChanged : %s"), *SelectedItem);
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, LogString);
		}
	}
#endif
	float NewMaxFPS = FCString::Atof(*SelectedItem);
	UGoSoccerPlayManager::ChangeSettings_MaxFPS(GetGameInstance(), NewMaxFPS);
}

void USettingWidget::OnAAChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr) return;

#if !UE_BUILD_SHIPPING
	if (GetWorld() != nullptr)
	{
		if (GEngine)
		{
			FString LogString = FString::Printf(TEXT("OnAAChanged : %s"), *SelectedItem);
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, LogString);
		}
	}
#endif

	int32* FoundValue = AAMap.Find(SelectedItem);
	
	UGoSoccerPlayManager::ChangeSettings_AA(GetGameInstance(), FoundValue ? *FoundValue : 0);
}

void USettingWidget::OnScreenResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	auto* FindKey = ResolutionMap.Find(SelectedItem);
	if (FindKey != nullptr)
	{
		UGoSoccerPlayManager::ChangeSettings_ScreenResolution(GetGameInstance(), *FindKey);
	}
}

void USettingWidget::OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	auto* FindKey = WindowModeMap.Find(SelectedItem);
	if (FindKey != nullptr)
	{
		UGoSoccerPlayManager::ChangeSettings_WindowMode(GetGameInstance(), *FindKey);
	}
}

void USettingWidget::SetInGameManuPanelVisibility(bool e)
{
	if (InGameManuPanel)
	{
		InGameManuPanel->SetVisibility(e ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void USettingWidget::ApplyGameSettings(UGameUserSettings* InUserSettings)
{
	if (InUserSettings == nullptr) return;

	float Volume_Master = 1.f;
	float Volume_BGM = 1.f;
	float Volume_SFX = 1.f;
	ELanguageSetting LanguageSetting = ELanguageSetting::ELS_Default;

	IIGoSoccerGameUserSettings::Execute_GetVolume_Master(InUserSettings, Volume_Master);
	IIGoSoccerGameUserSettings::Execute_GetVolume_BGM(InUserSettings, Volume_BGM);
	IIGoSoccerGameUserSettings::Execute_GetVolume_SFX(InUserSettings, Volume_SFX);
	IIGoSoccerGameUserSettings::Execute_GetLanguageSetting(InUserSettings, LanguageSetting);

	if (BP_Slider_Master)	BP_Slider_Master->SetSelectedValue(FMath::Max(Volume_Master * 100.f, 0.f));
	if (BP_Slider_SFX)		BP_Slider_SFX->SetSelectedValue		(FMath::Max(Volume_SFX * 100.f, 0.f));
	if (BP_Slider_BGM)		BP_Slider_BGM->SetSelectedValue		(FMath::Max(Volume_BGM * 100.f, 0.f));
	if (ComboBox_Language)	ComboBox_Language->SetSelectedIndex(FMath::Max(static_cast<int32>(LanguageSetting) - 1, 0));

	ComboBox_WindowMode->SetSelectedIndex(static_cast<int32>(InUserSettings->GetFullscreenMode()));
	//UGoSoccerPlayManager::ChangeSettings_WindowMode(GetGameInstance(), InUserSettings->GetFullscreenMode());

	const FIntPoint CurrentResolution = InUserSettings->GetScreenResolution();
	ComboBox_ScreenResolution->SetSelectedOption(FString::Printf(TEXT("%dx%d"), CurrentResolution.X, CurrentResolution.Y));
	//UGoSoccerPlayManager::ChangeSettings_ScreenResolution(GetGameInstance(), CurrentResolution);

	const int32 Graphic_Shadow				= InUserSettings->GetShadowQuality();
	const int32 Graphic_GlobalIllumination	= InUserSettings->GetGlobalIlluminationQuality();
	const int32 Graphic_Reflection			= InUserSettings->GetReflectionQuality();
	const int32 Graphic_PostProcess			= InUserSettings->GetPostProcessingQuality();
	const int32 Graphic_Texture				= InUserSettings->GetTextureQuality();
	const int32 Graphic_Effect				= InUserSettings->GetVisualEffectQuality();

	float Avg_G_Setting = (Graphic_Shadow +
		Graphic_GlobalIllumination +
		Graphic_Reflection +
		Graphic_PostProcess +
		Graphic_Texture +
		Graphic_Effect) / 6.f;
	BP_Graphic_Total->SetSelectedValue(Avg_G_Setting);

	BP_Graphic_Shadow->SetSelectedValue				(Graphic_Shadow);
	BP_Graphic_GlobalIllumination->SetSelectedValue	(Graphic_GlobalIllumination);
	BP_Graphic_Reflection->SetSelectedValue			(Graphic_Reflection);
	BP_Graphic_PostProcess->SetSelectedValue		(Graphic_PostProcess);
	BP_Graphic_Texture->SetSelectedValue			(Graphic_Texture);
	BP_Graphic_Effect->SetSelectedValue				(Graphic_Effect);

	ComboBox_Antialiasing->SetSelectedIndex(InUserSettings->GetAntiAliasingQuality());

#if UE_EDITOR
	UE_LOG(LogTemp, Log, TEXT("===== SettingWidget ApplyGameSettings ====="));
	UE_LOG(LogTemp, Log, TEXT("Graphic_Shadow : %d"), Graphic_Shadow);
	UE_LOG(LogTemp, Log, TEXT("Graphic_GlobalIllumination : %d"), Graphic_GlobalIllumination);
	UE_LOG(LogTemp, Log, TEXT("Graphic_Reflection : %d"), Graphic_Reflection);
	UE_LOG(LogTemp, Log, TEXT("Graphic_PostProcess : %d"), Graphic_PostProcess);
	UE_LOG(LogTemp, Log, TEXT("Graphic_Texture : %d"), Graphic_Texture);
	UE_LOG(LogTemp, Log, TEXT("Graphic_Effect : %d"), Graphic_Effect);
	UE_LOG(LogTemp, Log, TEXT("AAType : %d"), InUserSettings->GetAntiAliasingQuality());
	UE_LOG(LogTemp, Log, TEXT("===== SettingWidget ApplyGameSettings End ====="));
#endif
}

#include "Widget/GameConfigSettingWidget.h"
#include "Widget/CheckButtonWidget.h"
#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/WidgetSwitcher.h"
#include "GoSoccerPlayManager.h"

void UGameConfigSettingWidget::SetPlayGameMode_Implementation(EPlayGameMode InPlayGameMode)
{
	PlayGameMode = InPlayGameMode;

	if (InPlayGameMode == EPlayGameMode::EPGM_HotSeatGame)
	{
#if UE_EDITOR
		BP_Radio_TurnMode1->SetIsEnabled(true);
		BP_Radio_YesItem->SetIsEnabled(true);
#else
		BP_Radio_TurnMode1->SetIsEnabled(false);
		BP_Radio_YesItem->SetIsEnabled(false);
		OnCheckTurnMode0();
		//BP_Radio_TurnMode0->GetCheckBox()->SetCheckedState(ECheckBoxState::Checked);
		//BP_Radio_TurnMode1->GetCheckBox()->SetCheckedState(ECheckBoxState::Unchecked);
#endif
	}
	else if (InPlayGameMode == EPlayGameMode::EPGM_OnlineSession)
	{
#if UE_EDITOR
		BP_Radio_TurnMode1->SetIsEnabled(true);
#else
		BP_Radio_TurnMode1->SetIsEnabled(false);
		BP_Radio_YesItem->SetIsEnabled(false);
		OnCheckTurnMode0();
		//BP_Radio_TurnMode0->GetCheckBox()->SetCheckedState(ECheckBoxState::Unchecked);
		//BP_Radio_TurnMode1->GetCheckBox()->SetCheckedState(ECheckBoxState::Checked);
#endif
	}
	else if (InPlayGameMode == EPlayGameMode::EPGM_SingleAIPlay)
	{
		BP_Radio_TurnMode1->SetIsEnabled(false);
		BP_Radio_YesItem->SetIsEnabled(false);
		OnCheckTurnMode0();
		//BP_Radio_TurnMode0->GetCheckBox()->SetCheckedState(ECheckBoxState::Checked);
		//BP_Radio_TurnMode1->GetCheckBox()->SetCheckedState(ECheckBoxState::Unchecked);
	}
	else
	{
		BP_Radio_TurnMode1->SetIsEnabled(false);
		BP_Radio_YesItem->SetIsEnabled(false);
		OnCheckTurnMode0();
		//BP_Radio_TurnMode0->GetCheckBox()->SetCheckedState(ECheckBoxState::Checked);
		//BP_Radio_TurnMode1->GetCheckBox()->SetCheckedState(ECheckBoxState::Unchecked);
	}
	OnUpdateTooltip();
}

UGameConfigSettingWidget::UGameConfigSettingWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TimeLimitArr.Empty();
	TimeLimitArr.Add(90.f);
	TimeLimitArr.Add(180.f);
	TimeLimitArr.Add(300.f);
	TimeLimitArr.Add(957.f);
	TimeLimitArr.Add(5.f);
	TimeLimitArr.Add(30.f);

	AngularThresholdArr.Add(PI / 5.f);
	AngularThresholdArr.Add(PI / 4.f);
	AngularThresholdArr.Add(PI / 3.f);
	AngularThresholdArr.Add(PI * 2.f);

	DistanceThresholdArr.Add(22.5f);
	DistanceThresholdArr.Add(33.75f);
	DistanceThresholdArr.Add(45.f);
}

void UGameConfigSettingWidget::NativeConstruct()
{
	if (BP_Radio_Secret && BP_Radio_Secret->GetButton_BG())			BP_Radio_Secret->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckSecret);

	if (BP_Radio_Mode0 && BP_Radio_Mode0->GetButton_BG())			BP_Radio_Mode0->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckMode0);
	if (BP_Radio_Mode1 && BP_Radio_Mode1->GetButton_BG())			BP_Radio_Mode1->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckMode1);

	if (BP_Radio_DollMode_F && BP_Radio_DollMode_F->GetButton_BG())	BP_Radio_DollMode_F->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckDoll_F);
	if (BP_Radio_DollMode_A && BP_Radio_DollMode_A->GetButton_BG())	BP_Radio_DollMode_A->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckDoll_A);

	if (BP_Radio_TurnMode0 && BP_Radio_TurnMode0->GetButton_BG())	BP_Radio_TurnMode0->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckTurnMode0);
	if (BP_Radio_TurnMode1 && BP_Radio_TurnMode1->GetButton_BG())	BP_Radio_TurnMode1->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckTurnMode1);

	if (BP_Radio_NoItem && BP_Radio_NoItem->GetButton_BG())			BP_Radio_NoItem->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckNoItem);
	if (BP_Radio_YesItem && BP_Radio_YesItem->GetButton_BG())		BP_Radio_YesItem->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckYesItem);
	
	
	//if (BP_Radio_BoardPreset_None && BP_Radio_BoardPreset_None->GetButton_BG())		BP_Radio_BoardPreset_None->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckPreset_None);
	//if (BP_Radio_BoardPreset_China && BP_Radio_BoardPreset_China->GetButton_BG())	BP_Radio_BoardPreset_China->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckPreset_China);
	//if (BP_Radio_BoardPreset_Korea && BP_Radio_BoardPreset_Korea->GetButton_BG())	BP_Radio_BoardPreset_Korea->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckPreset_Korea);
	//if (BP_Radio_BoardPreset_Tibet && BP_Radio_BoardPreset_Tibet->GetButton_BG())	BP_Radio_BoardPreset_Tibet->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckPreset_Tibet);

	//if (BP_Radio_Doll_B && BP_Radio_Doll_B->GetButton_BG()) BP_Radio_Doll_B->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckDoll_B);
	//if (BP_Radio_Doll_W && BP_Radio_Doll_W->GetButton_BG()) BP_Radio_Doll_W->GetButton_BG()->OnReleased.AddDynamic(this, &UGameConfigSettingWidget::OnCheckDoll_W);
}

FSessionCreateData UGameConfigSettingWidget::GetSessionCreateData()
{
	FSessionCreateData SessionCreateData;
	if (EditableText_SessionName) SessionCreateData.SessionName = EditableText_SessionName->GetText().ToString();
	SessionCreateData.PlayGameMode = PlayGameMode;
	if (BP_Radio_Mode0 && BP_Radio_Mode1)
	{
		if (BP_Radio_Mode0->GetCheckBox()->IsChecked() && !BP_Radio_Mode1->GetCheckBox()->IsChecked())
		{
			SessionCreateData.PlayGameRule = EPlayGameRule::EPGR_5or5Mode;
		}
		else if (!BP_Radio_Mode0->GetCheckBox()->IsChecked() && BP_Radio_Mode1->GetCheckBox()->IsChecked())
		{
			SessionCreateData.PlayGameRule = EPlayGameRule::EPGR_5plus5Mode;
		}
	}
	if (BP_Radio_DollMode_A && BP_Radio_DollMode_F)
	{
		if (BP_Radio_DollMode_F->GetCheckBox()->IsChecked() && !BP_Radio_DollMode_A->GetCheckBox()->IsChecked())
		{
			SessionCreateData.PlayDollType = EPlayDollType::EPDT_Fragile;
		}
		else if (!BP_Radio_DollMode_F->GetCheckBox()->IsChecked() && BP_Radio_DollMode_A->GetCheckBox()->IsChecked())
		{
			SessionCreateData.PlayDollType = EPlayDollType::EPDT_AntiFragile;
		}
	}
	if (BP_Radio_TurnMode0 && BP_Radio_TurnMode1)
	{
		if (BP_Radio_TurnMode0->GetCheckBox()->IsChecked() && !BP_Radio_TurnMode1->GetCheckBox()->IsChecked())
		{
			SessionCreateData.PlayTurnRule = EPlayTurnRule::EPTR_TakeTurn;
		}
		else if (!BP_Radio_TurnMode0->GetCheckBox()->IsChecked() && BP_Radio_TurnMode1->GetCheckBox()->IsChecked())
		{
			SessionCreateData.PlayTurnRule = EPlayTurnRule::EPTR_RealTime;
		}
	}
	if (BP_Button_BoardPreset)
	{
		int32 SelectedBoardPreset = BP_Button_BoardPreset->GetSelectedIndex();
		switch (SelectedBoardPreset)
		{
		case(0):
			SessionCreateData.BoardPreset = EBoardPreset::EBP_Free;
			break;
		case(1):
			SessionCreateData.BoardPreset = EBoardPreset::EBP_China;
			break;
		case(2):
			SessionCreateData.BoardPreset = EBoardPreset::EBP_Korea;
			break;
		case(3):
			SessionCreateData.BoardPreset = EBoardPreset::EBP_Tibet;
			break;
		default:
			break;
		}

	}
	if (BP_Button_TimeLimit && TimeLimitArr.IsValidIndex(BP_Button_TimeLimit->GetSelectedIndex()))
	{
		SessionCreateData.TimeLimit = TimeLimitArr[BP_Button_TimeLimit->GetSelectedIndex()];
	}

	if (BP_Button_AngularThreshold && AngularThresholdArr.IsValidIndex(BP_Button_AngularThreshold->GetSelectedIndex()))
	{
		SessionCreateData.AngularThreshold = AngularThresholdArr[BP_Button_AngularThreshold->GetSelectedIndex()];
	}
	//if (BP_Button_DistanceThreshold && DistanceThresholdArr.IsValidIndex(BP_Button_DistanceThreshold->GetSelectedIndex()))
	//{
	//	SessionCreateData.DistanceThreshold = DistanceThresholdArr[BP_Button_DistanceThreshold->GetSelectedIndex()];
	//}
	if (BP_Slider_DistanceThreshold)
	{
		SessionCreateData.DistanceThreshold = BP_Slider_DistanceThreshold->GetSelectedValue();
	}
	if (BP_Radio_NoItem && BP_Radio_YesItem && BP_Radio_YesItem->GetCheckBox())
	{
		SessionCreateData.bEnableItem = BP_Radio_YesItem->GetCheckBox()->IsChecked();
	}
	if (BP_Radio_Secret && BP_Radio_Secret->GetCheckBox())
	{
		SessionCreateData.IsSecret = BP_Radio_Secret->GetCheckBox()->IsChecked();
	}
	//if (BP_Radio_Doll_B && BP_Radio_Doll_W)
// 
	//{
	//	if (BP_Radio_Doll_B->GetCheckBox()->IsChecked() && !BP_Radio_Doll_W->GetCheckBox()->IsChecked())
	//	{
	//		SessionCreateData.PlayAsDollColor = 0;
	//	}
	//	else if (!BP_Radio_Doll_B->GetCheckBox()->IsChecked() && BP_Radio_Doll_W->GetCheckBox()->IsChecked())
	//	{
	//		SessionCreateData.PlayAsDollColor = 1;
	//	}
	//	else
	//	{
	//		SessionCreateData.PlayAsDollColor = -1;
	//	}
	//}
	if (BP_Button_MatchRule)
	{
		SessionCreateData.MatchRule = static_cast<EMatchRule>(BP_Button_MatchRule->GetSelectedIndex() + 1);
	}

	return SessionCreateData;
}

void UGameConfigSettingWidget::OnCheckSecret()
{
	if (BP_Radio_Secret && BP_Radio_Secret->GetCheckBox())
	{
		BP_Radio_Secret->GetCheckBox()->SetIsChecked(BP_Radio_Secret->GetCheckBox()->CheckedState == ECheckBoxState::Checked ? false : true);
	}
}

void UGameConfigSettingWidget::OnCheckMode0()
{
	if (BP_Radio_Mode0 && BP_Radio_Mode1 && BP_Radio_Mode0->GetCheckBox() && BP_Radio_Mode1->GetCheckBox())
	{
		BP_Radio_Mode0->GetCheckBox()->SetIsChecked(true);
		BP_Radio_Mode1->GetCheckBox()->SetIsChecked(false);
	}
}

void UGameConfigSettingWidget::OnCheckMode1()
{
	if (BP_Radio_Mode0 && BP_Radio_Mode1 && BP_Radio_Mode0->GetCheckBox() && BP_Radio_Mode1->GetCheckBox())
	{
		BP_Radio_Mode0->GetCheckBox()->SetIsChecked(false);
		BP_Radio_Mode1->GetCheckBox()->SetIsChecked(true);
	}
}

void UGameConfigSettingWidget::OnCheckTurnMode0()
{
	if (BP_Radio_TurnMode0 && BP_Radio_TurnMode1 && BP_Radio_TurnMode0->GetCheckBox() && BP_Radio_TurnMode0->GetCheckBox())
	{
		BP_Radio_TurnMode0->GetCheckBox()->SetIsChecked(true);
		BP_Radio_TurnMode1->GetCheckBox()->SetIsChecked(false);
		if (BP_Radio_Mode0 && BP_Radio_Mode1 && BP_Radio_Mode0->GetCheckBox() && BP_Radio_Mode1->GetCheckBox())
		{
			BP_Radio_Mode0->SetIsEnabled(true);
			//BP_Radio_Mode0->GetCheckBox()->SetIsChecked(false);
			BP_Radio_Mode1->SetIsEnabled(true);
			//BP_Radio_Mode1->GetCheckBox()->SetIsChecked(true);
		}
		if (Switch_TimeLimit_DollSelect)
		{
			Switch_TimeLimit_DollSelect->SetActiveWidgetIndex(0);
		}
		OnUpdateTooltip();
	}
}

void UGameConfigSettingWidget::OnCheckTurnMode1()
{
	if (BP_Radio_TurnMode0 && BP_Radio_TurnMode1 && BP_Radio_TurnMode0->GetCheckBox() && BP_Radio_TurnMode1->GetCheckBox())
	{
		BP_Radio_TurnMode0->GetCheckBox()->SetIsChecked(false);
		BP_Radio_TurnMode1->GetCheckBox()->SetIsChecked(true);
		if (BP_Radio_Mode0 && BP_Radio_Mode1 && BP_Radio_Mode0->GetCheckBox() && BP_Radio_Mode1->GetCheckBox())
		{
			BP_Radio_Mode0->SetIsEnabled(true);
			BP_Radio_Mode0->GetCheckBox()->SetIsChecked(true);
			BP_Radio_Mode1->SetIsEnabled(false);
			BP_Radio_Mode1->GetCheckBox()->SetIsChecked(false);
		}
		if (Switch_TimeLimit_DollSelect)
		{
			Switch_TimeLimit_DollSelect->SetActiveWidgetIndex(1);
		}
		OnUpdateTooltip();
	}
}

void UGameConfigSettingWidget::OnCheckDoll_F()
{
	if (BP_Radio_DollMode_F && BP_Radio_DollMode_A && BP_Radio_DollMode_F->GetCheckBox() && BP_Radio_DollMode_A->GetCheckBox())
	{
		BP_Radio_DollMode_F->GetCheckBox()->SetIsChecked(true);
		BP_Radio_DollMode_A->GetCheckBox()->SetIsChecked(false);
	}
}

void UGameConfigSettingWidget::OnCheckDoll_A()
{
	if (BP_Radio_DollMode_F && BP_Radio_DollMode_A && BP_Radio_DollMode_F->GetCheckBox() && BP_Radio_DollMode_A->GetCheckBox())
	{
		BP_Radio_DollMode_F->GetCheckBox()->SetIsChecked(false);
		BP_Radio_DollMode_A->GetCheckBox()->SetIsChecked(true);
	}
}

void UGameConfigSettingWidget::OnCheckNoItem()
{
	if (BP_Radio_NoItem && BP_Radio_YesItem && BP_Radio_NoItem->GetCheckBox() && BP_Radio_YesItem->GetCheckBox())
	{
		BP_Radio_NoItem->GetCheckBox()->SetIsChecked(true);
		BP_Radio_YesItem->GetCheckBox()->SetIsChecked(false);
	}
}

void UGameConfigSettingWidget::OnCheckYesItem()
{
	if (BP_Radio_NoItem && BP_Radio_YesItem && BP_Radio_NoItem->GetCheckBox() && BP_Radio_YesItem->GetCheckBox())
	{
		BP_Radio_NoItem->GetCheckBox()->SetIsChecked(false);
		BP_Radio_YesItem->GetCheckBox()->SetIsChecked(true);
	}
}

void UGameConfigSettingWidget::ApplySessionCreateData(const FSessionCreateData& InSessionCreateData)
{
	TGuardValue<bool> ApplyGuard(bApplyingSessionCreateData, true);

	if (EditableText_SessionName) EditableText_SessionName->SetText(FText::FromString(InSessionCreateData.SessionName));
	PlayGameMode = InSessionCreateData.PlayGameMode;
	//InSessionCreateData.HostName; Wut
	//InSessionCreateData.PlayGameRule; wuut

	if (BP_Button_AngularThreshold)
	{
		int32 idx;
		if (!AngularThresholdArr.Find(InSessionCreateData.AngularThreshold, idx)) idx = 0;
		BP_Button_AngularThreshold->ChangeSelectedIndex(idx);
	}
	//if (BP_Button_DistanceThreshold)
	//{
	//	int32 idx;
	//	if (!DistanceThresholdArr.Find(InSessionCreateData.DistanceThreshold, idx)) idx = 0;
	//	BP_Button_DistanceThreshold->ChangeSelectedIndex(idx);
	//}
	if (BP_Slider_DistanceThreshold)
	{
		BP_Slider_DistanceThreshold->SetSelectedValue(InSessionCreateData.DistanceThreshold);
	}
	if (BP_Button_TimeLimit)
	{
		int32 idx;
		if (!DistanceThresholdArr.Find(InSessionCreateData.TimeLimit, idx)) idx = 0;
		BP_Button_TimeLimit->ChangeSelectedIndex(idx);
	}
	if (BP_Button_BoardPreset)
	{
		int32 iBoardPreset = static_cast<int32>(InSessionCreateData.BoardPreset);
		if (iBoardPreset > 0)
		{
			BP_Button_BoardPreset->ChangeSelectedIndex(iBoardPreset - 1);
			//BP_Button_BoardPreset->SelectedIndex = iBoardPreset - 1;
		}
	}
	if (BP_Radio_DollMode_A && BP_Radio_DollMode_F)
	{
		bool IsFragile = InSessionCreateData.PlayDollType == EPlayDollType::EPDT_Fragile;
		if (IsFragile) OnCheckDoll_F();
		else OnCheckDoll_A();
		//BP_Radio_DollMode_F->GetCheckBox()->SetCheckedState(IsFragile ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
		//BP_Radio_DollMode_A->GetCheckBox()->SetCheckedState(IsFragile ? ECheckBoxState::Unchecked : ECheckBoxState::Checked);
	}
	if (BP_Radio_Mode0 && BP_Radio_Mode1)
	{
		bool Is5or5 = InSessionCreateData.PlayGameRule == EPlayGameRule::EPGR_5or5Mode;
		if (Is5or5) OnCheckMode0();
		else OnCheckMode1();
		//BP_Radio_Mode0->GetCheckBox()->SetIsChecked(Is5or5);
		//BP_Radio_Mode1->GetCheckBox()->SetIsChecked(!Is5or5);
	}
	if (BP_Radio_TurnMode0 && BP_Radio_TurnMode1)
	{
		bool IsTakrTurn = InSessionCreateData.PlayTurnRule == EPlayTurnRule::EPTR_TakeTurn;
		if (IsTakrTurn) OnCheckTurnMode0();
		else OnCheckTurnMode1();
		//BP_Radio_TurnMode0->GetCheckBox()->SetIsChecked(IsTakrTurn);
		//BP_Radio_TurnMode1->GetCheckBox()->SetIsChecked(!IsTakrTurn);
	}
	if (BP_Radio_NoItem && BP_Radio_YesItem)
	{
		if (InSessionCreateData.bEnableItem) OnCheckYesItem();
		else OnCheckNoItem();
	}
	if (BP_Button_MatchRule)
	{
		int32 idx = static_cast<int32>(InSessionCreateData.MatchRule);
		BP_Button_MatchRule->ChangeSelectedIndex(FMath::Max(idx - 1, 0));
	}
	if (BP_Radio_Secret)
	{
		BP_Radio_Secret->GetCheckBox()->SetIsChecked(InSessionCreateData.IsSecret);
	}
}

void UGameConfigSettingWidget::OnUpdateTooltip()
{
	// 로컬 툴팁만 바꾸면 설정을 만지는 호스트 화면에서만 반영된다.
	// PlayerController -> Server -> GameMode 를 거쳐 모든 플레이어의 툴팁을 갱신한다.
	if (bApplyingSessionCreateData) return;

	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr && GetWorld() != nullptr) PC = GetWorld()->GetFirstPlayerController();
	if (PC == nullptr) return;
	UGoSoccerPlayManager::Request_UpdateTooltip_SessionConfig_ToAll(PC, GetSessionCreateData());
}

//void UGameConfigSettingWidget::OnCheckDoll_B()
//{
//	if (BP_Radio_Doll_B && BP_Radio_Doll_W && BP_Radio_Doll_B->GetCheckBox() && BP_Radio_Doll_W->GetCheckBox())
//	{
//		BP_Radio_Doll_B->GetCheckBox()->SetIsChecked(true);
//		BP_Radio_Doll_W->GetCheckBox()->SetIsChecked(false);
//	}
//}
//
//void UGameConfigSettingWidget::OnCheckDoll_W()
//{
//	if (BP_Radio_Doll_B && BP_Radio_Doll_W && BP_Radio_Doll_B->GetCheckBox() && BP_Radio_Doll_W->GetCheckBox())
//	{
//		BP_Radio_Doll_B->GetCheckBox()->SetIsChecked(false);
//		BP_Radio_Doll_W->GetCheckBox()->SetIsChecked(true);
//	}
//}

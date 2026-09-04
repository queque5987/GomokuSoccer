#include "Widget/SliderButtonWidget.h"
#include "Components/Slider.h"

float USliderButtonWidget::GetSelectedValue()
{
    if (Slider)
    {
        return Slider->GetValue();
    }
    return 0.0f;
}

void USliderButtonWidget::SetSelectedValue(float InValue)
{
    Slider->SetValue(InValue);
    OnSelectedValueChanged(InValue);
    Execute_ValueChangedDelegate(InValue);
}

void USliderButtonWidget::Execute_ValueChangedDelegate(float InChangedValue)
{
#if UE_EDITOR
    UE_LOG(LogTemp, Log, TEXT("Execute_ValueChangedDelegate : %f"), InChangedValue);
#endif
    Delegate_OnValueChanged.ExecuteIfBound(InChangedValue);
}

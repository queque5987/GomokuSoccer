#include "Widget/SelectButtonWidget.h"

void USelectButtonWidget::Execute_IndexChangedDelegate()
{
	Delegate_OnIndexChanged.ExecuteIfBound(static_cast<float>(GetSelectedIndex()));
}

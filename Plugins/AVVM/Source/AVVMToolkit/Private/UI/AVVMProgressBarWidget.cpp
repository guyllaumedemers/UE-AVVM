//Copyright(c) 2025 gdemers
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.
#include "UI/AVVMProgressBarWidget.h"

#include "CommonLazyImage.h"
#include "Materials/MaterialInstanceDynamic.h"

void UAVVMProgressBarWidget::SetValue(const float NewValue)
{
	if (IsValid(DynamicMaterial))
	{
		DynamicMaterial->SetScalarParameterValue(MaterialParameterName, NewValue);
	}
}

void UAVVMProgressBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!IsValid(MaterialImage) || MaterialInstance.IsNull())
	{
		return;
	}

	FOnLoadGuardStateChangedEvent Delegate = MaterialImage->OnLoadingStateChanged();
	if (!Delegate.IsBoundToObject(this))
	{
		MaterialImage->OnLoadingStateChanged().AddUObject(this, &UAVVMProgressBarWidget::OnLoadingStateChanged);
	}

	if (!MaterialInstance.IsValid())
	{
		MaterialImage->SetBrushFromLazyMaterial(MaterialInstance);
	}

	OnLoadingStateChanged(MaterialImage->IsLoading());
}

void UAVVMProgressBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(MaterialImage))
	{
		OnLoadingStateChanged(MaterialImage->IsLoading());
	}
}

void UAVVMProgressBarWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (IsValid(MaterialImage))
	{
		MaterialImage->OnLoadingStateChanged().RemoveAll(this);
	}
}

void UAVVMProgressBarWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

#if WITH_EDITORONLY_DATA
	SetValue(PreviewValue);
#endif
}

void UAVVMProgressBarWidget::OnLoadingStateChanged(bool bIsLoading)
{
	if (bIsLoading)
	{
		return;
	}

	if (IsValid(MaterialImage))
	{
		DynamicMaterial = MaterialImage->GetDynamicMaterial();
	}

	BP_PlayOnConstruct();
}

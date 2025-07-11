﻿//Copyright(c) 2025 gdemers
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
#include "Widgets/AVVMDynamicEntryBoxExtendedWidget.h"

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "CommonUserWidget.h"
#include "Components/DynamicEntryBox.h"
#include "Engine/AssetManager.h"
#include "Widgets/AVVMWidgetPickerDataAsset.h"

void UAVVMDynamicEntryBoxExtendedWidget::SetupEntries(TArray<UObject*> NewViewModels)
{
	if (!ensureAlwaysMsgf(HasWidgetClass(), TEXT("Invalid Widget Class!")))
	{
		return;
	}

	if (!IsValid(DynamicEntryBox))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UDynamicEntryBox& NewDynamicBox,
	                                             const TSubclassOf<UCommonUserWidget>& NewWidgetClass,
	                                             UObject* NewViewModel)
	{
		// @gdemers UDynamicEntryBox::CreateEntry already has a fallback support for NULL WidgetClass 
		auto* WidgetInstance = NewDynamicBox.CreateEntry<UCommonUserWidget>(NewWidgetClass);
		UAVVMUtilityFunctionLibrary::BindViewModel(NewViewModel, WidgetInstance);
	};

	DynamicEntryBox->Reset(true);

	TSubclassOf<UCommonUserWidget> WidgetClass = nullptr;
	if (WidgetPickerDataAsset.IsNull())
	{
		WidgetClass = DynamicEntryBox->GetEntryWidgetClass();
		for (UObject* ViewModel : NewViewModels)
		{
			CreateWidgetAndBindViewModel(*DynamicEntryBox, WidgetClass, ViewModel);
		}
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMDynamicEntryBoxExtendedWidget::SetupEntries, NewViewModels);
		StreamableHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		for (UObject* ViewModel : NewViewModels)
		{
			WidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(ViewModel) ? ViewModel->GetClass() : nullptr);
			CreateWidgetAndBindViewModel(*DynamicEntryBox, WidgetClass, ViewModel);
		}
	}
}

void UAVVMDynamicEntryBoxExtendedWidget::AddEntry(UObject* NewViewModel)
{
	if (!ensureAlwaysMsgf(HasWidgetClass(), TEXT("Invalid Widget Class!")))
	{
		return;
	}

	if (!IsValid(DynamicEntryBox))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UDynamicEntryBox& NewDynamicBox,
	                                             const TSubclassOf<UCommonUserWidget>& NewWidgetClass,
	                                             UObject* NewViewModel)
	{
		// @gdemers UDynamicEntryBox::CreateEntry already has a fallback support for NULL WidgetClass 
		auto* WidgetInstance = NewDynamicBox.CreateEntry<UCommonUserWidget>(NewWidgetClass);
		UAVVMUtilityFunctionLibrary::BindViewModel(NewViewModel, WidgetInstance);
	};

	TSubclassOf<UCommonUserWidget> WidgetClass = nullptr;
	if (WidgetPickerDataAsset.IsNull())
	{
		WidgetClass = DynamicEntryBox->GetEntryWidgetClass();
		CreateWidgetAndBindViewModel(*DynamicEntryBox, WidgetClass, NewViewModel);
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMDynamicEntryBoxExtendedWidget::AddEntry, NewViewModel);
		StreamableHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		WidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
		CreateWidgetAndBindViewModel(*DynamicEntryBox, WidgetClass, NewViewModel);
	}
}

void UAVVMDynamicEntryBoxExtendedWidget::RemoveEntry(UCommonUserWidget* NewWidget)
{
	if (IsValid(DynamicEntryBox))
	{
		DynamicEntryBox->RemoveEntry(NewWidget);
	}
}

void UAVVMDynamicEntryBoxExtendedWidget::NativeDestruct()
{
	Super::NativeDestruct();
	StreamableHandle.Reset();
}

bool UAVVMDynamicEntryBoxExtendedWidget::HasWidgetClass() const
{
	return !WidgetPickerDataAsset.IsNull() ||
			(IsValid(DynamicEntryBox) && IsValid(DynamicEntryBox->GetEntryWidgetClass()));
}

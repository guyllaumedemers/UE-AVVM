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
#include "AVVMToolkit/Public/UI/AVVMStaticMultiContextWindowWidget.h"

#include "AVVMUtilityFunctionLibrary.h"
#include "Components/DynamicEntryBox.h"
#include "Engine/AssetManager.h"
#include "UI/AVVMWidgetPickerDataAsset.h"

void UAVVMStaticMultiContextWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Root))
	{
		Root->Reset(true);
	}
}

void UAVVMStaticMultiContextWindowWidget::SetupWindows_Internal(TArray<UObject*> NewViewModels)
{
	if (!IsValid(Root))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UDynamicEntryBox& NewDynamicBox,
	                                             UObject* NewViewModel,
	                                             const TSubclassOf<UCommonUserWidget>& NewWidgetClass)
	{
		// @gdemers UDynamicEntryBox::CreateEntry already has a fallback support for NULL WidgetClass 
		auto* WidgetInstance = NewDynamicBox.CreateEntry<UCommonUserWidget>(NewWidgetClass);
		UAVVMUtilityFunctionLibrary::BindViewModel(NewViewModel, WidgetInstance);
	};

	Root->Reset(true);

	TSubclassOf<UCommonUserWidget> NewWidgetClass = WidgetClass.Get();
	if (WidgetPickerDataAsset.IsNull())
	{
		for (UObject* NewViewModel : NewViewModels)
		{
			CreateWidgetAndBindViewModel(*Root, NewViewModel, NewWidgetClass);
		}
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMStaticMultiContextWindowWidget::SetupWindows_Internal, NewViewModels);
		StreamableHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		for (UObject* NewViewModel : NewViewModels)
		{
			NewWidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
			CreateWidgetAndBindViewModel(*Root, NewViewModel, NewWidgetClass);
		}
	}
}

void UAVVMStaticMultiContextWindowWidget::AddWindow_Internal(UObject* NewViewModel)
{
	if (!IsValid(Root))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UDynamicEntryBox& NewDynamicBox,
	                                             UObject* NewViewModel,
	                                             const TSubclassOf<UCommonUserWidget>& NewWidgetClass)
	{
		// @gdemers UDynamicEntryBox::CreateEntry already has a fallback support for NULL WidgetClass 
		auto* WidgetInstance = NewDynamicBox.CreateEntry<UCommonUserWidget>(NewWidgetClass);
		UAVVMUtilityFunctionLibrary::BindViewModel(NewViewModel, WidgetInstance);
	};

	TSubclassOf<UCommonUserWidget> NewWidgetClass = WidgetClass.Get();
	if (WidgetPickerDataAsset.IsNull() || bOverrideWidgetPicker)
	{
		CreateWidgetAndBindViewModel(*Root, NewViewModel, NewWidgetClass);
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMStaticMultiContextWindowWidget::AddWindow_Internal, NewViewModel);
		StreamableHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		NewWidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
		CreateWidgetAndBindViewModel(*Root, NewViewModel, NewWidgetClass);
	}
}

void UAVVMStaticMultiContextWindowWidget::RemoveWindow_Internal(UObject* NewViewModel)
{
	if (IsValid(Root))
	{
		const FWindowZOrder* SearchResult = ViewModelToWindowContext.Find(NewViewModel);
		Root->RemoveEntry((SearchResult != nullptr) ? SearchResult->Window.Get() : nullptr);
	}
}

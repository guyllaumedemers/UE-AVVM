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
#include "AVVMToolkit/Public/UI/AVVMStaticFrameWidget.h"

#include "Components/DynamicEntryBox.h"
#include "Engine/AssetManager.h"
#include "UI/AVVMWidgetPickerDataAsset.h"

void UAVVMStaticFrameWidget::NativePreConstruct()
{
	if (IsValid(Root))
	{
		Root->Reset(true);
	}

	Super::NativePreConstruct();
}

void UAVVMStaticFrameWidget::SetupFrames_Internal(TArray<UObject*> NewViewModels)
{
	if (!IsValid(Root))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UAVVMStaticFrameWidget& NewParent,
	                                             UDynamicEntryBox& NewDynamicBox,
	                                             UObject* NewViewModel,
	                                             const TSubclassOf<UAVVMFrameWidget>& NewWidgetClass)
	{
		if (!IsValid(NewWidgetClass)) return;
		auto* WidgetInstance = NewDynamicBox.CreateEntry<UAVVMFrameWidget>(NewWidgetClass);
		NewParent.RegisterChild(NewViewModel, FFrameZOrder(WidgetInstance, NewParent.ZOrder + 1));
	};

	Root->Reset(true);

	TSubclassOf<UAVVMFrameWidget> NewWidgetClass = bOverrideWidgetPicker ? WidgetClass.Get() : nullptr;
	if (bOverrideWidgetPicker || WidgetPickerDataAsset.IsNull())
	{
		for (UObject* NewViewModel : NewViewModels)
		{
			CreateWidgetAndBindViewModel(*this, *Root, NewViewModel, NewWidgetClass);
		}
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMStaticFrameWidget::SetupFrames_Internal, NewViewModels);
		WidgetClassPickerHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		for (UObject* NewViewModel : NewViewModels)
		{
			NewWidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
			CreateWidgetAndBindViewModel(*this, *Root, NewViewModel, NewWidgetClass);
		}
	}
}

void UAVVMStaticFrameWidget::AddFrame_Internal(UObject* NewViewModel)
{
	const auto CreateWidgetAndBindViewModel = [](UAVVMStaticFrameWidget& NewParent,
	                                             UDynamicEntryBox& NewDynamicBox,
	                                             UObject* NewViewModel,
	                                             const TSubclassOf<UAVVMFrameWidget>& NewWidgetClass)
	{
		if (!IsValid(NewWidgetClass)) return;
		auto* WidgetInstance = NewDynamicBox.CreateEntry<UAVVMFrameWidget>(NewWidgetClass);
		NewParent.RegisterChild(NewViewModel, FFrameZOrder(WidgetInstance, NewParent.ZOrder + 1));
	};

	TSubclassOf<UAVVMFrameWidget> NewWidgetClass = bOverrideWidgetPicker ? WidgetClass.Get() : nullptr;
	if (bOverrideWidgetPicker || WidgetPickerDataAsset.IsNull())
	{
		CreateWidgetAndBindViewModel(*this, *Root, NewViewModel, NewWidgetClass);
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMStaticFrameWidget::AddFrame_Internal, NewViewModel);
		WidgetClassPickerHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		NewWidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
		CreateWidgetAndBindViewModel(*this, *Root, NewViewModel, NewWidgetClass);
	}
}

void UAVVMStaticFrameWidget::RemoveFrame_Internal(UObject* NewViewModel)
{
	UnRegisterChild(NewViewModel);
}

void UAVVMStaticFrameWidget::RegisterChild_Internal(UObject* NewViewModel, const FFrameZOrder& NewZOrder)
{
	auto* ChildFrame = NewZOrder.Frame.Get();
	if (IsValid(ChildFrame))
	{
		ChildFrame->SetParent(this, NewViewModel);
	}
}

void UAVVMStaticFrameWidget::UnRegisterChild_Internal(UObject* NewViewModel)
{
	if (IsValid(Root))
	{
		const FFrameZOrder* SearchResult = ViewModelToWindowContext.Find(NewViewModel);
		Root->RemoveEntry((SearchResult != nullptr) ? SearchResult->Frame.Get() : nullptr);
	}
}

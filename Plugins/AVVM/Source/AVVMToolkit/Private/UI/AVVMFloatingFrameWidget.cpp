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
#include "AVVMToolkit/Public/UI/AVVMFloatingFrameWidget.h"

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "Components/CanvasPanel.h"
#include "Engine/AssetManager.h"
#include "UI/AVVMWidgetPickerDataAsset.h"

void UAVVMFloatingFrameWidget::NativePreConstruct()
{
	if (IsValid(Root))
	{
		Root->ClearChildren();
	}

	Super::NativePreConstruct();

#if WITH_EDITORONLY_DATA
	const TArray<UPanelSlot*> Slots = IsValid(Root) ? Root->GetSlots() : TArray<UPanelSlot*>{};
	for (auto MySlot : Slots)
	{
		auto* NewSlot = Cast<UCanvasPanelSlot>(MySlot);
		if (IsValid(NewSlot))
		{
			NewSlot->SetAutoSize(bSizeToContent);
			NewSlot->SetAnchors(OverrideAnchorData.Anchors);
		}
	}
#endif
}

void UAVVMFloatingFrameWidget::SetupFrames_Internal(TArray<UObject*> NewViewModels)
{
	if (!IsValid(Root))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UAVVMFloatingFrameWidget& NewParent,
	                                             UObject* NewViewModel,
	                                             const TSubclassOf<UAVVMFrameWidget>& NewWidgetClass)
	{
		if (!IsValid(NewWidgetClass)) return;
		auto* WidgetInstance = Cast<UAVVMFrameWidget>(UUserWidget::CreateWidgetInstance(NewParent, NewWidgetClass, NAME_None));
		NewParent.RegisterChild(NewViewModel, FFrameZOrder(WidgetInstance, NewParent.ZOrder + 1));
	};

	Root->ClearChildren();

	TSubclassOf<UAVVMFrameWidget> NewWidgetClass = bOverrideWidgetPicker ? WidgetClass.Get() : nullptr;
	if (bOverrideWidgetPicker || WidgetPickerDataAsset.IsNull())
	{
		for (UObject* NewViewModel : NewViewModels)
		{
			CreateWidgetAndBindViewModel(*this, NewViewModel, NewWidgetClass);
		}
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMFloatingFrameWidget::SetupFrames_Internal, NewViewModels);
		WidgetClassPickerHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		for (UObject* NewViewModel : NewViewModels)
		{
			NewWidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
			CreateWidgetAndBindViewModel(*this, NewViewModel, NewWidgetClass);
		}
	}
}

void UAVVMFloatingFrameWidget::AddFrame_Internal(UObject* NewViewModel)
{
	const auto CreateWidgetAndBindViewModel = [](UAVVMFloatingFrameWidget& NewParent,
	                                             UObject* NewViewModel,
	                                             const TSubclassOf<UAVVMFrameWidget>& NewWidgetClass)
	{
		if (!IsValid(NewWidgetClass)) return;
		auto* WidgetInstance = Cast<UAVVMFrameWidget>(UUserWidget::CreateWidgetInstance(NewParent, NewWidgetClass, NAME_None));
		NewParent.RegisterChild(NewViewModel, FFrameZOrder(WidgetInstance, NewParent.ZOrder + 1));
	};

	TSubclassOf<UAVVMFrameWidget> NewWidgetClass = bOverrideWidgetPicker ? WidgetClass.Get() : nullptr;
	if (bOverrideWidgetPicker || WidgetPickerDataAsset.IsNull())
	{
		CreateWidgetAndBindViewModel(*this, NewViewModel, NewWidgetClass);
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMFloatingFrameWidget::AddFrame_Internal, NewViewModel);
		WidgetClassPickerHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		NewWidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
		CreateWidgetAndBindViewModel(*this, NewViewModel, NewWidgetClass);
	}
}

void UAVVMFloatingFrameWidget::RemoveFrame_Internal(UObject* NewViewModel)
{
	UnRegisterChild(NewViewModel);
}

void UAVVMFloatingFrameWidget::RegisterChild_Internal(UObject* NewViewModel, const FFrameZOrder& NewZOrder)
{
	if (!IsValid(Root))
	{
		return;
	}

	auto* ChildFrame = NewZOrder.Frame.Get();
	if (!IsValid(ChildFrame))
	{
		return;
	}

	auto* NewSlot = Cast<UCanvasPanelSlot>(Root->AddChild(ChildFrame));
	if (IsValid(NewSlot))
	{
		// TODO @gdemers child placement have to be resolved here through some external means! (maybe some placement system)
		NewSlot->SetAutoSize(bSizeToContent);
		NewSlot->SetAnchors(OverrideAnchorData.Anchors);
	}

	// @gdemers orders matter here. SetParent handle PanelSlot swap. we expect to be already slotted to a parent.
	ChildFrame->SetParent(this, NewViewModel);
}

void UAVVMFloatingFrameWidget::UnRegisterChild_Internal(UObject* NewViewModel)
{
	if (IsValid(Root))
	{
		const FFrameZOrder* SearchResult = ViewModelToWindowContext.Find(NewViewModel);
		Root->RemoveChild((SearchResult != nullptr) ? SearchResult->Frame.Get() : nullptr);
	}
}

bool UAVVMFloatingFrameWidget::AllowInnerBorders() const
{
	return true;
}

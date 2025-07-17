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
#include "AVVMToolkit/Public/UI/AVVMFrameWidget.h"

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/PanelSlot.h"
#include "Engine/AssetManager.h"
#include "UI/AVVMFrameSettings.h"

#if WITH_EDITORONLY_DATA
#include "UI/AVVMEditorPreviewViewModel.h"
#endif

FFrameZOrder::FFrameZOrder(UAVVMFrameWidget* NewFrame, const int32 NewZOrder)
	: Frame(NewFrame)
	, ZOrder(NewZOrder)
{
}

void UAVVMFrameWidget::SetupFrames(const TArray<UObject*>& NewViewModels)
{
	SetupFrames_Internal(NewViewModels);
}

void UAVVMFrameWidget::AddFrame(UObject* NewViewModel)
{
	AddFrame_Internal(NewViewModel);
}

void UAVVMFrameWidget::RemoveFrame(UObject* NewViewModel)
{
	RemoveFrame_Internal(NewViewModel);
}

void UAVVMFrameWidget::CloseAllFrames()
{
	for (auto Iterator = ViewModelToWindowContext.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveFrame(Iterator.Key().Get());
		Iterator.RemoveCurrent();
	}
}

void UAVVMFrameWidget::SetParent(const UAVVMFrameWidget* NewParent, UObject* NewViewModel)
{
	Parent = NewParent;
	AddBorder(NewViewModel);
}

void UAVVMFrameWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	MakeWidgetClass();
	MakeBorderClass();
}

void UAVVMFrameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WindowDecorators.Reset(WindowDecoratorClasses.Num());
	for (const TSubclassOf<UAVVMFrameDecorator>& WindowDecoratorClass : WindowDecoratorClasses)
	{
		auto* NewDecorator = NewObject<UAVVMFrameDecorator>(this, WindowDecoratorClass);
		WindowDecorators.Add(NewDecorator);
	}
}

void UAVVMFrameWidget::NativeDestruct()
{
	Super::NativeDestruct();
	WidgetClassPickerHandle.Reset();
	WidgetClassHandle.Reset();
	BorderClassHandle.Reset();
	WindowDecorators.Reset();
}

void UAVVMFrameWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	for (auto Iterator = WindowDecorators.CreateIterator(); Iterator; ++Iterator)
	{
		const TObjectPtr<const UAVVMFrameDecorator>& Decorator = *Iterator;
		if (!IsValid(Decorator))
		{
			Iterator.RemoveCurrentSwap();
		}
		else if (Decorator->DoesSupportTick())
		{
			Decorator->Tick(nullptr, InDeltaTime);
		}
	}
}

void UAVVMFrameWidget::MakeWidgetClass()
{
#if WITH_EDITORONLY_DATA
	if (WidgetClass.IsValid())
	{
		PreviewEntries();
		return;
	}
#endif

	if (bOverrideWidgetPicker && !WidgetClass.IsNull() && !WidgetClass.IsValid())
	{
		FStreamableDelegate Callback;
#if WITH_EDITORONLY_DATA
		Callback.BindUObject(this, &UAVVMFrameWidget::MakeWidgetClass);
#endif
		WidgetClassHandle = UAssetManager::Get().LoadAssetList({WidgetClass.ToSoftObjectPath()}, Callback);
	}
}

void UAVVMFrameWidget::MakeBorderClass()
{
#if WITH_EDITORONLY_DATA
	if (BorderWidgetClass.IsValid())
	{
		PreviewBorder();
		return;
	}
#endif

	if (bSupportBorderClass && !BorderWidgetClass.IsNull() && !BorderWidgetClass.IsValid())
	{
		FStreamableDelegate Callback;
#if WITH_EDITORONLY_DATA
		Callback.BindUObject(this, &UAVVMFrameWidget::MakeBorderClass);
#endif
		BorderClassHandle = UAssetManager::Get().LoadAssetList({BorderWidgetClass.ToSoftObjectPath()}, Callback);
	}
}

#if WITH_EDITORONLY_DATA
void UAVVMFrameWidget::PreviewEntries()
{
	if (!IsDesignTime())
	{
		return;
	}

	const bool bHasIdenticalNumEntries = (PreviousNumPreviewEntries == NumPreviewEntries);
	const bool bHasIdenticalClasses = (WidgetClass.IsValid() && IsValid(PreviousWidgetClass) && (WidgetClass->GetClass() == PreviousWidgetClass->GetClass()));

	if (bHasIdenticalNumEntries && bHasIdenticalClasses)
	{
		return;
	}

	PreviousNumPreviewEntries = NumPreviewEntries;
	PreviousWidgetClass = WidgetClass.Get();

	EditorPreviewObjects.Reset(NumPreviewEntries);
	for (int32 i = 0; i < NumPreviewEntries; ++i)
	{
		auto* EditorPreviewObject = NewObject<UAVVMEditorPreviewViewModel>(this);
		EditorPreviewObjects.Add(EditorPreviewObject);
	}

	SetupFrames_Internal(EditorPreviewObjects);
}

void UAVVMFrameWidget::PreviewBorder()
{
	if (!IsDesignTime())
	{
		return;
	}

	const bool bHasIdenticalClasses = (BorderWidgetClass.IsValid() && IsValid(PreviousBorderClass) && (BorderWidgetClass->GetClass() == PreviousBorderClass->GetClass()));
	if (bHasIdenticalClasses)
	{
		return;
	}

	PreviousBorderClass = BorderWidgetClass.Get();

	auto* NewViewModel = NewObject<UAVVMEditorPreviewViewModel>(this);
	AddBorder(NewViewModel);
}
#endif

void UAVVMFrameWidget::RegisterChild(UObject* NewViewModel, const FFrameZOrder& NewZOrder)
{
	FFrameZOrder& SearchResult = ViewModelToWindowContext.FindOrAdd(NewViewModel);
	SearchResult = NewZOrder;
	RegisterChild_Internal(NewViewModel, NewZOrder);
}

void UAVVMFrameWidget::UnRegisterChild(UObject* NewViewModel)
{
	UnRegisterChild_Internal(NewViewModel);
	ViewModelToWindowContext.Remove(NewViewModel);
}

UAVVMFrameBorder* UAVVMFrameWidget::IfCheckCreateBorder()
{
	const UAVVMFrameWidget* NewParent = Parent.Get();
	const bool bDoesFailIfCheck = (!IsValid(NewParent) || !NewParent->AllowInnerBorders() || UAVVMFrameSettings::IsUIBorderless() || (UAVVMFrameBorder::StaticClass() == GetClass()));
	if (!IsDesignTime() && bDoesFailIfCheck)
	{
		return nullptr;
	}

	auto* NewBorder = Cast<UAVVMFrameBorder>(UUserWidget::CreateWidgetInstance(*this, BorderWidgetClass.Get(), NAME_None));
	if (IsValid(NewBorder))
	{
		NewBorder->SetParent(NewParent, nullptr);
	}

	return NewBorder;
}

bool UAVVMFrameWidget::AllowInnerBorders() const
{
	return false;
}

void UAVVMFrameWidget::AddBorder(UObject* NewViewModel)
{
	if (!bSupportBorderClass || BorderWidgetClass.IsNull())
	{
		return;
	}

	if (!BorderWidgetClass.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMFrameWidget::AddBorder, NewViewModel);
		BorderClassHandle = UAssetManager::Get().LoadAssetList({BorderWidgetClass.ToSoftObjectPath()}, Callback);
	}
	else
	{
		UAVVMFrameBorder* NewBorder = IfCheckCreateBorder();
		if (IsValid(NewBorder))
		{
			UAVVMUtilityFunctionLibrary::BindViewModel(NewViewModel, NewBorder);
			NewBorder->SwapSlots(this);
			Parent = NewBorder;
		}

		OwningBorder = NewBorder;
	}
}

void UAVVMFrameBorder::SwapSlots(UAVVMFrameWidget* NewFrame)
{
	// TODO @gdemers Fix this! Doesnt work in Editor and prob not also in gameplay
	if (!IsValid(Slot))
	{
		return;
	}

	const UPanelSlot* NewFrameSlot = IsValid(NewFrame) ? NewFrame->Slot : nullptr;
	if (!IsValid(NewFrameSlot))
	{
		return;
	}

	UPanelWidget* NewFrameParent = NewFrameSlot->Parent;
	if (IsValid(NewFrameParent))
	{
		NewFrameParent->RemoveChild(NewFrameSlot->Content);
		NewFrameParent->AddChild(Slot->Content);
	}

	if (IsValid(Anchor))
	{
		Anchor->ClearChildren();
		Anchor->AddChild(NewFrameSlot->Content);
	}
}

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

#include "AVVMUtils.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
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

void UAVVMFrameWidget::SetParent(UAVVMFrameWidget* NewParent, UObject* NewViewModel)
{
	Parent = NewParent;

	if (IsValid(NewViewModel))
	{
		UAVVMUtils::BindViewModel(NewViewModel, NewParent);
		SafeAddBorder(NewViewModel);
	}
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

bool UAVVMFrameWidget::Initialize()
{
	const bool bResult = Super::Initialize();

	if (bResult)
	{
		MakeWidgetClass();
		MakeBorderClass();
	}

	return bResult;
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
	const bool bHasIdenticalClasses = (WidgetClass.Get() == PreviousWidgetClass.Get());

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

	const bool bHasIdenticalClasses = (BorderWidgetClass.Get() == PreviousBorderClass.Get());
	const bool bIdenticalState = (bSupportBorderClass == bPreviousBorderFlagStatus);
	if (bHasIdenticalClasses && bIdenticalState)
	{
		return;
	}

	bPreviousBorderFlagStatus = bSupportBorderClass;
	PreviousBorderClass = BorderWidgetClass.Get();

	auto* NewViewModel = NewObject<UAVVMEditorPreviewViewModel>(this);
	SafeAddBorder(NewViewModel);
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

	const bool bDoesFailIfCheck = (IsValid(NewParent) && !NewParent->AllowInnerBorders());
	if (!IsValid(WidgetTree) || UAVVMFrameSettings::IsUIBorderless() || (!IsDesignTime() && bDoesFailIfCheck))
	{
		return nullptr;
	}

	auto* NewFrameBorder = WidgetTree->ConstructWidget<UAVVMFrameBorder>(BorderWidgetClass.Get(), NAME_None);
	if (IsValid(NewFrameBorder))
	{
		NewFrameBorder->SwapRoots(this);
	}

	return NewFrameBorder;
}

bool UAVVMFrameWidget::AllowInnerBorders() const
{
	return false;
}

void UAVVMFrameWidget::SafeAddBorder(UObject* NewViewModel)
{
	if (!bSupportBorderClass || BorderWidgetClass.IsNull() || IsA(UAVVMFrameBorder::StaticClass()))
	{
		return;
	}

	if (!BorderWidgetClass.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMFrameWidget::SafeAddBorder, NewViewModel);
		BorderClassHandle = UAssetManager::Get().LoadAssetList({BorderWidgetClass.ToSoftObjectPath()}, Callback);
	}
	else
	{
		if (OwningBorder.IsValid())
		{
			OwningBorder->Revert(this);
		}

		UAVVMFrameBorder* NewBorder = IfCheckCreateBorder();
		if (IsValid(NewBorder))
		{
			NewBorder->SetParent(Parent.Get(), NewViewModel);
			Parent = NewBorder;
		}

		OwningBorder = NewBorder;
	}
}

void UAVVMFrameBorder::SwapRoots(UAVVMFrameWidget* NewFrame)
{
	if (!IsValid(Anchor))
	{
		return;
	}

	UWidgetTree* NewFrameWidgetTree = IsValid(NewFrame) ? NewFrame->WidgetTree.Get() : nullptr;
	if (!IsValid(NewFrameWidgetTree))
	{
		return;
	}

	Anchor->ClearChildren();

	auto* NewSlot = Cast<UOverlaySlot>(Anchor->AddChild(NewFrameWidgetTree->RootWidget));
	if (IsValid(NewSlot))
	{
		NewSlot->SetPadding(FMargin());
		NewSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		NewSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
	}

	NewFrameWidgetTree->RootWidget = WidgetTree->RootWidget;
}

void UAVVMFrameBorder::Revert(UAVVMFrameWidget* NewFrame)
{
	if (!IsValid(Anchor))
	{
		return;
	}

	UWidgetTree* NewFrameWidgetTree = IsValid(NewFrame) ? NewFrame->WidgetTree.Get() : nullptr;
	if (!IsValid(NewFrameWidgetTree))
	{
		return;
	}

	UWidget* OldFrameBorderRoot = NewFrameWidgetTree->RootWidget;
	if (IsValid(OldFrameBorderRoot))
	{
		OldFrameBorderRoot->RemoveFromParent();
	}

	NewFrameWidgetTree->RootWidget = Anchor->GetChildAt(0);
}

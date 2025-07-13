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

#include "Engine/AssetManager.h"

#if WITH_EDITORONLY_DATA
#include "UI/AVVMEditorPreviewViewModel.h"
#endif

FFrameZOrder::FFrameZOrder(UAVVMFrameWidget* NewFrame, const int32 NewZOrder)
	: Frame(NewFrame)
	, ZOrder(NewZOrder)
{
}

void UAVVMFrameWidget::SetupWindows(const TArray<UObject*>& NewViewModels)
{
	SetupWindows_Internal(NewViewModels);
}

void UAVVMFrameWidget::AddWindow(UObject* NewViewModel)
{
	AddWindow_Internal(NewViewModel);
}

void UAVVMFrameWidget::RemoveWindow(UObject* NewViewModel)
{
	RemoveWindow_Internal(NewViewModel);
}

void UAVVMFrameWidget::CloseAllWindows()
{
	for (auto Iterator = ViewModelToWindowContext.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveWindow(Iterator.Key().Get());
		Iterator.RemoveCurrent();
	}
}

void UAVVMFrameWidget::SetParent(const UAVVMFrameWidget* NewParent)
{
	Parent = NewParent;
}

void UAVVMFrameWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

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
		Callback.BindUObject(this, &UAVVMFrameWidget::PreviewEntries);
#endif
		StreamableHandle = UAssetManager::Get().LoadAssetList({WidgetClass.ToSoftObjectPath()}, Callback);
	}
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
	StreamableHandle.Reset();
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

#if WITH_EDITORONLY_DATA
void UAVVMFrameWidget::PreviewEntries()
{
	if (!IsDesignTime())
	{
		return;
	}

	const bool bHasIdenticalNumEntries = (PreviousNumPreviewEntries == NumPreviewEntries);
	const bool bHasIdenticalClasses = (WidgetClass.Get() == PreviousWidgetClass);

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

	SetupWindows_Internal(EditorPreviewObjects);
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

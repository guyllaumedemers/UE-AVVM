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

void UAVVMFloatingFrameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Root))
	{
		Root->ClearChildren();
	}
}

void UAVVMFloatingFrameWidget::SetupWindows_Internal(TArray<UObject*> NewViewModels)
{
	if (!IsValid(Root))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UAVVMFloatingFrameWidget& NewParent,
	                                             UCanvasPanel& NewCanvasPanel,
	                                             UObject* NewViewModel,
	                                             const TSubclassOf<UAVVMFrameWidget>& NewWidgetClass)
	{
		auto* WidgetInstance = Cast<UAVVMFrameWidget>(UUserWidget::CreateWidgetInstance(NewParent, NewWidgetClass, NAME_None));
		NewCanvasPanel.AddChild(WidgetInstance);
		UAVVMUtilityFunctionLibrary::BindViewModel(NewViewModel, WidgetInstance);

		if (IsValid(WidgetInstance))
		{
			WidgetInstance->SetParent(&NewParent);
		}
	};

	Root->ClearChildren();

	TSubclassOf<UAVVMFrameWidget> NewWidgetClass = WidgetClass.Get();
	if (WidgetPickerDataAsset.IsNull())
	{
		for (UObject* NewViewModel : NewViewModels)
		{
			CreateWidgetAndBindViewModel(*this, *Root, NewViewModel, NewWidgetClass);
		}
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMFloatingFrameWidget::SetupWindows_Internal, NewViewModels);
		StreamableHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		for (UObject* NewViewModel : NewViewModels)
		{
			WidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
			CreateWidgetAndBindViewModel(*this, *Root, NewViewModel, NewWidgetClass);
		}
	}
}

void UAVVMFloatingFrameWidget::AddWindow_Internal(UObject* NewViewModel)
{
	if (!IsValid(Root))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UAVVMFloatingFrameWidget& NewParent,
	                                             UCanvasPanel& NewCanvasPanel,
	                                             UObject* NewViewModel,
	                                             const TSubclassOf<UAVVMFrameWidget>& NewWidgetClass)
	{
		auto* WidgetInstance = Cast<UAVVMFrameWidget>(UUserWidget::CreateWidgetInstance(NewParent, NewWidgetClass, NAME_None));
		NewCanvasPanel.AddChild(WidgetInstance);
		UAVVMUtilityFunctionLibrary::BindViewModel(NewViewModel, WidgetInstance);

		if (IsValid(WidgetInstance))
		{
			WidgetInstance->SetParent(&NewParent);
		}
	};

	TSubclassOf<UAVVMFrameWidget> NewWidgetClass = WidgetClass.Get();
	if (WidgetPickerDataAsset.IsNull())
	{
		CreateWidgetAndBindViewModel(*this, *Root, NewViewModel, NewWidgetClass);
	}
	else if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMFloatingFrameWidget::AddWindow_Internal, NewViewModel);
		StreamableHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		NewWidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
		CreateWidgetAndBindViewModel(*this, *Root, NewViewModel, NewWidgetClass);
	}
}

void UAVVMFloatingFrameWidget::RemoveWindow_Internal(UObject* NewViewModel)
{
	if (IsValid(Root))
	{
		const FWindowZOrder* SearchResult = ViewModelToWindowContext.Find(NewViewModel);
		Root->RemoveChild((SearchResult != nullptr) ? SearchResult->Window.Get() : nullptr);
	}
}

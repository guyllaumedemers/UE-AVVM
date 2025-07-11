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
#include "Widgets/AVVMFloatingMultiContextWindowWidget.h"

#include "AVVM.h"
#include "AVVMUtilityFunctionLibrary.h"
#include "Components/CanvasPanel.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Widgets/AVVMWidgetPickerDataAsset.h"

void UAVVMFloatingMultiContextWindowWidget::SetupWindows_Internal(TArray<UObject*> NewViewModels)
{
	if (!ensureAlwaysMsgf(HasWidgetClass(), TEXT("Invalid Widget Class!")))
	{
		return;
	}

	if (!IsValid(FloatingWindowRoot))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UAVVMFloatingMultiContextWindowWidget& NewMultiContextWidget,
	                                             const TSubclassOf<UCommonUserWidget>& NewWidgetClass,
	                                             UCanvasPanel& CanvasPanel,
	                                             UObject* NewViewModel)
	{
		auto* WidgetInstance = Cast<UCommonUserWidget>(UUserWidget::CreateWidgetInstance(NewMultiContextWidget, NewWidgetClass, NAME_None));
		UAVVMUtilityFunctionLibrary::BindViewModel(NewViewModel, WidgetInstance);
		if (IsValid(WidgetInstance))
		{
			CanvasPanel.AddChild(WidgetInstance);
		}
	};

	FloatingWindowRoot->ClearChildren();

	TSubclassOf<UCommonUserWidget> WidgetClass = nullptr;
	if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMFloatingMultiContextWindowWidget::SetupWindows_Internal, NewViewModels);
		StreamableHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		for (UObject* ViewModel : NewViewModels)
		{
			WidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(ViewModel) ? ViewModel->GetClass() : nullptr);
			CreateWidgetAndBindViewModel(*this, WidgetClass, *FloatingWindowRoot, ViewModel);
		}
	}
}

void UAVVMFloatingMultiContextWindowWidget::AddWindow_Internal(UObject* NewViewModel)
{
	if (!ensureAlwaysMsgf(HasWidgetClass(), TEXT("Invalid Widget Class!")))
	{
		return;
	}

	if (!IsValid(FloatingWindowRoot))
	{
		return;
	}

	const auto CreateWidgetAndBindViewModel = [](UAVVMFloatingMultiContextWindowWidget& NewMultiContextWidget,
	                                             const TSubclassOf<UCommonUserWidget>& NewWidgetClass,
	                                             UCanvasPanel& CanvasPanel,
	                                             UObject* NewViewModel)
	{
		auto* WidgetInstance = Cast<UCommonUserWidget>(UUserWidget::CreateWidgetInstance(NewMultiContextWidget, NewWidgetClass, NAME_None));
		UAVVMUtilityFunctionLibrary::BindViewModel(NewViewModel, WidgetInstance);
		if (IsValid(WidgetInstance))
		{
			CanvasPanel.AddChild(WidgetInstance);
		}
	};

	FloatingWindowRoot->ClearChildren();

	TSubclassOf<UCommonUserWidget> WidgetClass = nullptr;
	if (!WidgetPickerDataAsset.IsValid())
	{
		FStreamableDelegate Callback;
		Callback.BindUObject(this, &UAVVMFloatingMultiContextWindowWidget::AddWindow_Internal, NewViewModel);
		StreamableHandle = UAssetManager::Get().LoadAssetList({WidgetPickerDataAsset.ToSoftObjectPath()}, Callback);
	}
	else
	{
		WidgetClass = WidgetPickerDataAsset->GetWidgetClass(IsValid(NewViewModel) ? NewViewModel->GetClass() : nullptr);
		CreateWidgetAndBindViewModel(*this, WidgetClass, *FloatingWindowRoot, NewViewModel);
	}
}

void UAVVMFloatingMultiContextWindowWidget::RemoveWindow_Internal(UObject* NewViewModel)
{
	if (IsValid(FloatingWindowRoot))
	{
		const FWindowZOrder* SearchResult = ViewModelToWindowContext.Find(NewViewModel);
		FloatingWindowRoot->RemoveChild((SearchResult != nullptr) ? SearchResult->Window.Get() : nullptr);
	}
}

bool UAVVMFloatingMultiContextWindowWidget::HasWidgetClass()
{
	return !WidgetPickerDataAsset.IsNull();
}

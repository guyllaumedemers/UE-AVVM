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
#include "Widgets/AVVMMultiContextWindowWidget.h"

void UAVVMMultiContextWindowWidget::SetupWindows(const TArray<UObject*>& NewViewModels)
{
	SetupWindows_Internal(NewViewModels);
}

void UAVVMMultiContextWindowWidget::AddWindow(UObject* NewViewModel)
{
	AddWindow_Internal(NewViewModel);
}

void UAVVMMultiContextWindowWidget::RemoveWindow(UObject* NewViewModel)
{
	RemoveWindow_Internal(NewViewModel);
}

void UAVVMMultiContextWindowWidget::CloseAllWindows()
{
	for (auto Iterator = ViewModelToWindowContext.CreateIterator(); Iterator; ++Iterator)
	{
		RemoveWindow(Iterator.Key().Get());
		Iterator.RemoveCurrent();
	}
}

void UAVVMMultiContextWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WindowDecorators.Reset(WindowDecoratorClasses.Num());
	for (const TSubclassOf<UAVVMWindowDecorator>& WindowDecoratorClass : WindowDecoratorClasses)
	{
		auto* NewDecorator = NewObject<UAVVMWindowDecorator>(this, WindowDecoratorClass);
		WindowDecorators.Add(NewDecorator);
	}
}

void UAVVMMultiContextWindowWidget::NativeDestruct()
{
	Super::NativeDestruct();
	WindowDecorators.Reset();
}

void UAVVMMultiContextWindowWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	for (auto Iterator = WindowDecorators.CreateIterator(); Iterator; ++Iterator)
	{
		const TObjectPtr<UAVVMWindowDecorator>& Decorator = *Iterator;
		if (!IsValid(Decorator))
		{
			Iterator.RemoveCurrentSwap();
		}
		else if (Decorator->DoesSupportTick())
		{
			Decorator->Tick(InDeltaTime);
		}
	}
}

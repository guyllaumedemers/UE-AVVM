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
#include "UI/SingleContextInventoryViewModel.h"

#include "UI/ItemObjectViewModel.h"

USingleContextInventoryViewModel* USingleContextInventoryViewModel::Make(const TArray<UItemObject*>& NewItems,
                                                                         UObject* NewOuter)
{
	USingleContextInventoryViewModel* NewViewModel = nullptr;
	if (!NewItems.IsEmpty())
	{
		NewViewModel = NewObject<USingleContextInventoryViewModel>(NewOuter);
		NewViewModel->Init(NewItems);
	}

	return NewViewModel;
}

void USingleContextInventoryViewModel::Init(const TArray<UItemObject*>& NewItems)
{
	FOnExchangeContextChanged Callback;
	Callback.AddUniqueDynamic(this, &USingleContextInventoryViewModel::OnItemChanged);

	ItemViewModels.Reset(NewItems.Num());
	for (UItemObject* Item : NewItems)
	{
		auto* NewViewModel = UItemObjectViewModel::Make(Item);
		if (IsValid(NewViewModel))
		{
			NewViewModel->OnExchangeContextChanged = Callback;
			ItemViewModels.Add(NewViewModel);
		}
	}

	// TODO @gdemers object placement would only be configured for outer referencing a local player. in the case of a shop displaying
	// content, the current default state would be to display items based on ordering defined in it's data asset. May require Fix later!
	const auto* OwningLocalPlayer = GetTypedOuter<ULocalPlayer>();
}

void USingleContextInventoryViewModel::Process(const UItemObjectViewModel* NewModifiedItem)
{
	// TODO @gdemers Define how the status change of an item affect the inventory state.
}

void USingleContextInventoryViewModel::OnItemChanged(const UItemObjectViewModel* NewModifiedItem)
{
	Process(NewModifiedItem);
}

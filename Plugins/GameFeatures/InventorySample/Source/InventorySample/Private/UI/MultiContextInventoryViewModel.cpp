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
#include "UI/MultiContextInventoryViewModel.h"

#include "ActorInventoryComponent.h"
#include "ItemObject.h"

UItemObjectViewModel* UItemObjectViewModel::Make(UItemObject* NewItem)
{
	UItemObjectViewModel* NewViewModel = nullptr;
	if (IsValid(NewItem))
	{
		NewViewModel = NewObject<UItemObjectViewModel>();
		NewViewModel->Init(NewItem);
	}

	return NewViewModel;
}

void UItemObjectViewModel::Init(UItemObject* NewItem)
{
	if (!IsValid(NewItem))
	{
		return;
	}

	NewItem->OnItemRuntimeStateChanged.AddUniqueDynamic(this, &UItemObjectViewModel::OnItemRuntimeStateChanged);
	NewItem->OnItemRuntimeCountChanged.AddUniqueDynamic(this, &UItemObjectViewModel::OnItemRuntimeCountChanged);
	OnItemRuntimeStateChanged(NewItem->GetRuntimeState());
	OnItemRuntimeCountChanged(NewItem->GetRuntimeCount());
}

void UItemObjectViewModel::OnItemRuntimeStateChanged(const FGameplayTagContainer& NewStateTags)
{
	UE_MVVM_SET_PROPERTY_VALUE(StateTags, NewStateTags);
	OnExchangeContextChanged.Broadcast(this);
}

void UItemObjectViewModel::OnItemRuntimeCountChanged(const int32 NewCounter)
{
	UE_MVVM_SET_PROPERTY_VALUE(Counter, NewCounter);
	OnExchangeContextChanged.Broadcast(this);
}

UExchangeContextViewModel* UExchangeContextViewModel::Make(const TArray<UItemObject*>& NewItems)
{
	UExchangeContextViewModel* NewViewModel = nullptr;
	if (!NewItems.IsEmpty())
	{
		NewViewModel = NewObject<UExchangeContextViewModel>();
		NewViewModel->Init(NewItems);
	}

	return NewViewModel;
}

void UExchangeContextViewModel::Init(const TArray<UItemObject*>& NewItems)
{
	FOnExchangeContextChanged Callback;
	Callback.AddUniqueDynamic(this, &UExchangeContextViewModel::OnItemChanged);

	PendingItemViewModels.Reset();
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
}

void UExchangeContextViewModel::OnItemChanged(const UItemObjectViewModel* NewModifiedItem)
{
	Process(NewModifiedItem);
}

void UMultiContextInventoryViewModel::SetPayload(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload)
{
	const auto* HandshakePayload = NewPayload.GetPtr<FAVVMHandshakePayload>();
	if (HandshakePayload == nullptr)
	{
		return;
	}

	const auto* Instigator = UActorInventoryComponent::GetActorComponent(HandshakePayload->Instigator.Get());
	if (IsValid(Instigator))
	{
		// TODO @gdemers create derive class
		// auto* NewSrc = UExchangeContextViewModel::Make(Instigator->GetItems());
		// UE_MVVM_SET_PROPERTY_VALUE(Src, NewSrc);
	}

	const auto* Target = UActorInventoryComponent::GetActorComponent(HandshakePayload->Target.Get());
	if (IsValid(Target))
	{
		// TODO @gdemers create derive class
		// auto* NewDest = UExchangeContextViewModel::Make(Target->GetItems());
		// UE_MVVM_SET_PROPERTY_VALUE(Dest, NewDest);
	}
}

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

FExchangeContext::FExchangeContext(const FAVVMHandshakePayload* NewPayload)
{
	if (NewPayload == nullptr)
	{
		return;
	}

	// @gdemers world actor interacted with or other player.
	const auto* Instigator = UActorInventoryComponent::GetActorComponent(NewPayload->Instigator);
	if (IsValid(Instigator))
	{
		RemoteItemObjects = Instigator->GetItems();
	}

	// @gdemers self or owned pc.
	const auto* Target = UActorInventoryComponent::GetActorComponent(NewPayload->Target);
	if (IsValid(Target))
	{
		LocalItemObjects = Target->GetItems();
	}
}

bool FExchangeContext::operator==(const FExchangeContext& Rhs) const
{
	return (LocalItemObjects == Rhs.LocalItemObjects)
			&& (RemoteItemObjects == Rhs.RemoteItemObjects);
}

void UMultiContextInventoryViewModel::SetPayload(const TInstancedStruct<FAVVMNotificationPayload>& NewPayload)
{
	UE_MVVM_SET_PROPERTY_VALUE(ExchangeContext,
	                           FExchangeContext(NewPayload.GetPtr<FAVVMHandshakePayload>()));
}

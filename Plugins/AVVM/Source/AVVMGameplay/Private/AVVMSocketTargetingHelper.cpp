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
#include "AVVMSocketTargetingHelper.h"

#include "AVVMGameplay.h"
#include "AVVMGameplayUtils.h"

TInstancedStruct<FAVVMSocketTargetingHelper> FAVVMSocketTargetingHelper::Empty;

UScriptStruct* TBaseStructure<FAVVMSocketTargetingHelper>::Get()
{
	return FAVVMSocketTargetingHelper::StaticStruct();
}

void IAVVMDoesSupportAttachmentNotify::NotifyOnNewSocketAttached(const FGameplayTag& NewItemAttachmentSlotTag, const AActor* NewChild) const
{
	OnNewSocketAttached.Broadcast(NewItemAttachmentSlotTag, NewChild);
}

FDelegateHandle IAVVMDoesSupportAttachmentNotify::OnNewSocketAttachedDelegate_Add(const FOnNewSocketAttachedDelegate::FDelegate& Callback)
{
	return OnNewSocketAttached.Add(Callback);
}

void IAVVMDoesSupportAttachmentNotify::OnNewSocketAttachedDelegate_Remove(const FDelegateHandle& Handle)
{
	OnNewSocketAttached.Remove(Handle);
}

void IAVVMDoesSupportAttachmentNotify::NotifyOnNewSocketDetached(const FGameplayTag& NewItemAttachmentSlotTag) const
{
	OnNewSocketDetached.Broadcast(NewItemAttachmentSlotTag);
}

FDelegateHandle IAVVMDoesSupportAttachmentNotify::OnNewSocketDetachedDelegate_Add(const FOnNewSocketDetachedDelegate::FDelegate& Callback)
{
	return OnNewSocketDetached.Add(Callback);
}

void IAVVMDoesSupportAttachmentNotify::OnNewSocketDetachedDelegate_Remove(const FDelegateHandle& Handle)
{
	OnNewSocketDetached.Remove(Handle);
}

FDelegateHandle IAVVMDoesSupportSocketDeferral::OnNewSocketParentAvailableDelegate_Add(const FOnNewSocketParentAvailableDelegate::FDelegate& Callback)
{
	return OnNewSocketParentAvailable.Add(Callback);
}

void IAVVMDoesSupportSocketDeferral::OnNewSocketParentAvailableDelegate_Remove(const FDelegateHandle& Handle)
{
	OnNewSocketParentAvailable.Remove(Handle);
}

bool FAVVMSocketTargetingHelper::Static_AttachToActorAsync(AActor* Src, const FAVVMSocketTargetingDeferralContextArgs& ContextArgs)
{
	const FGameplayTag& AttachmentSlotTag = ContextArgs.AttachmentSlotTag;
	const FName SocketName = ContextArgs.SocketName;
	AActor* Parent = ContextArgs.Parent.Get();

	if (!IsValid(Src) || !IsValid(Parent))
	{
		return false;
	}

	// @gdemers actor we traverse, and search socket on.
	AActor* SocketTarget = Parent;

	// @gdemers in some cases, this will fail which is expected!
	const bool bDoesImplement = Src->Implements<UAVVMDoesSupportSocketTargeting>();
	if (!bDoesImplement)
	{
		// @gdemers we don't support inner targeting so we must be rooted under the target Pawn.
		Src->AttachToActor(SocketTarget, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
		return true;
	}

	const TInstancedStruct<FAVVMSocketTargetingHelper> SocketHelper = IAVVMDoesSupportSocketTargeting::Execute_GetSocketHelper(Src);

	const auto* Helper = SocketHelper.GetPtr<FAVVMSocketTargetingHelper>();
	if (!ensureAlwaysMsgf(Helper != nullptr, TEXT("Invalid Socket Helper impl.")))
	{
		return false;
	}

	SocketTarget = Helper->GetDesiredTypedInner(Src, SocketTarget);
	if (!IsValid(SocketTarget))
	{
		UE_LOG(LogGameplay,
		       Log,
		       TEXT("Executed from \"%s\". Socket Owner \"%s\" not able to retrieve inner child type. Deferring Actor parenting."),
		       UAVVMGameplayUtils::PrintNetSource(Src).GetData(),
		       *Src->GetName());

		IAVVMDoesSupportSocketTargeting::Execute_DeferredSocketParenting(Src, ContextArgs);
		return false;
	}
	else
	{
		// @gdemers Update our Owning outer for future ASC retrieval from the AbilitySystem interface api.
		IAVVMDoesSupportSocketTargeting::Execute_Attach(Src, SocketTarget, AttachmentSlotTag, SocketName);
		return true;
	}
}

bool FAVVMSocketTargetingHelper::Static_AttachToActor(AActor* Src, const FAVVMSocketTargetingDeferralContextArgs& ContextArgs)
{
	const FGameplayTag& AttachmentSlotTag = ContextArgs.AttachmentSlotTag;
	const FName SocketName = ContextArgs.SocketName;
	AActor* Parent = ContextArgs.Parent.Get();

	if (!IsValid(Src) || !IsValid(Parent))
	{
		return false;
	}

	// @gdemers actor we traverse, and search socket on.
	AActor* SocketTarget = Parent;

	const bool bDoesImplement = Src->Implements<UAVVMDoesSupportSocketTargeting>();
	if (!bDoesImplement)
	{
		// @gdemers we don't support inner targeting so we must be rooted under the target Pawn.
		Src->AttachToActor(SocketTarget, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
		return true;
	}

	// @gdemers Update our Owning outer for future ASC retrieval from the AbilitySystem interface api.
	IAVVMDoesSupportSocketTargeting::Execute_Attach(Src, SocketTarget, AttachmentSlotTag, SocketName);
	return true;
}

bool FAVVMSocketTargetingHelper::Static_Detach(AActor* Src)
{
	if (!IsValid(Src) || !Src->Implements<UAVVMDoesSupportSocketTargeting>())
	{
		return false;
	}

	IAVVMDoesSupportSocketTargeting::Execute_Detach(Src);
	return true;
}

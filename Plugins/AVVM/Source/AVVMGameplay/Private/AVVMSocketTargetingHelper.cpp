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

FDelegateHandle IAVVMDoesSupportSocketDeferral::OnSocketParentAvailableDelegate_Add(const FOnParentSocketAvailableDelegate::FDelegate& Callback)
{
	return OnParentSocketAvailable.Add(Callback);
}

void IAVVMDoesSupportSocketDeferral::OnSocketParentAvailableDelegate_Remove(const FDelegateHandle& Handle)
{
	OnParentSocketAvailable.Remove(Handle);
}

bool FAVVMSocketTargetingHelper::Static_AttachToActorAsync(AActor* Src, const FAVVMSocketTargetingDeferralContextArgs& ContextArgs)
{
	const FName SocketName = ContextArgs.SocketName;
	AActor* Parent = ContextArgs.Parent.Get();

	if (!IsValid(Src) || !IsValid(Parent))
	{
		return false;
	}

	// @gdemers actor we traverse, and search socket on.
	AActor* SocketTarget = Parent;

	// @gdemers in some cases, this will fail which is expected!
	const bool bDoesImplement = Src->Implements<UAVVMDoesSupportInnerSocketTargeting>();
	if (!bDoesImplement)
	{
		// @gdemers we don't support inner targeting so we must be rooted under the target Pawn.
		Src->AttachToActor(SocketTarget, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
		return true;
	}

	const TInstancedStruct<FAVVMSocketTargetingHelper> SocketHelper = IAVVMDoesSupportInnerSocketTargeting::Execute_GetSocketHelper(Src);

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

		IAVVMDoesSupportInnerSocketTargeting::Execute_DeferredSocketParenting(Src, ContextArgs);
		return false;
	}
	else
	{
		// @gdemers Update our Owning outer for future ASC retrieval from the AbilitySystem interface api.
		IAVVMDoesSupportInnerSocketTargeting::Execute_Attach(Src, SocketTarget, SocketName);
		return true;
	}
}

bool FAVVMSocketTargetingHelper::Static_AttachToActor(AActor* Src, const FAVVMSocketTargetingDeferralContextArgs& ContextArgs)
{
	const FName SocketName = ContextArgs.SocketName;
	AActor* Parent = ContextArgs.Parent.Get();

	if (!IsValid(Src) || !IsValid(Parent))
	{
		return false;
	}

	// @gdemers actor we traverse, and search socket on.
	AActor* SocketTarget = Parent;

	const bool bDoesImplement = Src->Implements<UAVVMDoesSupportInnerSocketTargeting>();
	if (!bDoesImplement)
	{
		// @gdemers we don't support inner targeting so we must be rooted under the target Pawn.
		Src->AttachToActor(SocketTarget, FAttachmentTransformRules::KeepRelativeTransform, SocketName);
		return true;
	}

	// @gdemers Update our Owning outer for future ASC retrieval from the AbilitySystem interface api.
	IAVVMDoesSupportInnerSocketTargeting::Execute_Attach(Src, SocketTarget, SocketName);
	return true;
}

bool FAVVMSocketTargetingHelper::Static_Detach(AActor* Src)
{
	if (!IsValid(Src) || !Src->Implements<UAVVMDoesSupportInnerSocketTargeting>())
	{
		return false;
	}

	IAVVMDoesSupportInnerSocketTargeting::Execute_Detach(Src);
	return true;
}

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
#pragma once

#include "CoreMinimal.h"

#include "StructUtils/InstancedStruct.h"

#include "AVVMSocketTargetingHelper.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMSocketTargetingHelper is a context struct that define implementation details on how to handle socketing action with root actor.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMSocketTargetingHelper
{
	GENERATED_BODY()
	
	virtual ~FAVVMSocketTargetingHelper() = default;
	virtual AActor* GetDesiredTypedInner(AActor* Src, AActor* Target) const PURE_VIRTUAL(GetDesiredTypedInner, return nullptr;);

	static bool Static_AttachToActor(AActor* Src, const FAVVMSocketTargetingDeferralContextArgs& ContextArgs);
	static bool Static_Detach(AActor* Src);
	
	// @gdemers wrapper function template to avoid writing TInstancedStruct<FAVVMSocketTargetingHelper>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FAVVMSocketTargetingHelper> Make(TArgs&&... Args);

	static TInstancedStruct<FAVVMSocketTargetingHelper> Empty;
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FAVVMSocketTargetingHelper> FAVVMSocketTargetingHelper::Make(TArgs&&... Args)
{
	return TInstancedStruct<FAVVMSocketTargetingHelper>::Make<TChild>(Forward<TArgs>(Args)...);
}

template<> struct TBaseStructure<FAVVMSocketTargetingHelper> 
{
	static AVVMGAMEPLAY_API UScriptStruct* Get(); 
};

/**
 *	Class description:
 *	
 *	FAVVMSocketTargetingDeferralContextArgs is a context struct that encapsulate information about
 *	socket targeting behaviour.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMSocketTargetingDeferralContextArgs
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadWrite)
	TWeakObjectPtr<AActor> Parent = nullptr;

	UPROPERTY(Transient, BlueprintReadWrite)
	FName SocketName = NAME_None;

	UPROPERTY(Transient, BlueprintReadWrite)
	FSoftObjectPath SrcAttributeSetSoftObjectPath = FSoftObjectPath();
};

/**
 *	Class description:
 *	
 *	IAVVMDoesSupportSocketInnerTargeting is an interface to be impl in actor classes that require attachment from an actor thats lives as child of a root actor. example : Attachment Actor that require
 *	socketing in Weapons (which itself is rooted under the ACharacter).
 */
UINTERFACE(BlueprintType)
class AVVMGAMEPLAY_API UAVVMDoesSupportInnerSocketTargeting : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMDoesSupportInnerSocketTargeting
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	TInstancedStruct<FAVVMSocketTargetingHelper> GetSocketHelper() const;
	virtual TInstancedStruct<FAVVMSocketTargetingHelper> GetSocketHelper_Implementation() const PURE_VIRTUAL(GetSocketHelper_Implementation, return FAVVMSocketTargetingHelper::Empty;);

	UFUNCTION(BlueprintNativeEvent)
	void DeferredSocketParenting(const FAVVMSocketTargetingDeferralContextArgs& ContextArgs);
	virtual void DeferredSocketParenting_Implementation(const FAVVMSocketTargetingDeferralContextArgs& ContextArgs) PURE_VIRTUAL(DefferSocketParenting_Implementation, return;);
	
	UFUNCTION(BlueprintNativeEvent)
	void Attach(AActor* Target, const FName SocketName);
	virtual void Attach_Implementation(AActor* Target, const FName NewSocketName) PURE_VIRTUAL(Attach_Implementation, return;);

	UFUNCTION(BlueprintNativeEvent)
	void Detach();
	virtual void Detach_Implementation() PURE_VIRTUAL(Detach_Implementation, return;);
};

/**
 *	Class description:
 *	
 *	IAVVMDoesSupportSocketDeferral is an interface to be impl in actor classes that require deferred attachment to parent actor that arent
 *	yet available via the inventory system.
 */
UINTERFACE(BlueprintType)
class AVVMGAMEPLAY_API UAVVMDoesSupportSocketDeferral : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMDoesSupportSocketDeferral
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnParentSocketAvailableDelegate, AActor* Parent, AActor* Target);

	virtual void NotifyAvailableSocketParent(AActor* SocketTarget) PURE_VIRTUAL(NotifyAvailableSocketParent, return;);
	FDelegateHandle OnSocketParentAvailableDelegate_Add(const FOnParentSocketAvailableDelegate::FDelegate& Callback);
	void OnSocketParentAvailableDelegate_Remove(const FDelegateHandle& Handle);

protected:
	FOnParentSocketAvailableDelegate OnParentSocketAvailable;
};

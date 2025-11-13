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

#include "Components/ActorComponent.h"
#include "StructUtils/InstancedStruct.h"

#include "AVVMGameStateHandshakeComponent.generated.h"

struct FAVVMNotificationPayload;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnHandshakeRequestComplete, const bool, bWasSuccess, const TInstancedStruct<FAVVMNotificationPayload>&, NewHandshakePayload);

/**
 *	Class description:
 *
 *	UAVVMGameStateHandshakeComponent is a system that handle server validation when trying to execute actions between actors during gameplay.
 *
 *	Example : Trading items between players, Trying to raise a player, etc...
 */
UCLASS(ClassGroup=("AVVMGameplay"), Blueprintable, meta=(BlueprintSpawnableComponent))
class AVVMGAMEPLAY_API UAVVMGameStateHandshakeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable)
	static void Static_ProcessHandshake(const UObject* WorldContextObject,
	                                    const TInstancedStruct<FAVVMNotificationPayload>& NewHandshakePayload,
	                                    const FOnHandshakeRequestComplete& NewCallback);

protected:
	static UAVVMGameStateHandshakeComponent* GetActorComponent(const UObject* WorldContextObject);
	
	void ProcessHandshake(const TInstancedStruct<FAVVMNotificationPayload>& NewHandshakePayload,
						  const FOnHandshakeRequestComplete& NewCallback) const;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<const AActor> OwningOuter = nullptr;
};

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

#include "Engine/StreamableManager.h"
#include "Subsystems/LocalPlayerSubsystem.h"

#include "AVVMGameFrameworkInputMappingContextManager.generated.h"

class UInputMappingContext;
class ULocalPlayer;

/**
 *	Class description:
 *	
 *	UAVVMGameFrameworkInputMappingContextManager is a locally controlled subsystem that tracks IMCs
 *	registration through the GameFeatureAction system.
 *	
 *	This system allow GameFeature plugin to register/bind with the underline input system piped through the AVVMAbilityInputComponent,
 *	and Unreal EnhancedInputSystem.
 */
UCLASS()
class AVVMGAMEPLAY_API UAVVMGameFrameworkInputMappingContextManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	static void AddGameFrameworkInputMappingContextReceiver(const APlayerController* Receiver, const bool bAddOnlyInGameWorlds = true);
	static void RemoveGameFrameworkInputMappingContextReceiver(const APlayerController* Receiver);

protected:
	static UAVVMGameFrameworkInputMappingContextManager* Get(const ULocalPlayer* LocalPlayer);
	void AddReceiver(const APlayerController* Receiver, const bool bAddOnlyInGameWorlds = true);
	void RemoveReceiver(const APlayerController* Receiver);

	void AddIMCRequest(const UWorld* World,
	                   const ULocalPlayer* LocalPlayer,
	                   const TSoftObjectPtr<UInputMappingContext>& IMCSoftObjectPtr);

	void UnRegisterInputMappingContext(const APlayerController* NewPC);
	void RegisterInputMappingContext(const APlayerController* NewPC);

	/**
	 *	Class description:
	 *	
	 *	FAVVMRegisteredInputMappingContexts is a context struct to cached IMCs resources allocated during GameFeature activation.
	 */
	struct FAVVMRegisteredInputMappingContexts
	{
		TSharedPtr<FStreamableHandle> StreamableHandle;
		TArray<FSoftObjectPath> IMCSoftObjectPaths;
	};

	TMap<TWeakObjectPtr<const ULocalPlayer>, FAVVMRegisteredInputMappingContexts> RegisteredInputMappingContexts;
	friend class UAVVMGameFeatureAction_AddInputMappingContext;
};

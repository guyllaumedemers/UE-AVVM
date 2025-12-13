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
#include "Inputs/AVVMGameFrameworkInputMappingContextManager.h"

#include "InputMappingContext.h"
#include "Ability/AVVMAbilityInputComponent.h"
#include "Engine/AssetManager.h"

void UAVVMGameFrameworkInputMappingContextManager::AddGameFrameworkInputMappingContextReceiver(const APlayerController* Receiver, bool bAddOnlyInGameWorlds)
{
	if (!IsValid(Receiver))
	{
		return;
	}

	auto* ContextManager = UAVVMGameFrameworkInputMappingContextManager::Get(Receiver->GetLocalPlayer());
	if (IsValid(ContextManager))
	{
		ContextManager->AddReceiver(Receiver, bAddOnlyInGameWorlds);
	}
}

void UAVVMGameFrameworkInputMappingContextManager::RemoveGameFrameworkInputMappingContextReceiver(const APlayerController* Receiver)
{
	if (!IsValid(Receiver))
	{
		return;
	}

	auto* ContextManager = UAVVMGameFrameworkInputMappingContextManager::Get(Receiver->GetLocalPlayer());
	if (IsValid(ContextManager))
	{
		ContextManager->RemoveReceiver(Receiver);
	}
}

UAVVMGameFrameworkInputMappingContextManager* UAVVMGameFrameworkInputMappingContextManager::Get(const ULocalPlayer* LocalPlayer)
{
	return ULocalPlayer::GetSubsystem<UAVVMGameFrameworkInputMappingContextManager>(LocalPlayer);
}

void UAVVMGameFrameworkInputMappingContextManager::AddReceiver(const APlayerController* Receiver, bool bAddOnlyInGameWorlds)
{
	if (!IsValid(Receiver) || Receiver->IsTemplate(RF_ClassDefaultObject))
	{
		return;
	}

	const UWorld* World = Receiver->GetWorld();
	if (!IsValid(World) || !World->IsGameWorld())
	{
		return;
	}

	RegisterInputMappingContext(Receiver);
}

void UAVVMGameFrameworkInputMappingContextManager::RemoveReceiver(const APlayerController* Receiver)
{
	if (!IsValid(Receiver) || Receiver->IsTemplate(RF_ClassDefaultObject))
	{
		return;
	}

	const UWorld* World = Receiver->GetWorld();
	if (!IsValid(World) || !World->IsGameWorld())
	{
		return;
	}

	UnRegisterInputMappingContext(Receiver);
}

void UAVVMGameFrameworkInputMappingContextManager::AddIMCRequest(const UWorld* World,
                                                                 const ULocalPlayer* LocalPlayer,
                                                                 const TSoftObjectPtr<UInputMappingContext>& IMCSoftObjectPtr)
{
	if (!IsValid(LocalPlayer))
	{
		return;
	}

	FAVVMRegisteredInputMappingContexts& OutResult = RegisteredInputMappingContexts.FindOrAdd(LocalPlayer);
	OutResult.IMCSoftObjectPaths.Add(IMCSoftObjectPtr.ToSoftObjectPath());

	const APlayerController* PC = LocalPlayer->GetPlayerController(World);
	RegisterInputMappingContext(PC);
}

void UAVVMGameFrameworkInputMappingContextManager::UnRegisterInputMappingContext(const APlayerController* NewPC)
{
	if (!IsValid(NewPC))
	{
		return;
	}

	auto* AbilityInputComponent = NewPC->GetComponentByClass<UAVVMAbilityInputComponent>();
	if (!IsValid(AbilityInputComponent))
	{
		return;
	}

	FAVVMRegisteredInputMappingContexts& OutResult = RegisteredInputMappingContexts.FindOrAdd(NewPC->GetLocalPlayer());
	if (OutResult.IMCSoftObjectPaths.IsEmpty())
	{
		return;
	}

	TSharedPtr<FStreamableHandle> Handle = OutResult.StreamableHandle;
	if (!Handle.IsValid())
	{
		return;
	}

	TArray<UObject*> OutResources;
	Handle->GetLoadedAssets(OutResources);

	TArray<const UInputMappingContext*> IMCs;
	for (const UObject* OutResource : OutResources)
	{
		const auto* IMC = Cast<UInputMappingContext>(OutResource);
		IMCs.Add(IMC);
	}

	AbilityInputComponent->UnRegisterGameFrameworkIMCs(IMCs);
	OutResult.StreamableHandle.Reset();
}

void UAVVMGameFrameworkInputMappingContextManager::RegisterInputMappingContext(const APlayerController* NewPC)
{
	if (!IsValid(NewPC))
	{
		return;
	}

	FAVVMRegisteredInputMappingContexts& OutResult = RegisteredInputMappingContexts.FindOrAdd(NewPC->GetLocalPlayer());
	if (OutResult.IMCSoftObjectPaths.IsEmpty())
	{
		return;
	}

	const auto OnAsyncRequestComplete = [](const TWeakObjectPtr<const UAVVMGameFrameworkInputMappingContextManager>& Caller,
	                                       const TWeakObjectPtr<const APlayerController>& PC)
	{
		const UAVVMGameFrameworkInputMappingContextManager* InputContextManager = Caller.Get();
		if (!IsValid(InputContextManager))
		{
			return;
		}

		const APlayerController* Target = PC.Get();
		if (!IsValid(Target))
		{
			return;
		}

		auto* AbilityInputComponent = Target->GetComponentByClass<UAVVMAbilityInputComponent>();
		if (!IsValid(AbilityInputComponent))
		{
			return;
		}

		const FAVVMRegisteredInputMappingContexts* OutResult = InputContextManager->RegisteredInputMappingContexts.Find(Target->GetLocalPlayer());
		if (!ensureAlwaysMsgf(OutResult != nullptr, TEXT("Invalid Search result.")))
		{
			return;
		}

		TSharedPtr<FStreamableHandle> Handle = OutResult->StreamableHandle;
		if (!Handle.IsValid())
		{
			return;
		}

		TArray<UObject*> OutResources;
		Handle->GetLoadedAssets(OutResources);

		TArray<const UInputMappingContext*> IMCs;
		for (const UObject* OutResource : OutResources)
		{
			const auto* IMC = Cast<UInputMappingContext>(OutResource);
			IMCs.Add(IMC);
		}

		AbilityInputComponent->RegisterGameFrameworkIMCs(IMCs);
	};

	const auto Callback = FStreamableDelegate::CreateWeakLambda(this, OnAsyncRequestComplete, TWeakObjectPtr(this), TWeakObjectPtr(NewPC));
	OutResult.StreamableHandle = UAssetManager::Get().LoadAssetList(OutResult.IMCSoftObjectPaths, Callback);
}

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

#include "Ability/AVVMAbilityInputComponent.h"

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

TSharedPtr<FAVVMIMCRequestHandle> UAVVMGameFrameworkInputMappingContextManager::AddIMCRequest(const UWorld* World,
                                                                                              const ULocalPlayer* LocalPlayer,
                                                                                              const TSoftObjectPtr<UInputMappingContext>& IMCSoftObjectPtr)
{
	if (!IsValid(LocalPlayer))
	{
		return nullptr;
	}
	
	FAVVMRegisteredInputMappingContexts& OutResult = RegisteredInputMappingContexts.FindOrAdd(LocalPlayer);
	OutResult.Add(IMCSoftObjectPtr.ToSoftObjectPath());
	
	const APlayerController* PC = LocalPlayer->GetPlayerController(World);
	RegisterInputMappingContext(PC);

	return MakeShared<FAVVMIMCRequestHandle>(this, LocalPlayer);
}

void UAVVMGameFrameworkInputMappingContextManager::UnRegisterInputMappingContext(const APlayerController* NewPC)
{
	if (!IsValid(NewPC))
	{
		return;
	}

	auto* AbilityInputComponent = NewPC->GetComponentByClass<UAVVMAbilityInputComponent>();
	if (IsValid(AbilityInputComponent))
	{
		FAVVMRegisteredInputMappingContexts& OutResult = RegisteredInputMappingContexts.FindOrAdd(NewPC->GetLocalPlayer());
		AbilityInputComponent->UnRegisterGameFrameworkIMCs(OutResult.StreamableHandle);
	}
}

void UAVVMGameFrameworkInputMappingContextManager::RegisterInputMappingContext(const APlayerController* NewPC)
{
	if (!IsValid(NewPC))
	{
		return;
	}

	auto* AbilityInputComponent = NewPC->GetComponentByClass<UAVVMAbilityInputComponent>();
	if (IsValid(AbilityInputComponent))
	{
		FAVVMRegisteredInputMappingContexts& OutResult = RegisteredInputMappingContexts.FindOrAdd(NewPC->GetLocalPlayer());
		OutResult.StreamableHandle = AbilityInputComponent->RegisterGameFrameworkIMCs(OutResult.IMCSoftObjectPaths);
	}
}

void UAVVMGameFrameworkInputMappingContextManager::FAVVMRegisteredInputMappingContexts::Add(const FSoftObjectPath& IMCSoftObjectPath)
{
}

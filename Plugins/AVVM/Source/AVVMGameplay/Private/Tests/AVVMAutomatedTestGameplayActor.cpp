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
#include "AVVMAutomatedTestGameplayActor.h"

#include "AVVMAutomatedTestResourceComponent.h"
#include "AVVMGameplaySettings.h"
#include "Ability/AVVMAbilityResourceHandlingImpl.h"
#include "Ability/AVVMAbilitySystemComponent.h"
#include "AutomatedTest/AVVMAutomatedTestResourceValidationManager.h"
#include "Data/AVVMActorResourceHandlingImpl.h"

AAVVMAutomatedTestGameplayActor::AAVVMAutomatedTestGameplayActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ResourceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMAutomatedTestResourceComponent>(this, TEXT("ResourceManagerComponent"));
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
}

UAVVMResourceManagerComponent* AAVVMAutomatedTestGameplayActor::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}

TArray<FDataRegistryId> AAVVMAutomatedTestGameplayActor::GetResourceDefinitionRegistryIds_Implementation() const
{
	const auto TestRegistryId = FDataRegistryId(UAVVMGameplaySettings::GetActorDefinitionRegistryType(), TEXT("DEMO_AutomatedTest"));
	ensureAlwaysMsgf(TestRegistryId.IsValid(), TEXT("RegistryType or ItemName arent valid. Please validate your Data Registry."));
	return {TestRegistryId};
}

TArray<FDataRegistryId> AAVVMAutomatedTestGameplayActor::CheckIsDoneAcquiringResources_Implementation(const TArray<UObject*>& Resources) const
{
	TArray<FDataRegistryId> OutResults;
	OutResults.Append(UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources(
	                                                                                UAVVMActorResourceHandlingImpl::StaticClass(),
	                                                                                nullptr/*expected*/,
	                                                                                Resources));

	OutResults.Append(UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources(
	                                                                                UAVVMAbilityResourceHandlingImpl::StaticClass(),
	                                                                                AbilitySystemComponent,
	                                                                                Resources));

	return OutResults;
}

void AAVVMAutomatedTestGameplayActor::SetTestFlag(TSharedRef<bool> bNewIsAsyncProcessCompleted)
{
	bIsAsyncProcessCompleted = bNewIsAsyncProcessCompleted;
}

bool AAVVMAutomatedTestGameplayActor::CheckContentIntegrity() const
{
	return UAVVMAutomatedTestResourceValidationManager::Static_IsIntegral(GetWorld(), ResourceManagerComponent);
}

bool AAVVMAutomatedTestGameplayActor::CheckASCIntegrity() const
{
	return UAVVMAutomatedTestResourceValidationManager::Static_IsIntegral(GetWorld(), AbilitySystemComponent);
}

bool AAVVMAutomatedTestGameplayActor::HasASCFinishedAllStreaming() const
{
	if (!IsValid(AbilitySystemComponent) || AbilitySystemComponent->AbilityHandleSystem.IsEmpty())
	{
		return true;
	}

	bool bHasFinishedStreaming = true;
	for (const auto& [TokenId, StreamingHandle] : AbilitySystemComponent->AbilityHandleSystem)
	{
		bHasFinishedStreaming &= (StreamingHandle.IsValid() && StreamingHandle->HasLoadCompleted());
	}

	return bHasFinishedStreaming && (AbilitySystemComponent->AbilitySpecHandles.Num() == AbilitySystemComponent->AbilityHandleSystem.Num());
}

void AAVVMAutomatedTestGameplayActor::ForceCompletion() const
{
	(*bIsAsyncProcessCompleted) = true;
}

FOnResourceAsyncLoadingComplete AAVVMAutomatedTestGameplayActor::GetOnCompleteDelegate()
{
	FOnResourceAsyncLoadingComplete Callback;
	Callback.BindDynamic(this, &AAVVMAutomatedTestGameplayActor::OnRequestCompleted);
	return Callback;
}

void AAVVMAutomatedTestGameplayActor::OnRequestCompleted()
{
	// @gdemers Our async process is complete. All resources were loaded!
	ForceCompletion();
}

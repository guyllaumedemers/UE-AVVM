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
#include "AutomatedTestInventoryActor.h"

#include "ActorInventoryComponent.h"
#include "AVVMGameplaySettings.h"
#include "AutomatedTest/AVVMAutomatedTestResourceValidationManager.h"
#include "Data/AVVMActorResourceHandlingImpl.h"
#include "Resources/InventoryResourceHandlingImpl.h"

AAutomatedTestInventoryActor::AAutomatedTestInventoryActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ResourceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMResourceManagerComponent>(this, TEXT("ResourceManagerComponent"));
	InventoryComponent = ObjectInitializer.CreateDefaultSubobject<UActorInventoryComponent>(this, TEXT("InventoryComponent"));
}

UAVVMResourceManagerComponent* AAutomatedTestInventoryActor::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}

TArray<FDataRegistryId> AAutomatedTestInventoryActor::GetResourceDefinitionRegistryIds_Implementation() const
{
	const auto TestRegistryId = FDataRegistryId(UAVVMGameplaySettings::GetActorDefinitionRegistryType(), TEXT("DEMO_AutomatedTest_InventoryActor"));
	ensureAlwaysMsgf(TestRegistryId.IsValid(), TEXT("RegistryType or ItemName arent valid. Please validate your Data Registry."));
	return {TestRegistryId};
}

TArray<FDataRegistryId> AAutomatedTestInventoryActor::CheckIsDoneAcquiringResources_Implementation(const TArray<UObject*>& Resources) const
{
	TArray<FDataRegistryId> OutResults;
	OutResults.Append(UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources(
	                                                                                UAVVMActorResourceHandlingImpl::StaticClass(),
	                                                                                nullptr/*expected*/,
	                                                                                Resources));

	OutResults.Append(UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources(
	                                                                                UInventoryResourceHandlingImpl::StaticClass(),
	                                                                                InventoryComponent,
	                                                                                Resources));

	return OutResults;
}

void AAutomatedTestInventoryActor::SetTestFlag(TSharedRef<bool> bNewIsAsyncProcessCompleted)
{
	bIsAsyncProcessCompleted = bNewIsAsyncProcessCompleted;
}

bool AAutomatedTestInventoryActor::CheckContentIntegrity() const
{
	return UAVVMAutomatedTestResourceValidationManager::Static_IsIntegral(GetWorld(), ResourceManagerComponent);
}

bool AAutomatedTestInventoryActor::CheckInventoryIntegrity() const
{
	return true;
}

bool AAutomatedTestInventoryActor::HasInventoryFinishedAllStreaming() const
{
	return true;
}

void AAutomatedTestInventoryActor::ForceCompletion() const
{
	(*bIsAsyncProcessCompleted) = true;
}

FOnResourceAsyncLoadingComplete AAutomatedTestInventoryActor::GetOnCompleteDelegate()
{
	FOnResourceAsyncLoadingComplete Callback;
	Callback.BindDynamic(this, &AAutomatedTestInventoryActor::OnRequestCompleted);
	return Callback;
}

void AAutomatedTestInventoryActor::OnRequestCompleted()
{
	// @gdemers Our async process is complete. All resources were loaded!
	ForceCompletion();
}

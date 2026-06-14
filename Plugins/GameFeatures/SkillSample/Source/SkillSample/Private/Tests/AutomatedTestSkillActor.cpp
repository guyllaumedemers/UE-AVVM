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
#include "AutomatedTestSkillActor.h"

#include "ActorSkillTreeComponent.h"
#include "AVVMFileHelper.h"
#include "SkillTreeSettings.h"
#include "AutomatedTest/AVVMAutomatedTestResourceValidationManager.h"
#include "Engine/StreamableManager.h"
#include "Resources/SkillTreeResourceHandlingImpl.h"
#include "Tags/PrivateTags.h"

AAutomatedTestSkillActor::AAutomatedTestSkillActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ResourceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMResourceManagerComponent>(this, TEXT("ResourceManagerComponent"));
	SkillTreeComponent = ObjectInitializer.CreateDefaultSubobject<UActorSkillTreeComponent>(this, TEXT("SkillTreeComponent"));
}

int32 AAutomatedTestSkillActor::GetProviderUniqueId_Implementation() const
{
	// @gdemers Hard coded in our ActorIdentifier Data Table.
	return 0;
}

UAVVMResourceManagerComponent* AAutomatedTestSkillActor::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}

TArray<FDataRegistryId> AAutomatedTestSkillActor::GetResourceDefinitionRegistryIds_Implementation() const
{
	const auto TestRegistryId = FDataRegistryId(USkillTreeSettings::GetSkillTreeRegistryType(), TEXT("DEMO_SkillSample_AutomatedTest_C"));
	ensureAlwaysMsgf(TestRegistryId.IsValid(), TEXT("RegistryType or ItemName arent valid. Please validate your Data Registry."));
	return {TestRegistryId};
}

TArray<FDataRegistryId> AAutomatedTestSkillActor::CheckIsDoneAcquiringResources_Implementation(const TArray<UObject*>& Resources) const
{
	TArray<FDataRegistryId> OutResults;
	OutResults.Append(UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources(
	                                                                                USkillTreeResourceHandlingImpl::StaticClass(),
	                                                                                SkillTreeComponent,
	                                                                                Resources));

	return OutResults;
}

void AAutomatedTestSkillActor::SetTestFlag(TSharedRef<bool> bNewIsAsyncProcessCompleted)
{
	bIsAsyncProcessCompleted = bNewIsAsyncProcessCompleted;
}

bool AAutomatedTestSkillActor::CheckContentIntegrity() const
{
	return UAVVMAutomatedTestResourceValidationManager::Static_IsIntegral(GetWorld(), ResourceManagerComponent);
}

bool AAutomatedTestSkillActor::CheckSkillTreeIntegrity() const
{
	return UAVVMAutomatedTestResourceValidationManager::Static_IsIntegral(GetWorld(), SkillTreeComponent);
}

bool AAutomatedTestSkillActor::HasSkillTreeFinishedAllStreaming() const
{
	if (!IsValid(SkillTreeComponent))
	{
		return true;
	}

	bool bHasFinishedStreaming = true;
	for (const auto& [TokenId, StreamingHandle] : SkillTreeComponent->SkillTreeNodeHandleSystem)
	{
		bHasFinishedStreaming &= (StreamingHandle.IsValid() && StreamingHandle->HasLoadCompleted());
	}

	return bHasFinishedStreaming && (SkillTreeComponent->NonReplicatedActiveGameplayEffectHandles.Num() == SkillTreeComponent->SkillTreeNodeHandleSystem.Num());
}

void AAutomatedTestSkillActor::ForceCompletion() const
{
	(*bIsAsyncProcessCompleted) = true;
}

FOnResourceAsyncLoadingComplete AAutomatedTestSkillActor::GetOnCompleteDelegate()
{
	FOnResourceAsyncLoadingComplete Callback;
	Callback.BindDynamic(this, &AAutomatedTestSkillActor::OnRequestCompleted);
	return Callback;
}

void AAutomatedTestSkillActor::OnRequestCompleted()
{
	// @gdemers Our async process is complete. All resources were loaded!
	ForceCompletion();
}

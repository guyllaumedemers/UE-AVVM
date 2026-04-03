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

AAVVMAutomatedTestGameplayActor::AAVVMAutomatedTestGameplayActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ResourceManagerComponent = ObjectInitializer.CreateDefaultSubobject<UAVVMAutomatedTestResourceComponent>(this, TEXT("ResourceManagerComponent"));
}

UAVVMResourceManagerComponent* AAVVMAutomatedTestGameplayActor::GetResourceManagerComponent_Implementation() const
{
	return ResourceManagerComponent;
}

TArray<FDataRegistryId> AAVVMAutomatedTestGameplayActor::GetResourceDefinitionResourceIds_Implementation() const
{
	return {TestRegistryId};
}

TArray<FDataRegistryId> AAVVMAutomatedTestGameplayActor::CheckIsDoneAcquiringResources_Implementation(const TArray<UObject*>& Resources) const
{
	const TArray<FDataRegistryId> OutResults = UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources(ResourceHandlingImplClass,
	                                                                                                         nullptr,
	                                                                                                         Resources);

	// TODO @gdemers we may want to run more advance async loading, and have nested resource loading. This
	// will require passing in ActorComponent of specific types.
	if (OutResults.IsEmpty())
	{
		// @gdemers Our async process is complete. All resources were loaded!
		ForceCompletion();
	}

	return OutResults;
}

void AAVVMAutomatedTestGameplayActor::SetTestFlag(TSharedRef<bool> bNewIsAsyncProcessCompleted)
{
	bIsAsyncProcessCompleted = bNewIsAsyncProcessCompleted;
}

bool AAVVMAutomatedTestGameplayActor::CheckContentIntegrality() const
{
	// TODO @gdemers we may want to valid our content integrity here...
	return true;
}

void AAVVMAutomatedTestGameplayActor::ForceCompletion() const
{
	(*bIsAsyncProcessCompleted) = true;
}

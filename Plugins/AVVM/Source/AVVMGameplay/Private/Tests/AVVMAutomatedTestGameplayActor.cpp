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
#include "UObject/GarbageCollectionSchema.h"

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
	// @gdemers set 1x registry id per definition type this framework expose
	return {};
}

void AAVVMAutomatedTestGameplayActor::Run() const
{
	UAVVMResourceManagerComponent* TestComponent = GetResourceManagerComponent();
	if (!IsValid(TestComponent))
	{
		return;
	}

	for (const auto& ResourceId : GetResourceDefinitionResourceIds())
	{
		TestComponent->RequestAsyncLoading(ResourceId, {});
	}
}

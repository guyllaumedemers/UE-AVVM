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
#include "Resources/AVVMResourceProvider.h"

#include "AbilitySystemComponent.h"
#include "Ability/AVVMAbilityData.h"
#include "Ability/AVVMAbilitySystemComponent.h"

TArray<FDataRegistryId> UAVVMResourceHandlingBlueprintFunctionLibrary::CheckAbilities(UAbilitySystemComponent* AbilitySystemComponent,
                                                                                      const TArray<UObject*>& Resources)
{
	TArray<FDataRegistryId> OutResources;
	TArray<UObject*> OutAbilities;

	for (UObject* Resource : Resources)
	{
		const auto* AbilityGroup = Cast<UAVVMAbilityGroupDataAsset>(Resource);
		if (IsValid(AbilityGroup))
		{
			OutResources.Append(AbilityGroup->GetAbilities());
		}
		else
		{
			const auto* Ability = Cast<UAVVMAbilityDataAsset>(Resource);
			if (IsValid(Ability))
			{
				OutAbilities.Add(Resource);
			}
		}
	}

	auto* ASC = Cast<UAVVMAbilitySystemComponent>(AbilitySystemComponent);
	if (IsValid(ASC) && UAVVMResourceHandlingBlueprintFunctionLibrary::HasAuthority(ASC->GetTypedOuter<AActor>()) && !OutAbilities.IsEmpty())
	{
		ASC->SetupAbilities(OutAbilities);
	}

	return OutResources;
}

bool UAVVMResourceHandlingBlueprintFunctionLibrary::HasAuthority(const AActor* Outer)
{
	return IsValid(Outer) ? Outer->HasAuthority() : false;
}

bool UAVVMResourceHandlingBlueprintFunctionLibrary::ExecuteResourceProviderDelegate(const TArray<FDataRegistryId>& QueuedResourcesId,
                                                                                    const FKeepProcessingResources& Callback)
{
	return ensureAlwaysMsgf(Callback.IsBound(), TEXT("Error: Previously bound Delegate invalid!")) ? Callback.Execute(QueuedResourcesId) : false;
}

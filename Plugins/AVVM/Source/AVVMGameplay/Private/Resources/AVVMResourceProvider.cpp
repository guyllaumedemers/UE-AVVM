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

#include "Ability/AVVMAbilityDefinitionDataAsset.h"
#include "Resources/AVVMResourceHandlingImpl.h"

TArray<FDataRegistryId> UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources(TSubclassOf<UAVVMResourceHandlingImpl> ResourceHandlingImplClass,
                                                                                      UActorComponent* ActorComponent,
                                                                                      const TArray<UObject*>& Resources)
{
	if (!ensureAlwaysMsgf(IsValid(ResourceHandlingImplClass),
	                      TEXT("UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources missing valid TSubclassOf<UAVVMResourceHandlingImpl> !")))
	{
		return TArray<FDataRegistryId>();
	}

	const auto* ResourceHandlingImpl = ResourceHandlingImplClass->GetDefaultObject<UAVVMResourceHandlingImpl>();
	if (!ensureAlwaysMsgf(IsValid(ResourceHandlingImpl),
	                      TEXT("UAVVMResourceHandlingBlueprintFunctionLibrary::CheckResources CDO cannot be reached!")))
	{
		return TArray<FDataRegistryId>();
	}
	else
	{
		return ResourceHandlingImpl->ProcessResources(ActorComponent, Resources);
	}
}

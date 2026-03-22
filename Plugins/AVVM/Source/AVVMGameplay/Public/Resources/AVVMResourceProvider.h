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

#include "DataRegistryId.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/Interface.h"

#include "AVVMResourceProvider.generated.h"

class UAVVMResourceManagerComponent;
class UAVVMResourceHandlingImpl;

/**
 *	Class description:
 *	
 *	EAVVMDataSourceType represent the Data source Type. I.e How was the data retrieved. Dynamically (as a collection list provided by the Backend) ? or
 *	Statically (from a collection list provided by a Data Asset reference) ?
 */
UENUM(BlueprintType)
enum class EAVVMDataSourceType : uint8
{
	None,
	Static,
	Dynamic,
};

inline const TCHAR* EnumToString(EAVVMDataSourceType State)
{
	switch (State)
	{
		case EAVVMDataSourceType::Static:
			return TEXT("Static");
		case EAVVMDataSourceType::Dynamic:
			return TEXT("Dynamic");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *
 *	UAVVMResourceProvider provide a resource definition via Registry id and support recursive request for loading nested resources.
 */
UINTERFACE(BlueprintType, Blueprintable)
class AVVMGAMEPLAY_API UAVVMResourceProvider : public UInterface
{
	GENERATED_BODY()
};

class AVVMGAMEPLAY_API IAVVMResourceProvider
{
	GENERATED_BODY()

public:
	// @gdemers allow retrieval of RegistryId relevant to Provider type.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FDataRegistryId> GetResourceDefinitionResourceIds() const;
	virtual TArray<FDataRegistryId> GetResourceDefinitionResourceIds_Implementation() const PURE_VIRTUAL(GetResourceDefinitionResourceIds_Implementation, return TArray<FDataRegistryId>(););
	
	// @gdemers allow retrieval of RegistryId from backend {FActorContent.UniqueId} relevant to Provider type.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	int32 GetProviderUniqueId() const;
	virtual int32 GetProviderUniqueId_Implementation() const PURE_VIRTUAL(GetProviderUniqueId_Implementation, return INDEX_NONE;);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UAVVMResourceManagerComponent* GetResourceManagerComponent() const;
	virtual UAVVMResourceManagerComponent* GetResourceManagerComponent_Implementation() const PURE_VIRTUAL(GetResourceManagerComponent_Implementation, return nullptr;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	EAVVMDataSourceType GetDataSourceType() const;
	virtual EAVVMDataSourceType GetDataSourceType_Implementation() const PURE_VIRTUAL(GetDataSourceType_Implementation, return EAVVMDataSourceType::None;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RequestDataFromDataAsset() const;
	virtual void RequestDataFromDataAsset_Implementation() const PURE_VIRTUAL(RequestDataFromDataAsset_Implementation, return;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RequestDataFromMicroService() const;
	virtual void RequestDataFromMicroService_Implementation() const PURE_VIRTUAL(RequestDataFromMicroService_Implementation, return;);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FDataRegistryId> CheckIsDoneAcquiringResources(const TArray<UObject*>& Resources) const;
	virtual TArray<FDataRegistryId> CheckIsDoneAcquiringResources_Implementation(const TArray<UObject*>& Resources) const PURE_VIRTUAL(CheckIsDoneAcquiringResources_Implementation, return TArray<FDataRegistryId>{};);
};

/**
 *	Class description:
 *
 *	UAVVMResourceHandlingBlueprintFunctionLibrary define a set of api to allow reusability and circumvent some issues that arise
 *	with design decision and how BP prevent Dynamic Delegates from being broadcast from BP code.
 */
UCLASS(BlueprintType)
class AVVMGAMEPLAY_API UAVVMResourceHandlingBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="AVVM|Utils")
	static TArray<FDataRegistryId> CheckResources(TSubclassOf<UAVVMResourceHandlingImpl> ResourceHandlingImplClass,
	                                              UActorComponent* ActorComponent,
	                                              const TArray<UObject*>& Resources);
};

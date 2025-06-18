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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "InventoryProvider.generated.h"

class UItemObject;
class UItemCollectionDefinitionDataAsset;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnRetrieveInventoryItems, const TArray<UItemObject*>&, Items);

/**
 *	Class description:
 *	
 *	EItemSrcType represent the Item source Type. I.e How was the data retrieved. Dynamically (as a collection list provided by the Backend) ? or
 *	Statically (from a collection list provided by a Data Asset reference) ?
 */
UENUM(BlueprintType)
enum class EItemSrcType : uint8
{
	None,
	Static,
	Dynamic,
};

inline const TCHAR* EnumToString(EItemSrcType State)
{
	switch (State)
	{
		case EItemSrcType::Static:
			return TEXT("Static");
		case EItemSrcType::Dynamic:
			return TEXT("Dynamic");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 *	Class description:
 *
 *	UInventoryProvider provide a set of api to determine if the Owning Actor support a pre-defined list of Items to hold. Note : Since the interface is defined in a GFP,
 *	we expect the plugin to be pre-loaded and never unloaded at runtime, otherwise expect linkage issues with undefined symbol for external classes overriding the api.
 *
 *	Additionally, we suggest handling the override details in BP and avoid direct reference of this DLL in your system to reduce compile/linkage time.
 */
UINTERFACE(BlueprintType, Blueprintable)
class INVENTORYSAMPLE_API UInventoryProvider : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYSAMPLE_API IInventoryProvider
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ProcessStaticItems(const FOnRetrieveInventoryItems& Callback) const;
	virtual void ProcessStaticItems_Implementation(const FOnRetrieveInventoryItems& Callback) const PURE_VIRTUAL(ProcessStaticItems_Implementation, return;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ProcessDynamicItems(const FOnRetrieveInventoryItems& Callback) const;
	virtual void ProcessDynamicItems_Implementation(const FOnRetrieveInventoryItems& Callback) const PURE_VIRTUAL(ProcessDynamicItems_Implementation, return;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	EItemSrcType GetItemSrcType() const;
	virtual EItemSrcType GetItemSrcType_Implementation() const PURE_VIRTUAL(GetItemSrcType_Implementation, return EItemSrcType::None;);
};

/**
 *	Class description:
 *
 *	UInventoryBlueprintFunctionLibrary is a function library that expose basic api for executing Provider Request in Blueprint. It works around the issue
 *	where Blueprint Classes, who implement an interface in BP Only, not in Native C++, cannot bind to the 'Add Parent Call' Node.
 */
UCLASS()
class INVENTORYSAMPLE_API UInventoryBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void RequestItems(const UObject* Outer,
	                         const FOnRetrieveInventoryItems& Callback);

	UFUNCTION(BlueprintCallable)
	static void ExecuteInventoryProviderDelegate(const TArray<UItemObject*>& NewItems,
	                                             const FOnRetrieveInventoryItems& Callback);
};

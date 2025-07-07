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
class UItemGroupDefinitionDataAsset;

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
	void RequestItemsFromDataAsset() const;
	virtual void RequestItemsFromDataAsset_Implementation() const PURE_VIRTUAL(RequestItemsFromDataAsset_Implementation, return;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RequestItemsFromMicroService() const;
	virtual void RequestItemsFromMicroService_Implementation() const PURE_VIRTUAL(RequestItemsFromMicroService_Implementation, return;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	EItemSrcType GetItemSrcType() const;
	virtual EItemSrcType GetItemSrcType_Implementation() const PURE_VIRTUAL(GetItemSrcType_Implementation, return EItemSrcType::None;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool DoesSupportSpawnOnLoad() const;
	virtual bool DoesSupportSpawnOnLoad_Implementation() const PURE_VIRTUAL(DoesSupportSpawnOnLoad_Implementation, return false;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsItemEquipped(const UItemObject* NewItem) const;
	virtual bool IsItemEquipped_Implementation(const UItemObject* NewItem) const PURE_VIRTUAL(IsItemEquipped_Implementation, return false;);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	int32 GetProgressionStageIndex(const UItemObject* NewItem) const;
	virtual int32 GetProgressionStageIndex_Implementation(const UItemObject* NewItem) const PURE_VIRTUAL(GetProgressionStageIndex_Implementation, return INDEX_NONE;);
};

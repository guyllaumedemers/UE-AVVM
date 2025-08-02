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

#include "UObject/Object.h"

#include "InventoryStringParser.generated.h"

struct FItemModifierProxy;
struct FActorContent;
struct FActorContentProxy;
struct FItem;
struct FItemProxy;
struct FItemHolder;
struct FItemHolderProxy;
struct FItemModifier;
struct FItemModifierPRoxy;

/**
*	Class description:
 *
 *	UInventoryStringParser is the UObject interfacing with Unreal JSon system to parse FString payload specific
 *	to inventory content stored on the backend.
 */
UCLASS(BlueprintType, Blueprintable)
class INVENTORYSAMPLE_API UInventoryStringParser : public UObject
{
	GENERATED_BODY()

public:
	void FromString(const FString& NewPayload, FActorContent& OutActorContent) const;
	void ToString(const FActorContent& NewActorContent, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FItemHolder& OutItemHolder) const;
	void ToString(const FItemHolder& NewItemHolder, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FItem& OutItem) const;
	void ToString(const FItem& NewItem, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FItemModifier& OutItemModifier) const;
	void ToString(const FItemModifier& NewItemModifier, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FActorContentProxy& OutActorContent) const;
	void ToString(const FActorContentProxy& NewActorContent, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FItemHolderProxy& OutItemHolder) const;
	void ToString(const FItemHolderProxy& NewItemHolder, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FItemProxy& OutItem) const;
	void ToString(const FItemProxy& NewItem, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FItemModifierProxy& OutItemModifier) const;
	void ToString(const FItemModifierProxy& NewItemModifier, FString& OutFormat) const;
};

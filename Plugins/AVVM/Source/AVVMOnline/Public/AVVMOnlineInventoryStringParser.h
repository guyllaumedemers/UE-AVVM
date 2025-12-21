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

#include "AVVMOnlineInventoryStringParser.generated.h"

struct FAVVMItemModifierProxy;
struct FAVVMActorContent;
struct FAVVMActorContentProxy;
struct FAVVMItem;
struct FAVVMItemProxy;
struct FAVVMItemHolder;
struct FAVVMItemHolderProxy;
struct FAVVMItemModifier;
struct FItemModifierPRoxy;

/**
*	Class description:
 *
 *	UInventoryStringParser is the UObject interfacing with Unreal JSon system to parse FString payload specific
 *	to inventory content stored on the backend.
 */
UCLASS(BlueprintType, Blueprintable)
class AVVMONLINE_API UAVVMOnlineInventoryStringParser : public UObject
{
	GENERATED_BODY()

public:
	void FromString(const FString& NewPayload, FAVVMActorContent& OutActorContent) const;
	void ToString(const FAVVMActorContent& NewActorContent, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMItemHolder& OutItemHolder) const;
	void ToString(const FAVVMItemHolder& NewItemHolder, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMItem& OutItem) const;
	void ToString(const FAVVMItem& NewItem, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMItemModifier& OutItemModifier) const;
	void ToString(const FAVVMItemModifier& NewItemModifier, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMActorContentProxy& OutActorContent) const;
	void ToString(const FAVVMActorContentProxy& NewActorContent, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMItemHolderProxy& OutItemHolder) const;
	void ToString(const FAVVMItemHolderProxy& NewItemHolder, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMItemProxy& OutItem) const;
	void ToString(const FAVVMItemProxy& NewItem, FString& OutFormat) const;

	void FromString(const FString& NewPayload, FAVVMItemModifierProxy& OutItemModifier) const;
	void ToString(const FAVVMItemModifierProxy& NewItemModifier, FString& OutFormat) const;
};

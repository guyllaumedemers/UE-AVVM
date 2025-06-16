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

#include "GameStateInventoryHandshakeComponent.generated.h"

class UActorInventoryComponent;
class UItemObject;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHandshakeComplete, const bool, bWasSuccess);

/**
 *	Class description:
 *
 *	FInventoryHandshake define the endpoint of a transaction between two entities and
 *	the data exchanged.
 *
 *	TODO @gdemers we do not account for the fact that a trade may involve currencies. TBD! 
 */
USTRUCT(BlueprintType)
struct INVENTORYSAMPLE_API FInventoryHandshake
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly)
	UActorInventoryComponent* Src = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	UActorInventoryComponent* Dest = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<UItemObject*> SrcItems;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<UItemObject*> DestItems;
};

/**
 *	Class description:
 *
 *	UGameStateInventoryHandshakeComponent handle system behaviour for updating two Inventory Component states
 *	during a handshake action like :
 *		trade,
 *		sell or buy
 */
UCLASS(ClassGroup=("Inventory"), Blueprintable, meta=(BlueprintSpawnableComponent))
class INVENTORYSAMPLE_API UGameStateInventoryHandshakeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	static UGameStateInventoryHandshakeComponent* GetActorComponent(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	void ShakeHands(const FInventoryHandshake& Context,
	                const FOnHandshakeComplete& Callback) const;
};

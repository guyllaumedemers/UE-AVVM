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

#include "AVVMDataTableRow.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "AVVMActorUIDefinitionDataAsset.generated.h"

/**
 *	Class description:
 *	
 *	FAVVMActorUIDefinition is a context struct that can be extended to expose UI information about an Actor type.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMActorUIDefinition
{
	GENERATED_BODY()

	virtual ~FAVVMActorUIDefinition() = default;

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FAVVMActorUIDefinition>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FAVVMActorUIDefinition> Make(TArgs&&... Args);

	static TInstancedStruct<FAVVMActorUIDefinition> Empty;
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FAVVMActorUIDefinition> FAVVMActorUIDefinition::Make(TArgs&&... Args)
{
	return TInstancedStruct<FAVVMActorUIDefinition>::Make<TChild>(Forward<TArgs>(Args)...);
}

template<> struct TBaseStructure<FAVVMActorUIDefinition> 
{
	static AVVMGAMEPLAY_API UScriptStruct* Get(); 
};

/**
 *	Class description:
 *
 *	UAVVMActorUIDefinitionDataAsset is the unique definition of an Actor UI and the content that's expected to be in presented at Runtime.
 */
UCLASS(BlueprintType, NotBlueprintable)
class AVVMGAMEPLAY_API UAVVMActorUIDefinitionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	template <typename T = FAVVMActorUIDefinition>
	const T* GetActorUIDefinition() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TInstancedStruct<FAVVMActorUIDefinition> ActorUIDefinition;
};

template <typename T>
const T* UAVVMActorUIDefinitionDataAsset::GetActorUIDefinition() const
{
	return ActorUIDefinition.GetPtr<T>();
}

/**
 *	Class description:
 *
 *	FAVVMActorUIDefinitionDataTableRow is an entry in a DataTableRow for UI referenced by a unique Actor type.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMActorUIDefinitionDataTableRow : public FAVVMDataTableRow
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual TArray<FSoftObjectPath> GetResourcesPaths() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	TSoftObjectPtr<UAVVMActorUIDefinitionDataAsset> ActorUIDefinition = nullptr;
};

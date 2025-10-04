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

#include "GameplayTagContainer.h"
#include "GameFramework/WorldSettings.h"

#include "AVVMWorldSetting.generated.h"

struct FStreamableHandle;

/**
 *	Class description:
 *
 *	UAVVMWorldRule is a UObject instance that define a condition set specific to the active world.
 *	Server Authority is the sole owner of rules. User shouldnt attempt to modify the state on the Client.
 *
 *	Example of Rules :
 *
 *		* Batch Destroy system.
 *		* Batch Ticking system.
 *		* Pooling system.
 *		* Match Victory conditions.
 *		* Match Phase management.
 *		* etc...
 */
UCLASS(Abstract, BlueprintType)
class AVVMGAMEPLAY_API UAVVMWorldRule : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	bool Predicate() const;
	virtual bool Predicate_Implementation() const PURE_VIRTUAL(Predicate_Implementation, return false;);

	const FGameplayTag& GetRuleTag() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag RuleTag = FGameplayTag::EmptyTag;
};

/**
 *	Class description:
 *
 *	AAVVMWorldSetting is a per-world objects that aggregate conditions for world specific behaviour.
 */
UCLASS()
class AVVMGAMEPLAY_API AAVVMWorldSetting : public AWorldSettings
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable)
	UAVVMWorldRule* GetRule(const FGameplayTag& RuleTag) const;
	
	UFUNCTION(BlueprintCallable)
	bool DoesRuleClassExist(const UClass* RuleClass) const;

	template<typename TRule>
	TRule* CastRule(const FGameplayTag& RuleTag) const;
	
protected:
	TArray<FSoftObjectPath> GetRulePaths() const;
	void AsyncLoadWorldRules(const TArray<FSoftObjectPath>& SoftObjectPaths);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, TSoftClassPtr<UAVVMWorldRule>> RuleClassPerTag;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<FGameplayTag, TObjectPtr<UAVVMWorldRule>> Rules;

	TSharedPtr<FStreamableHandle> StreamableHandle = nullptr;
};

template <typename TRule>
TRule* AAVVMWorldSetting::CastRule(const FGameplayTag& RuleTag) const
{
	return Cast<TRule>(GetRule(RuleTag));
}

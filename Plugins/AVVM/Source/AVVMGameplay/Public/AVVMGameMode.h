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

#include "AVVMOnlineInterface.h"
#include "ModularGameMode.h"

#include "AVVMGameMode.generated.h"

class AAVVMWorldSetting;
class AGameStateBase;

/**
 *	Class description:
 *
 *	FAVVMGameModeRuleTagAggregator is a context struct that aggregate Rule tags relevant to AGameMode execution.
 */
USTRUCT(BlueprintType)
struct AVVMGAMEPLAY_API FAVVMGameModeRuleTagAggregator
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag MatchStartTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag MatchEndTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FGameplayTag MatchProgressTag = FGameplayTag::EmptyTag;
};

/**
 *	Class description:
 *
*	AAVVMGameMode is a GFP Receiver who registers with the GameFeatureFramework and react to GFP actions. It expects to receive system specific components depending on your project needs.
 *	
 *	IMPORTANT : Replicated systems SHOULD BE managed through a component added via GFP. Unreal's USubsystem derived classes CANNOT be replicated! Doing this approach makes for a more modular system
 *	due to component addition and removal no longer requiring hard references in the Actor derived class.
 *
 *	Note : for your UI systems. We expect to push an AVVMComponent with a pre-defined list of UAVVMPresenter class on it! Additionally, by implementing the api defined in the interface below,
 *	you will be able to forward gameplay information without creating dependencies between modules. However, it comes with a price due to interface dispatch!
 */
UCLASS()
class AVVMGAMEPLAY_API AAVVMGameMode : public AModularGameMode,
                                       public IAVVMOnlineBattlePassInterface,
                                       public IAVVMOnlineChallengesInterface,
                                       public IAVVMOnlineFriendInterface,
                                       public IAVVMOnlineIdentityInterface,
                                       public IAVVMOnlineMessagingInterface,
                                       public IAVVMOnlinePartyInterface,
                                       public IAVVMOnlineStoreInterface
{
	GENERATED_BODY()

public:
	AAVVMGameMode(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual bool HasMatchStarted() const override;
	virtual bool HasMatchEnded() const override;
	virtual bool IsMatchInProgress() const override;
	
	UFUNCTION()
	void OnGameStateSet(AGameStateBase* NewGameState);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Designers")
	FAVVMGameModeRuleTagAggregator RuleTagAggregator = FAVVMGameModeRuleTagAggregator();

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<AAVVMWorldSetting> WorldSetting = nullptr;
};

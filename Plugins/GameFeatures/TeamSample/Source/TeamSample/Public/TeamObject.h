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
#include "UObject/Object.h"

#include "TeamObject.generated.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

class APlayerState;
struct FAVVMPartyProxy;

/**
 *	Class description:
 *
 *	UTeamObject is a replicated object that encapsulate information about the team composition, and
 *	allow user actions to be executed on it based on the provided owning id.
 */
UCLASS(BlueprintType, NotBlueprintable)
class TEAMSAMPLE_API UTeamObject : public UObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context,
	                                          UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	UFUNCTION(BlueprintCallable)
	void SetTeam(const FGameplayTag& NewTeamTag);
	
	UFUNCTION(BlueprintCallable)
	void RegisterPlayer(const APlayerState* NewPlayer, const FString& NewPlayerUniqueNetId);

	static UTeamObject* Factory(UObject* Outer,
	                            const FAVVMPartyProxy& Party,
	                            const TArray<TWeakObjectPtr<APlayerState>>& Players);

protected:
	UFUNCTION()
	void OnRep_OnTeamCompositionChanged(const TArray<FString>& OldPlayerUniqueNetIds);

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_OnTeamCompositionChanged")
	TArray<FString> PlayerUniqueNetIds;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	FGameplayTag TeamTag = FGameplayTag::EmptyTag;
};

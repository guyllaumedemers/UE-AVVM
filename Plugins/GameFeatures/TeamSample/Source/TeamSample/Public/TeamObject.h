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

#include "AVVMNotificationSubsystem.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "TeamObject.generated.h"

class APlayerState;
struct FAVVMPartyProxy;
class UTeamRule;

/**
 *	Class description:
 *
 *	FTeamPayload is the Payload base class for caching data specific to team composition.
 */
USTRUCT()
struct TEAMSAMPLE_API FTeamPayload : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	FTeamPayload() = default;
	explicit FTeamPayload(const TArray<FString>& NewPlayerUniqueNetIds,
	                      const FGameplayTag& NewTeamTag);

	// @gdemers wrapper function template to avoid writing TInstancedStruct<FTeamPayload>::Make<T>
	template <typename TChild, typename... TArgs>
	static TInstancedStruct<FTeamPayload> Make(TArgs&&... Args);

	UPROPERTY(Transient, BlueprintReadWrite)
	FGameplayTag TeamTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> PlayerUniqueNetIds;
};

template <typename TChild, typename... TArgs>
TInstancedStruct<FTeamPayload> FTeamPayload::Make(TArgs&&... Args)
{
	return TInstancedStruct<FTeamPayload>::Make<TChild>(Forward<TArgs>(Args)...);
}

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
	void RegisterPlayerState(const APlayerState* NewPlayerState, const FString& NewPlayerUniqueNetId);
	
	UFUNCTION(BlueprintCallable)
	void UnRegisterPlayerState(const APlayerState* OldPlayerState);

	UFUNCTION(BlueprintCallable)
	TInstancedStruct<FTeamPayload> GetTeamPayload() const;

	UFUNCTION(BlueprintCallable)
	const FGameplayTag& GetTeamTag() const;

protected:
	UFUNCTION()
	void OnRep_OnTeamCompositionChanged(const TArray<FString>& OldPlayerUniqueNetIds);

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing="OnRep_OnTeamCompositionChanged")
	TArray<FString> PlayerUniqueNetIds;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	FGameplayTag TeamTag = FGameplayTag::EmptyTag;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	int32 PartyUniqueId = INDEX_NONE;

	friend class UTeamUtils;
};

/**
 *	Class description:
 *
 *	UTeamUtils expose reusable functional api. 
 */
UCLASS()
class TEAMSAMPLE_API UTeamUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void CreateOrAppendTeams(UObject* Outer,
	                                const TArray<TWeakObjectPtr<const APlayerState>>& UnassignedPlayerStates,
	                                const TArray<FAVVMPartyProxy>& NewParties,
	                                const UTeamRule* Rule,
	                                TArray<UTeamObject*>& OutTeams);

	static UTeamObject* CreateTeam(UObject* Outer,
	                               const TArray<TWeakObjectPtr<const APlayerState>>& UnassignedPlayerStates,
	                               const FAVVMPartyProxy& NewParty);

	static void AppendTeam(const TArray<TWeakObjectPtr<const APlayerState>>& UnassignedPlayerStates,
	                       const TArray<FString>& NewPlayerConnections,
	                       UTeamObject* NewTeam);

	static UTeamObject* FindTeam(const TArray<UTeamObject*>& NewTeams,
	                             const int32 NewPartyUniqueId);

	static UTeamObject* FindTeam(const TArray<UTeamObject*>& NewTeams,
	                             const APlayerState* NewPlayerState);
};

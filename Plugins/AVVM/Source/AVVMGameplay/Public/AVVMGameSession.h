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

#include "GameFramework/GameSession.h"

#include "AVVMGameSession.generated.h"

class APlayerState;

/**
 *	Class description:
 *	
 *	AAVVMGameSession is a server sided actor who tracking player connection to gameplay session. It may also exist
 *	on your client if the lobby is simulated (example: Offline lobby), and not within a shared level/hub.
 */
UCLASS()
class AVVMGAMEPLAY_API AAVVMGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	virtual void RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite) override;
	virtual void UnregisterPlayer(const APlayerController* ExitingPlayer) override;

	UFUNCTION(BlueprintCallable)
	static int32 Static_GetPlayerConnectionId(const UWorld* World,
	                                          const APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable)
	static int32 Static_GetPlayerProfileId(const UWorld* World,
	                                       const APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable)
	static int32 Static_GetPlayerPresetId(const UWorld* World,
	                                      const APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable)
	static TArray<int32> Static_GetPlayerPresetItems(const UWorld* World,
	                                                 const int32 ProfileId);

protected:
	static AAVVMGameSession* Get(const UWorld* World);
	int32 GetPlayerConnectionId(const APlayerState* PlayerState) const;
	int32 GetPlayerProfileId(const APlayerState* PlayerState) const;
	int32 GetPlayerPresetId(const APlayerState* PlayerState) const;
	TArray<int32> GetPlayerPresetItems(const int32 ProfileId) const;

	// @gdemers {FAVVMParty::UniqueId}
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 PartyId = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<FString/*{FAVVMPlayerConnection::UniqueNetId}*/, int32/*{FAVVMPlayerConnection::UniqueId}*/> PlayerConnectionIds;

	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<int32/*{FAVVMPlayerConnection::UniqueId}*/, int32/*{FAVVMPlayerProfile::UniqueId}*/> ProfileIds;

	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<int32/*{FAVVMPlayerProfile::UniqueId}*/, int32/*{FAVVMPlayerPreset::UniqueId}*/> PresetIds;

	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<int32/*{FAVVMPlayerProfile::UniqueId}*/, FString/*FAVVMPlayerProfile*/> ResolvedProfiles;

	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<int32/*{FAVVMPlayerPreset::UniqueId}*/, FString/*FAVVMPlayerPreset*/> ResolvedPresets;

private:
	void AddPlayer(const FString& UniqueNetId);
	void RemovePlayer(const FString& UniqueNetId);
	virtual int32 ResolveNewPlayerConnection(const FString& UniqueNetId);
	virtual void CleanupOldPlayerConnection(const int32 PlayerConnectionId);
};

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
#include "AVVMOnlinePlayer.h"

#include "AVVMOnlinePlayerProxy.generated.h"

/**
 *	Class description:
 *
 *	FAVVMPlayerAccountProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerAccountProxy : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerAccountProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FAVVMPlayerLoginContext}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Login = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Gamertag = FString();

	// @gdemers {FAVVMPlayerWallet}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Wallet = FString();

	// @gdemers {FAVVMPlayerProfileProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Profiles;

	// @gdemers {FAVVMPlayerPresetProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Presets;
};

/**
 *	Class description:
 *
 *	FAVVMPlayerProfileProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerProfileProxy : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerProfileProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may refer to a unique name tied to your playable character.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ProfileId = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Progression = FString();

	// @gdemers {InventorySample::FActorContentProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Inventories;

	// @gdemers {FAVVMPlayerChallenge}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Challenges;

	// @gdemers {FAVVMPlayerPresetProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString EquippedPreset = FString();
};

/**
 *	Class description:
 *
 *	FAVVMPlayerPresetProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerPresetProxy : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerPresetProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may refer to a unique name tied to your profile preset.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PresetId = FString();

	// @gdemers {FItemProxy}.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> EquippedItems;
};

/**
 *	Class description:
 *
 *	FAVVMPartyProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPartyProxy : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPartyProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may represent a party name.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PartyId = FString();

	// @gdemers NA, China, Russia, etc...
	UPROPERTY(Transient, BlueprintReadOnly)
	FString Region = FString();

	// @gdemers sub-identifier to the region.
	UPROPERTY(Transient, BlueprintReadOnly)
	FString District = FString();

	// @gdemers {FAVVMHostConfiguration}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString HostConfiguration = FString();

	// @gdemers {FAVVMPlayerConnectionProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> PlayerConnections;
};

/**
 *	Class description:
 *
 *	FAVVMPlayerConnectionProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerConnectionProxy : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	bool operator==(const FAVVMPlayerConnectionProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;

	// @gdemers convert using FUniqueNetIdString::Create()
	UPROPERTY(Transient, BlueprintReadWrite)
	FString UniqueNetId = FString();

	UPROPERTY(Transient, BlueprintReadWrite)
	EAVVMPlayerStatus PlayerStatus = EAVVMPlayerStatus::Default;

	// @gdemers {FAVVMPlayerProfileProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Profile = FString();
};

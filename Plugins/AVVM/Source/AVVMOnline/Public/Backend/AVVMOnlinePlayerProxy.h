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

	FAVVMPlayerAccountProxy() = default;
	FAVVMPlayerAccountProxy(const FAVVMPlayerAccountProxy&) = default;
	FAVVMPlayerAccountProxy(FAVVMPlayerAccountProxy&&) noexcept = default;
	FAVVMPlayerAccountProxy& operator=(const FAVVMPlayerAccountProxy&) = default;
	FAVVMPlayerAccountProxy& operator=(FAVVMPlayerAccountProxy&&) noexcept = default;
	bool operator==(const FAVVMPlayerAccountProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FAVVMPlayerLoginContext}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Login{};

	UPROPERTY(Transient, BlueprintReadWrite)
	FString Gamertag{};

	// @gdemers {FAVVMPlayerWalletProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Wallet{};

	// @gdemers {FAVVMPlayerProfileProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Profiles{};

	// @gdemers {FAVVMPlayerPresetProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Presets{};
};

/**
 *	Class description:
 *
 *	FAVVMPlayerWalletProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMPlayerWalletProxy : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	FAVVMPlayerWalletProxy() = default;
	FAVVMPlayerWalletProxy(const FAVVMPlayerWalletProxy&) = default;
	FAVVMPlayerWalletProxy(FAVVMPlayerWalletProxy&&) noexcept = default;
	FAVVMPlayerWalletProxy& operator=(const FAVVMPlayerWalletProxy&) = default;
	FAVVMPlayerWalletProxy& operator=(FAVVMPlayerWalletProxy&&) noexcept = default;
	bool operator==(const FAVVMPlayerWalletProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers {FAVVMCurrency} collection of currencies tied to player account.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> IrlMoneys{};
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

	FAVVMPlayerProfileProxy() = default;
	FAVVMPlayerProfileProxy(const FAVVMPlayerProfileProxy&) = default;
	FAVVMPlayerProfileProxy(FAVVMPlayerProfileProxy&&) noexcept = default;
	FAVVMPlayerProfileProxy& operator=(const FAVVMPlayerProfileProxy&) = default;
	FAVVMPlayerProfileProxy& operator=(FAVVMPlayerProfileProxy&&) noexcept = default;
	bool operator==(const FAVVMPlayerProfileProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may refer to a unique name tied to your playable character.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString ProfileId{};

	// @gdemers {FAVVMPlayerResource}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Inventories{};

	// @gdemers {FAVVMPlayerResource}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Skills{};

	// @gdemers {FAVVMPlayerChallenge}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> Challenges{};

	// @gdemers {FAVVMPlayerPresetProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString EquippedPreset{};
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

	FAVVMPlayerPresetProxy() = default;
	FAVVMPlayerPresetProxy(const FAVVMPlayerPresetProxy&) = default;
	FAVVMPlayerPresetProxy(FAVVMPlayerPresetProxy&&) noexcept = default;
	FAVVMPlayerPresetProxy& operator=(const FAVVMPlayerPresetProxy&) = default;
	FAVVMPlayerPresetProxy& operator=(FAVVMPlayerPresetProxy&&) noexcept = default;
	bool operator==(const FAVVMPlayerPresetProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may refer to a unique name tied to your profile preset.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PresetId{};

	// @gdemers {FAVVMPlayerResource}.
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> EquippedItems{};
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

	FAVVMPartyProxy() = default;
	FAVVMPartyProxy(const FAVVMPartyProxy&) = default;
	FAVVMPartyProxy(FAVVMPartyProxy&&) noexcept = default;
	FAVVMPartyProxy& operator=(const FAVVMPartyProxy&) = default;
	FAVVMPartyProxy& operator=(FAVVMPartyProxy&&) noexcept = default;
	bool operator==(const FAVVMPartyProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;

	// @gdemers may represent a party name.
	UPROPERTY(Transient, BlueprintReadWrite)
	FString PartyId{};

	// @gdemers NA, China, Russia, etc...
	UPROPERTY(Transient, BlueprintReadOnly)
	FString Region{};

	// @gdemers sub-identifier to the region.
	UPROPERTY(Transient, BlueprintReadOnly)
	FString District{};

	// @gdemers {FAVVMHostConfigurationProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString HostConfiguration{};

	// @gdemers {FAVVMPlayerConnectionProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	TArray<FString> PlayerConnections{};
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

	FAVVMPlayerConnectionProxy() = default;
	FAVVMPlayerConnectionProxy(const FAVVMPlayerConnectionProxy&) = default;
	FAVVMPlayerConnectionProxy(FAVVMPlayerConnectionProxy&&) noexcept = default;
	FAVVMPlayerConnectionProxy& operator=(const FAVVMPlayerConnectionProxy&) = default;
	FAVVMPlayerConnectionProxy& operator=(FAVVMPlayerConnectionProxy&&) noexcept = default;
	bool operator==(const FAVVMPlayerConnectionProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadOnly)
	int32 UniqueId = INDEX_NONE;

	// @gdemers convert using FUniqueNetIdString::Create()
	UPROPERTY(Transient, BlueprintReadWrite)
	FString UniqueNetId{};

	UPROPERTY(Transient, BlueprintReadWrite)
	EAVVMPlayerStatus PlayerStatus = EAVVMPlayerStatus::Default;

	// @gdemers {FAVVMPlayerProfileProxy}
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Profile{};
};

/**
 *	Class description:
 *
 *	FAVVMHostConfigurationProxy is the POD representation of the RESOLVED data returned by the backend following user request.
 */
USTRUCT(BlueprintType)
struct AVVMONLINE_API FAVVMHostConfigurationProxy : public FAVVMNotificationPayload
{
	GENERATED_BODY()

	FAVVMHostConfigurationProxy() = default;
	FAVVMHostConfigurationProxy(const FAVVMHostConfigurationProxy&) = default;
	FAVVMHostConfigurationProxy(FAVVMHostConfigurationProxy&&) noexcept = default;
	FAVVMHostConfigurationProxy& operator=(const FAVVMHostConfigurationProxy&) = default;
	FAVVMHostConfigurationProxy& operator=(FAVVMHostConfigurationProxy&&) noexcept = default;
	bool operator==(const FAVVMHostConfigurationProxy& Rhs) const;

	UPROPERTY(Transient, BlueprintReadWrite)
	int32 UniqueId = INDEX_NONE;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString GameMode{};

	UPROPERTY(Transient, BlueprintReadWrite)
	FString GameModeAdditiveOptions{};
};

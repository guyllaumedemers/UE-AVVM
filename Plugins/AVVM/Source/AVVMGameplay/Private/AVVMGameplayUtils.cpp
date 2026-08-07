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
#include "AVVMGameplayUtils.h"

#include "AVVMGameplaySettings.h"
#include "DataRegistrySubsystem.h"
#include "GameplayEffect.h"
#include "Data/AVVMActorIdentifierTableRow.h"
#include "Data/AVVMGameplayEffectIdentifierDataTableRow.h"
#include "Engine/NetConnection.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

int32 UAVVMGameplayUtils::GetActorUniqueIdentifierByActor(const AActor* Actor)
{
	const UClass* ActorClass = IsValid(Actor) ? Actor->GetClass() : nullptr;
	if (!IsValid(ActorClass))
	{
		return INDEX_NONE;
	}

	const FDataRegistryId ActorUniqueId =
	{
			UAVVMGameplaySettings::GetActorIdentifierRegistryType(),
			ActorClass->GetFName()
	};

	return UAVVMGameplayUtils::GetActorUniqueIdentifierByRegistryId(ActorUniqueId);
}

int32 UAVVMGameplayUtils::GetActorUniqueIdentifierByRegistryId(const FDataRegistryId& ActorIdentifierId)
{
	if (!ensureAlwaysMsgf(ActorIdentifierId.IsValid(),
	                      TEXT("Composed RegistryId isn't valid. You may be missing a reference in DeveloperSettings for the Actor Identifier RegistryType.")))
	{
		return INDEX_NONE;
	}

	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return INDEX_NONE;
	}

	// @gdemers imply we pre-cache our DT (which is fine! we can set that in editor, and is lightweight)
	const auto* RowValue = Subsystem->GetCachedItem<FAVVMActorIdentifierDataTableRow>(ActorIdentifierId);
	if (ensureAlwaysMsgf(RowValue != nullptr,
	                     TEXT("Invalid Row Entry. Make sure FAVVMActorIdentifierDataTableRow match the Data Table.")))
	{
		return RowValue->UniqueId;
	}
	else
	{
		return INDEX_NONE;
	}
}

int32 UAVVMGameplayUtils::GetGameplayEffectUniqueIdentifierByGameplayEffect(const UGameplayEffect* GameplayEffect)
{
	const UClass* GameplayEffectClass = IsValid(GameplayEffect) ? GameplayEffect->GetClass() : nullptr;
	if (!IsValid(GameplayEffectClass))
	{
		return INDEX_NONE;
	}

	const FDataRegistryId GameplayEffectUniqueId =
	{
			UAVVMGameplaySettings::GetGameplayEffectIdentifierRegistryType(),
			GameplayEffectClass->GetFName()
	};

	return UAVVMGameplayUtils::GetGameplayEffectUniqueIdentifierByRegistryId(GameplayEffectUniqueId);
}

int32 UAVVMGameplayUtils::GetGameplayEffectUniqueIdentifierByRegistryId(const FDataRegistryId& GameplayEffectIdentifierId)
{
	if (!ensureAlwaysMsgf(GameplayEffectIdentifierId.IsValid(),
	                      TEXT("Composed RegistryId isn't valid. You may be missing a reference in DeveloperSettings for the TreeNode Identifier RegistryType.")))
	{
		return INDEX_NONE;
	}

	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return INDEX_NONE;
	}

	// @gdemers unique identifier that represent a GameplayEffect
	const auto* RowValue = Subsystem->GetCachedItem<FAVVMGameplayEffectIdentifierDataTableRow>(GameplayEffectIdentifierId);
	if (ensureAlwaysMsgf(RowValue != nullptr,
	                     TEXT("Invalid Row Entry. Make sure FAVVMGameplayEffectIdentifierDataTableRow match the Data Table.")))
	{
		return RowValue->UniqueId;
	}
	else
	{
		return INDEX_NONE;
	}
}

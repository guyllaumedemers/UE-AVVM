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
#include "InventoryManagerSubsystem.h"

#include "ItemRandomizerRule.h"
#include "AVVMWorldSetting.h"
#include "InventorySettings.h"
#include "NativeGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// @gdemers WARNING : Careful about Server-Client mismatch. Server grants tags so this module has to be available there.
UE_DEFINE_GAMEPLAY_TAG(TAG_WORLD_RULE_ITEM_RANDOMIZER, "WorldRule.ItemRandomizer");

bool UInventoryManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	const bool bIsGameWorld = IsValid(World) ? World->IsGameWorld() : false;
	if (bIsGameWorld)
	{
		return !World->IsNetMode(NM_Client);
	}

	return false;
}

void UInventoryManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GetItemRandomizerRuleOnAuthority();
}

void UInventoryManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	StreamableHandle.Reset();
}

AActor* UInventoryManagerSubsystem::Static_CreateItemActor(const UWorld* World,
                                                           const UClass* ItemActorClass,
                                                           AActor* Outer)
{
	auto* InventoryManagerSubsystem = UInventoryManagerSubsystem::Get(World);
	if (IsValid(InventoryManagerSubsystem))
	{
		FActorSpawnParameters Params;
		Params.Owner = Outer;
		return InventoryManagerSubsystem->CreateItemActor(ItemActorClass, Params);
	}

	return nullptr;
}

void UInventoryManagerSubsystem::Static_Shutdown(const UWorld* World,
                                                 AActor* ItemActor)
{
	auto* InventoryManagerSubsystem = UInventoryManagerSubsystem::Get(World);
	if (IsValid(InventoryManagerSubsystem))
	{
		InventoryManagerSubsystem->Shutdown(ItemActor);
	}
}

TArray<UItemObject*> UInventoryManagerSubsystem::Static_GetRandomItems(const UWorld* World,
                                                                       const AActor* Actor,
                                                                       const TArray<UItemObject*>& PoolItems)
{
	TArray<UItemObject*> OutResults;

	auto* InventoryManagerSubsystem = UInventoryManagerSubsystem::Get(World);
	if (IsValid(InventoryManagerSubsystem))
	{
		OutResults = InventoryManagerSubsystem->GetRandomItems(Actor, PoolItems);
	}

	return OutResults;
}

void UInventoryManagerSubsystem::GetItemRandomizerRuleOnAuthority()
{
	const auto OnAsyncLoadComplete = [](const TWeakObjectPtr<UInventoryManagerSubsystem>& NewInventorySubsystem,
	                                    const TWeakObjectPtr<AAVVMWorldSetting>& NewWorldSettings)
	{
		if (!NewInventorySubsystem.IsValid() || !NewWorldSettings.IsValid())
		{
			return;
		}

		TSharedPtr<FStreamableHandle> Handle = NewInventorySubsystem->StreamableHandle;
		if (!Handle.IsValid())
		{
			return;
		}

		const auto* RuleClass = Cast<UClass>(Handle->GetLoadedAsset());
		if (!IsValid(RuleClass))
		{
			return;
		}

		NewInventorySubsystem->ItemRandomizerRule = NewWorldSettings->GetOrCreatePluginRule<UItemRandomizerRule>(TAG_WORLD_RULE_ITEM_RANDOMIZER, RuleClass);
	};

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	auto* WorldSettings = Cast<AAVVMWorldSetting>(World->GetWorldSettings());
	if (IsValid(WorldSettings))
	{
		const auto Callback = FStreamableDelegate::CreateWeakLambda(this, OnAsyncLoadComplete, TWeakObjectPtr(this), TWeakObjectPtr(WorldSettings));
		StreamableHandle = WorldSettings->AsyncLoadPluginRule(UInventorySettings::GetItemRandomizerRuleClass(), Callback);
	}
}

UInventoryManagerSubsystem* UInventoryManagerSubsystem::Get(const UWorld* World)
{
	return UWorld::GetSubsystem<UInventoryManagerSubsystem>(World);
}

AActor* UInventoryManagerSubsystem::CreateItemActor(const UClass* ItemActorClass,
                                                    const FActorSpawnParameters& SpawnParams)
{
	return Factory(ItemActorClass, SpawnParams);
}

AActor* UInventoryManagerSubsystem::Factory(const UClass* ItemActorClass,
                                            const FActorSpawnParameters& SpawnParams)
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		// @gdemers another good example of const-ness issue in the engine. theres no point in violating bitwise const-ness or logical const-ness and yet
		// the engine require a UClass* to be non-const.
		return World->SpawnActor(const_cast<UClass*>(ItemActorClass), &FTransform::Identity, SpawnParams);
	}

	return nullptr;
}

void UInventoryManagerSubsystem::Shutdown(AActor* ItemActor)
{
	if (IsValid(ItemActor))
	{
		ItemActor->Destroy();
	}
}

TArray<UItemObject*> UInventoryManagerSubsystem::GetRandomItems(const AActor* Actor,
                                                                const TArray<UItemObject*>& PoolItems)
{
	if (ItemRandomizerRule.IsValid())
	{
		const TArray<UItemObject*> OutResults = ItemRandomizerRule->GetRandomSubset(Actor, PoolItems);
		return OutResults;
	}
	else
	{
		return {};
	}
}

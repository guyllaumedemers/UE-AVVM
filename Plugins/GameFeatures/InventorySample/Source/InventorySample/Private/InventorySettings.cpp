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
#include "InventorySettings.h"

const FGameplayTag& UInventorySettings::GetPendingSpawnTag()
{
	return GetDefault<UInventorySettings>()->PendingSpawnTag;
}

const FGameplayTag& UInventorySettings::GetInstancedTag()
{
	return GetDefault<UInventorySettings>()->InstancedTag;
}

const FGameplayTag& UInventorySettings::GetEquippedTag()
{
	return GetDefault<UInventorySettings>()->EquippedTag;
}

const FGameplayTag& UInventorySettings::GetActiveTag()
{
	return GetDefault<UInventorySettings>()->ActiveTag;
}

const FGameplayTagContainer& UInventorySettings::GetEmptyItemCount_BlockedActions()
{
	return GetDefault<UInventorySettings>()->EmptyItemCount_BlockedActions;
}

const FGameplayTagContainer& UInventorySettings::GetFullInventory_BlockedActions()
{
	return GetDefault<UInventorySettings>()->FullInventory_BlockedActions;
}

const FName UInventorySettings::GetItemMaxStackCount(const FGameplayTag& StackCategoryTag)
{
	const auto& NewItemMaxStackCounts = GetDefault<UInventorySettings>()->ItemMaxStackCounts;
	const bool bDoesContains = NewItemMaxStackCounts.Contains(StackCategoryTag);
	if (bDoesContains)
	{
		return NewItemMaxStackCounts[StackCategoryTag];
	}

	static const FName Empty = NAME_None;
	return Empty;
}

const TSoftObjectPtr<UDataTable>& UInventorySettings::GetItemMaxStackCountDataTable()
{
	return GetDefault<UInventorySettings>()->ItemMaxStackCountDataTable;
}

const FGameplayTagContainer& UInventorySettings::GetStorageRuleset()
{
	return GetDefault<UInventorySettings>()->StorageRuleset;
}

const FGameplayTagContainer& UInventorySettings::GetHoldingRuleset()
{
	return GetDefault<UInventorySettings>()->HoldingRuleset;
}

const FGameplayTagContainer& UInventorySettings::GetEquippedRuleset()
{
	return GetDefault<UInventorySettings>()->EquippedRuleset;
}

const FGameplayTagContainer& UInventorySettings::GetPassiveRuleset()
{
	return GetDefault<UInventorySettings>()->PassiveRuleset;
}

const FGameplayTagContainer& UInventorySettings::GetOffensiveRuleset()
{
	return GetDefault<UInventorySettings>()->OffensiveRuleset;
}

const FGameplayTagContainer& UInventorySettings::GetDefensiveRuleset()
{
	return GetDefault<UInventorySettings>()->DefensiveRuleset;
}

const FGameplayTagContainer& UInventorySettings::GetConsumableRuleset()
{
	return GetDefault<UInventorySettings>()->ConsumableRuleset;
}

const FString& UInventorySettings::GetAppDataDirPath()
{
	return GetDefault<UInventorySettings>()->AppDataDirPath;
}

const TSoftObjectPtr<UDataTable>& UInventorySettings::GetDefaultProviderInventories()
{
	return GetDefault<UInventorySettings>()->DefaultProviderInventories;
}

const FGameplayTag& UInventorySettings::GetStorageTypeTag()
{
	return GetDefault<UInventorySettings>()->StorageTypeTag;
}

const FGameplayTag& UInventorySettings::GetAttachmentTypeTag()
{
	return GetDefault<UInventorySettings>()->AttachmentTypeTag;
}

const FGameplayTagContainer& UInventorySettings::GetItemActorSpawnConditions()
{
	return GetDefault<UInventorySettings>()->ItemActorSpawnConditions;
}

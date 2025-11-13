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

bool UInventoryManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const auto* World = Cast<UWorld>(Outer);
	if (IsValid(World))
	{
		return !World->IsNetMode(NM_Client);
	}

	return false;
}

AActor* UInventoryManagerSubsystem::Static_CreateItemActor(const UWorld* World,
                                                           const UClass* ItemActorClass,
                                                           AActor* Owner)
{
	const auto* InventoryManagerSubsystem = UInventoryManagerSubsystem::GetSubsystem(World);
	if (IsValid(InventoryManagerSubsystem))
	{
		FActorSpawnParameters Params;
		Params.Owner = Owner;
		return InventoryManagerSubsystem->CreateItemActor(ItemActorClass, Params);
	}

	return nullptr;
}

UInventoryManagerSubsystem* UInventoryManagerSubsystem::GetSubsystem(const UWorld* World)
{
	return UWorld::GetSubsystem<UInventoryManagerSubsystem>(World);
}

AActor* UInventoryManagerSubsystem::CreateItemActor(const UClass* ItemActorClass,
                                                    const FActorSpawnParameters& SpawnParams) const
{
	return Factory(ItemActorClass, SpawnParams);
}

AActor* UInventoryManagerSubsystem::Factory(const UClass* ItemActorClass,
                                            const FActorSpawnParameters& SpawnParams) const
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

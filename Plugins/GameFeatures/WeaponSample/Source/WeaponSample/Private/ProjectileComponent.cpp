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
#include "ProjectileComponent.h"

#include "AVVMGameplayUtils.h"
#include "NonReplicatedProjectileActor.h"
#include "ProjectileManagerSubsystem.h"
#include "WeaponSample.h"
#include "Data/ProjectileDefinitionDataAsset.h"
#include "Engine/AssetManager.h"

void UProjectileComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningOuter = GetTypedOuter<AActor>();
	ProjectileManagerSubsystem = UProjectileManagerSubsystem::GetSubsystem(GetWorld());
}

void UProjectileComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ProjectileManagerSubsystem.Reset();
	ProjectileTemplates.Reset();
	StreamableHandle.Reset();
	OwningOuter.Reset();
}

UProjectileComponent* UProjectileComponent::GetActorComponent(const AActor* NewActor)
{
	return IsValid(NewActor) ? NewActor->GetComponentByClass<UProjectileComponent>() : nullptr;
}

void UProjectileComponent::Fire(const FGameplayTag& FiringModeTag,
                                const FTransform& AimTransform) const
{
	const bool bDoesContains = ProjectileTemplates.Contains(FiringModeTag);
	if (!bDoesContains)
	{
		return;
	}

	if (ProjectileManagerSubsystem.IsValid())
	{
		const FProjectileFiringMode& ProjectileMode = ProjectileTemplates[FiringModeTag];
		ProjectileManagerSubsystem->CreateProjectile(ProjectileMode.ProjectileClass.Get(),
		                                             ProjectileMode.ProjectileParams,
		                                             AimTransform);
	}
}

void UProjectileComponent::SetupProjectiles(const TArray<UObject*>& NewResources)
{
	const AActor* Outer = OwningOuter.Get();
	if (!ensureAlwaysMsgf(IsValid(Outer), TEXT("Invalid Outer!")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(!NewResources.IsEmpty(),
						  TEXT("Attempting to load invalid Attachment set on Outer \"%s\"."),
						  *Outer->GetName()))
	{
		return;
	}

	const auto* IsServerOrClientString = UAVVMGameplayUtils::PrintNetSource(Outer).GetData();
	
	TArray<FSoftObjectPath> DeferredItems;
	for (const UObject* Resource : NewResources)
	{
		const auto* ProjectileAsset = Cast<UProjectileDefinitionDataAsset>(Resource);
		if (!IsValid(ProjectileAsset))
		{
			continue;
		}

		UE_LOG(LogWeaponSample,
			   Log,
			   TEXT("Executed from \"%s\". New \"%s\" Recorded."),
			   IsServerOrClientString,
			   *ProjectileAsset->GetName());

		DeferredItems.Add(ProjectileAsset->GetProjectileClass().ToSoftObjectPath());

		// @gdemers cache resources based on Firing mode.
		FProjectileFiringMode& Out = ProjectileTemplates.FindOrAdd(ProjectileAsset->GetProjectileFiringModeTag());
		Out.ProjectileParams = ProjectileAsset->GetProjectileParams();
	}

	if (!DeferredItems.IsEmpty())
	{
		const auto Callback = FStreamableDelegate::CreateUObject(this, &UProjectileComponent::OnSoftObjectAcquired);
		StreamableHandle = UAssetManager::Get().LoadAssetList(DeferredItems, Callback);
	}
}

void UProjectileComponent::OnSoftObjectAcquired()
{
	if (!StreamableHandle.IsValid())
	{
		return;
	}

	TArray<UObject*> OutResources;
	StreamableHandle->GetLoadedAssets(OutResources);

	for (const UObject* Resource : OutResources)
	{
		const auto* ProjectileClass = Cast<UClass>(Resource);
		if (!IsValid(ProjectileClass))
		{
			continue;
		}

		// TODO @gdemers confirm that this is valid cdo retrieval. not sure if cdo from UClass is valid operation.
		const auto* CDO = ProjectileClass->GetDefaultObject<ANonReplicatedProjectileActor>();
		if (!IsValid(CDO))
		{
			continue;
		}

		const FGameplayTag& ProjectileFiringMode = CDO->ProjectileFiringMode;

		const bool bDoesContains = ProjectileTemplates.Contains(ProjectileFiringMode);
		if (bDoesContains)
		{
			FProjectileFiringMode& Out = ProjectileTemplates[ProjectileFiringMode];
			Out.ProjectileClass = ProjectileClass;
		}
	}
}
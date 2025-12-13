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
#include "Inputs/AVVMGameFeatureAction_AddInputMappingContext.h"

#include "GameFeaturesSubsystem.h"

void UAVVMGameFeatureAction_AddInputMappingContext::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	if (!IsValid(GEngine))
	{
		return;
	}

	FContextHandles& Handles = ContextHandles.FindOrAdd(Context);
	Handles.GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &UAVVMGameFeatureAction_AddInputMappingContext::HandleGameInstanceStart, FGameFeatureStateChangeContext(Context));

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		const bool bShouldApply = Context.ShouldApplyToWorldContext(WorldContext);
		if (bShouldApply)
		{
			AddToWorld(WorldContext, Handles);
		}
	}
}

void UAVVMGameFeatureAction_AddInputMappingContext::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FContextHandles& Handles = ContextHandles.FindOrAdd(Context);
	FWorldDelegates::OnStartGameInstance.Remove(Handles.GameInstanceStartHandle);
	
	Handles.IMCRequestHandles.Empty();
}

#if WITH_EDITOR
EDataValidationResult UAVVMGameFeatureAction_AddInputMappingContext::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);
	if (IMCs.IsEmpty())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("UAVVMGameFeatureAction_AddInputMappingContext", "", "IMCs empty!"));
	}

	return Result;
}
#endif

void UAVVMGameFeatureAction_AddInputMappingContext::HandleGameInstanceStart(UGameInstance* GameInstance,
                                                                            FGameFeatureStateChangeContext ChangeContext)
{
	if (!IsValid(GameInstance))
	{
		return;
	}

	FWorldContext* WorldContext = GameInstance->GetWorldContext();
	if (WorldContext == nullptr)
	{
		return;
	}

	const bool bShouldApply = ChangeContext.ShouldApplyToWorldContext(*WorldContext);
	if (!bShouldApply)
	{
		return;
	}

	FContextHandles* Handles = ContextHandles.Find(ChangeContext);
	if (ensureAlwaysMsgf(Handles, TEXT("Invalid Context.")))
	{
		AddToWorld(*WorldContext, *Handles);
	}
}

void UAVVMGameFeatureAction_AddInputMappingContext::AddToWorld(const FWorldContext& WorldContext,
                                                               FContextHandles& Handles)
{
	const UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	const UWorld* World = WorldContext.World();

	if (!IsValid(World) || !World->IsGameWorld() || World->IsNetMode(NM_DedicatedServer) || !IsValid(GameInstance))
	{
		return;
	}

	for (auto Iterator = GameInstance->GetLocalPlayerIterator(); Iterator; ++Iterator)
	{
		auto* GFIMCM = ULocalPlayer::GetSubsystem<UAVVMGameFrameworkInputMappingContextManager>(*Iterator);
		if (!IsValid(GFIMCM))
		{
			continue;
		}

		// TODO @gdemers push this further, and define a UAVVMWorldRule, for IMC
		// that can be blocked per-world.
		for (const TSoftObjectPtr<UInputMappingContext>& IMC : IMCs)
		{
			TSharedPtr<FAVVMIMCRequestHandle> IMCHandle = GFIMCM->AddIMCRequest(World, *Iterator, IMC);
			Handles.IMCRequestHandles.Add(IMCHandle);
		}
	}
}

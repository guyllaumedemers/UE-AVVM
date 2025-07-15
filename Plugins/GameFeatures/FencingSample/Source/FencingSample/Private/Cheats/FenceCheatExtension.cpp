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
#include "Cheats/FenceCheatExtension.h"

#include "ActorFenceComponent.h"
#include "AVVMGameplay.h"
#include "FenceManagerSubsystem.h"

#if WITH_AVVM_DEBUGGER
#include "Containers/StringFwd.h"
#include <imgui.h>
#endif

void UFenceCheatExtension::AddedToCheatManager_Implementation()
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Registering %s"),
	       *GetName());

	bHasFencesChanged = true;

#if WITH_AVVM_DEBUGGER
	FAVVMDebuggerModule::Get().GetDebuggerContext().AddDescriptor(this);
#endif
}

void UFenceCheatExtension::RemovedFromCheatManager_Implementation()
{
	UE_LOG(LogGameplay,
	       Log,
	       TEXT("Unregistering %s"),
	       *GetName());

	UFenceManagerSubsystem* NewFenceSubsystem = UFenceManagerSubsystem::Get(this);
	if (IsValid(NewFenceSubsystem))
	{
		NewFenceSubsystem->OnFencesCountModified.RemoveAll(this);
	}

#if WITH_AVVM_DEBUGGER
	FAVVMDebuggerModule::Get().GetDebuggerContext().RemoveDescriptor(this);
#endif
}

#if WITH_AVVM_DEBUGGER
void UFenceCheatExtension::Draw()
{
	if (!ImGui::CollapsingHeader("Cheats [Fence]"))
	{
		return;
	}

	const char* const FenceInstances = LazyGatherFenceInstances(bHasFencesChanged);

	{
		ImGui::Text("Active Fences");
		ImGui::Separator();

		ImGui::BeginGroup();

		static int32 CurrentFenceIndex = 0;
		static const char* const FenceInstancesTitle = "Fence Instances";
		ImGui::Combo(FenceInstancesTitle, &CurrentFenceIndex, FenceInstances);

		ImGui::SameLine();

		if (ImGui::Button("Lower Fence"))
		{
			LowerFence(CurrentFenceIndex);
		}

		ImGui::EndGroup();
	}
}

const char* UFenceCheatExtension::LazyGatherFenceInstances(bool& bForceGathering)
{
	static TAnsiStringBuilder<512> StringBuilder;
	static bool bWasInitialized = false;

	if (bWasInitialized && !bForceGathering)
	{
		return *StringBuilder;
	}

	UFenceManagerSubsystem* NewFenceSubsystem = UFenceManagerSubsystem::Get(this);
	if (!IsValid(NewFenceSubsystem))
	{
		return *StringBuilder;
	}

	const bool bIsAlreadyBound = NewFenceSubsystem->OnFencesCountModified.IsAlreadyBound(this, &UFenceCheatExtension::OnFencesCountModified);
	if (!bIsAlreadyBound)
	{
		NewFenceSubsystem->OnFencesCountModified.AddUniqueDynamic(this, &UFenceCheatExtension::OnFencesCountModified);
	}

	TArray<TWeakObjectPtr<const UActorFenceComponent>> Fences = NewFenceSubsystem->Fences;

	FenceComponents.Reset(Fences.Num());
	StringBuilder.Reset();

	for (auto Iterator = Fences.CreateIterator(); Iterator; ++Iterator)
	{
		if (!Iterator->IsValid())
		{
			Iterator.RemoveCurrentSwap();
		}
		else
		{
			const UActorFenceComponent* Fence = Iterator->Get();
			const FString Name = Fence->GetName();
			StringBuilder.Append(Name.GetCharArray());
			StringBuilder.Append("\0"/*enforce null termination between entries*/);
			FenceComponents.Add(Fence);
		}
	}

	bWasInitialized = true;
	bForceGathering = false;
	return *StringBuilder;
}
#endif

void UFenceCheatExtension::OnFencesCountModified()
{
	bHasFencesChanged = true;
}

void UFenceCheatExtension::LowerFence(const int32 NewIndex)
{
	const bool bIsValidIndex = FenceComponents.IsValidIndex(NewIndex);
	if (!bIsValidIndex)
	{
		return;
	}

	const UActorFenceComponent* FenceComponent = FenceComponents[NewIndex].Get();
	if (IsValid(FenceComponent))
	{
		FenceComponent->ForceLowering();
	}
}

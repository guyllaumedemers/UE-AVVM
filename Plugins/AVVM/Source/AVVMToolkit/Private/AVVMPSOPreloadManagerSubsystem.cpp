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
#include "AVVMPSOPreloadManagerSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"

FAVVMPSOPreloadToken UAVVMPSOPreloadManagerSubsystem::Static_PreloadPSO(const UGameInstance* GameInstance,
                                                                        const TArray<TSoftObjectPtr<UWorld>>& LevelInstances)
{
	auto* Subsystem = UAVVMPSOPreloadManagerSubsystem::Get(GameInstance);
	if (IsValid(Subsystem))
	{
		const FAVVMPSOPreloadToken NewToken = Subsystem->PreloadPSO(LevelInstances);
		return NewToken;
	}

	static FAVVMPSOPreloadToken Empty;
	return Empty;
}

void UAVVMPSOPreloadManagerSubsystem::Static_UnloadPSOHandle(const UGameInstance* GameInstance,
                                                             const FAVVMPSOPreloadToken& Token)
{
	auto* Subsystem = UAVVMPSOPreloadManagerSubsystem::Get(GameInstance);
	if (IsValid(Subsystem))
	{
		Subsystem->UnloadPSOHandle(Token);
	}
}

UAVVMPSOPreloadManagerSubsystem* UAVVMPSOPreloadManagerSubsystem::Get(const UGameInstance* GameInstance)
{
	return UGameInstance::GetSubsystem<UAVVMPSOPreloadManagerSubsystem>(GameInstance);
}

FAVVMPSOPreloadToken UAVVMPSOPreloadManagerSubsystem::PreloadPSO(const TArray<TSoftObjectPtr<UWorld>>& LevelInstances)
{
	TArray<FSoftObjectPath> OutPaths;
	for (const TSoftObjectPtr<UWorld>& LevelInstance : LevelInstances)
	{
		OutPaths.Add(LevelInstance.ToSoftObjectPath());
	}
	
	// TODO @gdemers We need a more thorough approach to triggering PSO compilation than just level loading, otherwise
	// we will load in memory level instances EVERYTIME we make a request from the above call (which is only valid the first time).

	const TSharedPtr<FStreamableHandle> NewHandle = UAssetManager::Get().LoadAssetList(OutPaths);
	const FAVVMPSOPreloadToken NewToken = FAVVMPSOPreloadToken::Make();

	PSOHandles.FindOrAdd(NewToken.UniqueId, NewHandle);

	return NewToken;
}

void UAVVMPSOPreloadManagerSubsystem::UnloadPSOHandle(const FAVVMPSOPreloadToken& Token)
{
	const TSharedPtr<FStreamableHandle>* SearchResult = PSOHandles.Find(Token.UniqueId);
	if ((SearchResult != nullptr) && SearchResult->IsValid())
	{
		(*SearchResult)->ReleaseHandle();
		PSOHandles.Remove(Token.UniqueId);
	}
}

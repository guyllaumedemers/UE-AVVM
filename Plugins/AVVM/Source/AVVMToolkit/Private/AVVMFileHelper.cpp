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
#include "AVVMFileHelper.h"

#include "AVVMSaveGame.h"
#include "Engine/Engine.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"

TStrongObjectPtr<UAVVMFileHelper> UAVVMFileHelper::gFileHelper = nullptr;

void UAVVMFileHelper::Static_SetSaveGameSlot(const FName SaveGameSlot)
{
	auto* FileHelper = Get();
	if (ensureAlwaysMsgf(IsValid(FileHelper), TEXT("Invalid File Helper")))
	{
		FileHelper->SetSaveGameSlot(SaveGameSlot);
	}
}

UAVVMSaveGame* UAVVMFileHelper::Static_GetSetSaveGame()
{
	auto* FileHelper = Get();
	return ensureAlwaysMsgf(IsValid(FileHelper), TEXT("Invalid File Helper")) ? FileHelper->GetSetSaveGame() : nullptr;
}

UAVVMFileHelper* UAVVMFileHelper::Get()
{
	if (!gFileHelper.IsValid())
	{
		auto* Instance = NewObject<UAVVMFileHelper>();
		gFileHelper.Reset(Instance);
	}

	return gFileHelper.Get();
}

FName UAVVMFileHelper::GetSetSaveGameSlot()
{
	if (ActiveSaveGameSlot.IsNone())
	{
		// @gdemers default a random slot name until
		// user implement a save slot selector.
		ActiveSaveGameSlot = TEXT("Empty");
	}

	return ActiveSaveGameSlot;
}

void UAVVMFileHelper::SetSaveGameSlot(const FName SaveGameSlot)
{
	ActiveSaveGameSlot = SaveGameSlot;
}

UAVVMSaveGame* UAVVMFileHelper::GetSetSaveGame()
{
	if (!SaveGameObject.IsValid())
	{
#if WITH_EDITOR
		auto* NewSameObject = NewObject<UAVVMSaveGame>(this);
		const FString SaveGameSlot = GetSetSaveGameSlot().ToString();
		NewSameObject->SetSaveSlotName(SaveGameSlot);
#else
		APlayerController* PC = UGameplayStatics::GetPlayerController(GEngine, 0);
		const FString SaveGameSlot = GetSetSaveGameSlot().ToString();
		auto* NewSameObject = Cast<UAVVMSaveGame>(ULocalPlayerSaveGame::LoadOrCreateSaveGameForLocalPlayer(UAVVMSaveGame::StaticClass(), PC, SaveGameSlot));
#endif
		SaveGameObject.Reset(NewSameObject);
	}

	return SaveGameObject.Get();
}

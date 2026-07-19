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

#include "GameFramework/SaveGame.h"

#include "AVVMSaveGame.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMSaveGame 
 */
UCLASS()
class AVVMTOOLKIT_API UAVVMSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()

public:
	static FStringView Static_GetSetFileContent(const FName PayloadType,
	                                            const TFunction<FString()>& GenerateDefaultContent,
	                                            const bool bShouldDelete = false);

	static void Static_Serialize(const FName PayloadType,
	                             const FString& NewPayload);

	virtual void HandlePreSave() override;
	virtual void HandlePostLoad() override;
	virtual void HandlePostSave(bool bSuccess) override;

protected:
	FStringView GetSetFileContent(const FName PayloadType,
								  const TFunction<FString()>& GenerateDefaultContent,
								  const bool bShouldDelete);

	// @gdemers _v2 prevent function name shadowing in base UObject class.
	void Serialize_v2(const FName PayloadType, const FString& NewPayload);
	void MarkFileDirty();

	// @gdemers for hot-reload
	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<FName, FString> PrevPayloadPerType;

	// @gdemers property that reference ALL payload delta representation for progression tracking
	// in story mode.
	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<FName, FString> CurrPayloadPerType;

	UPROPERTY(Transient, BlueprintReadOnly)
	double SessionStartTime = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly)
	double TotalPlayTime = 0.f;
	
	UPROPERTY(Transient,  BlueprintReadOnly)
	bool bIsMarkedDirty = false;
};

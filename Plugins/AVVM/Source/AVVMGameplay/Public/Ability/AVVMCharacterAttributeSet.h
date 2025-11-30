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

#include "AVVMAttributeSet.h"

#include "AVVMCharacterAttributeSet.generated.h"

/**
 *	Class description:
 *	
 *	UAVVMCharacterAttributeSet is a data type that initialize properties on a Character ASC.
 */
UCLASS(Blueprintable)
class AVVMGAMEPLAY_API UAVVMCharacterAttributeSet : public UAVVMAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Init() override;

	ATTRIBUTE_ACCESSORS_BASIC(UAVVMCharacterAttributeSet, Movement_JumpHeight);
	ATTRIBUTE_ACCESSORS_BASIC(UAVVMCharacterAttributeSet, Movement_WalkSpeed);
	ATTRIBUTE_ACCESSORS_BASIC(UAVVMCharacterAttributeSet, Movement_SprintSpeed);
	ATTRIBUTE_ACCESSORS_BASIC(UAVVMCharacterAttributeSet, Movement_DashDistance);
	ATTRIBUTE_ACCESSORS_BASIC(UAVVMCharacterAttributeSet, Movement_RollDistance);
	ATTRIBUTE_ACCESSORS_BASIC(UAVVMCharacterAttributeSet, Movement_SlideDistance);

protected:
	// ------------------- FCharacterProperties ------------------- //
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FCharacterProperties")
	FGameplayAttributeData Movement_JumpHeight = FGameplayAttributeData();

	// @gdemers environment that modify speed properties such as : in water, in powder snow, etc...
	// should apply modifiers to WalkSpeed/SprintSpeed depending on input pressed.
	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FCharacterProperties")
	FGameplayAttributeData Movement_WalkSpeed = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FCharacterProperties")
	FGameplayAttributeData Movement_SprintSpeed = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FCharacterProperties")
	FGameplayAttributeData Movement_DashDistance = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FCharacterProperties")
	FGameplayAttributeData Movement_RollDistance = FGameplayAttributeData();

	UPROPERTY(Transient, BlueprintReadOnly, Replicated, Category="Designers|FCharacterProperties")
	FGameplayAttributeData Movement_SlideDistance = FGameplayAttributeData();
};

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

#include "GameUIManagerSubsystem.h"

#include "DerivedGameUIManagerSubsystem.generated.h"

/**
*	Class Description :
 *
 *	UDerivedGameUIManagerSubsystem derived from required base and support CommonGame policy system.
 */
UCLASS()
class UISAMPLE_API UDerivedGameUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

	/**
	 *	Note : UGameUIPolicy Class has to be set manually in the DefaultGame.ini file which
	 *	isn't the preferred way of configuring Project Settings.
	 *
	*	UGameInstance Class also has to be updated to be of derived type UCommonGameInstance for this system
	 *	to work correctly! - Require deriving from BP as UCommonGameInstance is made Abstract (which shouldn't be required...) and
	 *	updating the project settings.
	 *
	*	APlayerController Class also has to be updated to be of derived type ACommonPlayerController for this system
	 *	to work correctly! Our Local Player will not have a valid Controller when first trying to create the PrimaryGameLayout. This action will be defered
	 *	and handled by the derived Controller class. (Created a Custom GameMode and reference ACommonPlayerController)
	 *
	 *	ULocalPlayer Class also has to be updated to be of derived type UCommonLocalPlayer for this system
	 *	to work correctly!
	 */
};

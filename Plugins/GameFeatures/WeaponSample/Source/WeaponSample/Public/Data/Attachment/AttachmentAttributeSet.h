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

#include "Ability/AVVMAttributeSet.h"

#include "AttachmentAttributeSet.generated.h"

/**
 *	Class description:
 *	
 *	UAttachmentSight_AttributeSet is a data type that initialize properties on an Actor ASC for an attachment.
 *	
 *		* Iron Sight
 *		* Optic Sight
 *		* Whatever else there is ?
 */
UCLASS()
class WEAPONSAMPLE_API UAttachmentSight_AttributeSet : public UAVVMAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Init() override;
};

/**
 *	Class description:
 *	
 *	UAttachmentMagazine_AttributeSet is a data type that initialize properties on an Actor ASC for an attachment.
 *	
 *		* Single
 *		* Double
 */
UCLASS()
class WEAPONSAMPLE_API UAttachmentMagazine_AttributeSet : public UAVVMAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Init() override;
};

/**
 *	Class description:
 *	
 *	UAttachmentBarrel_AttributeSet is a data type that initialize properties on an Actor ASC for an attachment.
 *
 *		* Muzzle
 *		* Silencer
 *		* Whatever else there is ?
 */
UCLASS()
class WEAPONSAMPLE_API UAttachmentBarrel_AttributeSet : public UAVVMAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Init() override;
};

/**
 *	Class description:
 *	
 *	UAttachmentHandGuard_AttributeSet is a data type that initialize properties on an Actor ASC for an attachment.
 *	
 *		* Rocket launcher
 *		* Foregrip
 */
UCLASS()
class WEAPONSAMPLE_API UAttachmentHandGuard_AttributeSet : public UAVVMAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Init() override;
};

/**
 *	Class description:
 *	
 *	UAttachmentGrip_AttributeSet is a data type that initialize properties on an Actor ASC for an attachment.
 */
UCLASS()
class WEAPONSAMPLE_API UAttachmentGrip_AttributeSet : public UAVVMAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Init() override;
};

/**
 *	Class description:
 *	
 *	UAttachmentButtstock_AttributeSet is a data type that initialize properties on an Actor ASC for an attachment.
 *	
 *		* Stock
 */
UCLASS()
class WEAPONSAMPLE_API UAttachmentButtstock_AttributeSet : public UAVVMAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Init() override;
};

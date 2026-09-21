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
#include "Resources/SkillTreeStubDataProvider.h"

#include "DataRegistrySubsystem.h"
#include "SkillTreeSettings.h"
#include "SkillTreeUtils.h"
#include "Backend/AVVMOnlinePlayer.h"
#include "Data/SkillTreeProviderTableRow.h"

USkillTreeStubDataProvider::USkillTreeStubDataProvider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (IsTemplate(RF_ClassDefaultObject))
	{
		UAVVMOnlineStubDataHelper::Static_RegisterPropertyProvider(TAG_AVVMONLINE_BACKEND_STUB_SKILLS, GetClass());
	}
}

TArray<int32> USkillTreeStubDataProvider::MakePropertyStubData() const
{
	const auto* Subsystem = UDataRegistrySubsystem::Get();
	if (!IsValid(Subsystem))
	{
		return TArray<int32>{};
	}

	const auto* Row = Subsystem->GetCachedItem<FSkillTreeProviderTableRow>(USkillTreeSettings::GetStubDataProviderSkillTreeId());
	if (!ensureAlwaysMsgf(Row != nullptr, TEXT("Invalid Stub Data Provider.")))
	{
		return TArray<int32>{};
	}

	TArray<int32> PrivateTreeNodeIds{};
	for (const auto& SkillTreeNodeDefinition : Row->SkillTreeNodeDefinitions)
	{
		const int32 RelationshipBitmask = FSkillTreeNodeDefinition::Static_GetRelationshipBitmask(SkillTreeNodeDefinition);
		const int32 PrivateTreeNodeId = USkillTreeUtils::CreateDefaultPrivateTreeNodeId(SkillTreeNodeDefinition.SkillTreeNodeId,
		                                                                                RelationshipBitmask,
		                                                                                SkillTreeNodeDefinition.InstancedId,
		                                                                                SkillTreeNodeDefinition.EffectLevel);

		PrivateTreeNodeIds.Add(PrivateTreeNodeId);
	}

	return PrivateTreeNodeIds;
}

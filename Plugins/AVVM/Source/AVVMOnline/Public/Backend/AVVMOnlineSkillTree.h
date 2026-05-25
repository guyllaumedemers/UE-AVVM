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
// ReSharper disable CppIncompleteSwitchStatement
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#pragma once

// @gdemers Skill Tree Nodes are referenced by {FAVVMPlayerResource}. More advance encoding are put in place to parse information
// within the {FAVVMPlayerProfile::SkillIds}. The preprocessors available below are symbols defining the constraints of the bits encoding used.
#ifdef AVVMONLINE_USE_DEFAULT_SKILL_TREE_ENCODING
// @gdemers tree node (number of possible skills/perks/traits across all classes)
#define GET_SKILL_TREE_NODE_ID_ENCODING_BIT_RANGE (9)
#define GET_SKILL_TREE_NODE_ID_ENCODING_RSHIFT (0)
// @gdemers 
#define GET_SKILL_TREE_NODE_LEVEL_ENCODING_BIT_RANGE (3)
#define GET_SKILL_TREE_NODE_LEVEL_ENCODING_RSHIFT (10)
// @gdemers tree node position index (number of possible slots within a skill tree)
#define GET_SKILL_TREE_NODE_POSITION_ENCODING_BIT_RANGE (5)
#define GET_SKILL_TREE_NODE_POSITION_ENCODING_RSHIFT (14)
// @gdemers see AVVMOnlineInventory for our items hard limit for Id range. Keep in mind, we can handle shifting
// using our validation bits below!
#define GET_SKILL_TREE_NODE_OWNER_ID_ENCODING_BIT_RANGE (8)
#define GET_SKILL_TREE_NODE_OWNER_ID_ENCODING_RSHIFT (20)
// @gdemers our skill tree may be tied to an entity, and may only allow being active when that entity
// is active during gameplay.
#define CHECK_PLAYER_DEPENDENT_ENCODING (1 << 29)
#define CHECK_WEAPON_DEPENDENT_ENCODING (1 << 30)
#define CHECK_ATTACHMENT_DEPENDENT_ENCODING (1 << 31)
#endif

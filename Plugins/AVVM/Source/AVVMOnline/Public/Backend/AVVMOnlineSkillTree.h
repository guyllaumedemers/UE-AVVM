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
// @gdemers RELATIONSHIP Bitmask (7)
// Tree Node	000
// attachment	001
// character	010
// item			100
#define GET_SKILL_TREE_NODE_RELATIONSHIP_BIT_RANGE (2)
#define GET_SKILL_TREE_NODE_RELATIONSHIP_RSHIFT (0)
// @gdemers skill tree node global unique id (2047)
#define GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_BIT_RANGE (10)
#define GET_SKILL_TREE_NODE_VIRTUAL_GLOBAL_ID_RSHIFT (3)
// @gdemers skill tree node id to identify duplicated instances (7) (solve the problem of ownership, and attachment composition)
#define GET_SKILL_TREE_NODE_INSTANCED_ID_BIT_RANGE (2)
#define GET_SKILL_TREE_NODE_INSTANCED_ID_RSHIFT (14)
// @gdemers entry level (127)
#define GET_SKILL_TREE_NODE_LEVEL_BIT_RANGE (6)
#define GET_SKILL_TREE_NODE_LEVEL_RSHIFT (17)
// @gdemers entry position within a skill tree (255)
#define GET_SKILL_TREE_NODE_POSITION_BIT_RANGE (7)
#define GET_SKILL_TREE_NODE_POSITION_RSHIFT (24)
#endif

// @gdemers element lookup for supporting the socketing process -- identify dependencies between an attachment and an owner (which may have more than one instance)
#ifdef AVVMONLINE_USE_DEFAULT_SKILL_TREE_LOOKUP_ENCODING
// @gdemers physical id that represent the element we are evaluating dependencies for
#define GET_SKILL_TREE_NODE_LOOKUP_PHYSICAL_GLOBAL_ID_BIT_RANGE (10)
#define GET_SKILL_TREE_NODE_LOOKUP_PHYSICAL_GLOBAL_ID_RSHIFT (0)
// @gdemers the instance id that uniquely identify 'this' element
#define GET_SKILL_TREE_NODE_LOOKUP_INSTANCED_ID_BIT_RANGE (2)
#define GET_SKILL_TREE_NODE_LOOKUP_INSTANCED_ID_RSHIFT (11)
// @gdemers the virtual id of a dependency that reference our element we evaluate
#define GET_SKILL_TREE_NODE_LOOKUP_OWNER_VIRTUAL_GLOBAL_ID_BIT_RANGE (10)
#define GET_SKILL_TREE_NODE_LOOKUP_OWNER_VIRTUAL_GLOBAL_ID_RSHIFT (14)
// @gdemers the instance id of a dependency that reference our element we evaluate
#define GET_SKILL_TREE_NODE_LOOKUP_OWNER_INSTANCED_ID_BIT_RANGE (2)
#define GET_SKILL_TREE_NODE_LOOKUP_OWNER_INSTANCED_ID_RSHIFT (25)
#endif

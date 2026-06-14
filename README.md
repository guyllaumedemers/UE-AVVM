# UE-AVVM (Framework)

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7%20%2B-blue?logo=unrealengine&logoColor=white)](https://www.unrealengine.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-PC%20%20%7C%20Console-lightgrey.svg)]()
[![GitHub issues](https://img.shields.io/github/issues/guyllaumedemers/UE-AVVM?style=flat-square)](https://github.com/guyllaumedemers/UE-AVVM/issues)
[![GitHub stars](https://img.shields.io/github/stars/guyllaumedemers/UE-AVVM?style=flat-square)](https://github.com/guyllaumedemers/UE-AVVM/stargazers)

A lightweight, modular, and performance-driven framework for Unreal Engine designed to streamline [e.g., Gameplay Ability Systems, AI Behavior, State Management, etc.]. Built with a **C++ core** and exposed cleanly to **Blueprints** to bridge the gap between performance and rapid prototyping.

---

## 🚀 Key Features

* **Modular Architecture:** Easily plug and play core systems without tight coupling.
* **Performance First:** Core heavy-lifting handled in highly optimized C++, utilizing data-oriented design where applicable.
* **Developer Tooling:** Built-in debug imgui/draw tools and comprehensive logging categories for painless troubleshooting.
* **Blueprint Friendly:** Clean `@Gameplay` macros and strictly validated `BlueprintCallable` / `BlueprintNativeEvent` structures.

---

## 🛠️ Tech Stack & Requirements

| Requirement | Supported / Recommended Version                      |
| :--- |:-----------------------------------------------------|
| **Unreal Engine** | UE 5.7+ (Compatible with 5.X with minor adjustments) |
| **Language** | C++17 / C++20 & Blueprints                           |
| **Build Tool** | UnrealBuildTool (UBT) / Visual Studio 2022 or Rider  |
| **Target Platforms** | Windows         |

---

## 📦 Installation & Setup

### As a Project Plugin (Recommended)

1. **Clone the repository** directly into your project's `Plugins` directory:
   
```bash
   cd YourProject/Plugins
   git clone https://github.com/guyllaumedemers/UE-AVVM.git
```

---

## 📖 Architecture & Usage Examples

UE-AVVM is a lightweight, component-driven architectural framework for Unreal Engine that implements an Actor-View-ViewModel (AVVM) pattern. By leveraging the RAII (Resource Acquisition Is Initialization) principle and Unreal's GameFeaturePlugin system, UE-AVVM modularizes game state and UI presentation, entirely eliminating the architectural headaches caused by global, persistent ViewModels.

### 🚀 Key Features

1. RAII-Based Actor Componentization

* **Dynamic Extension**: Define a base Actor and inject runtime functionality dynamically using Unreal’s Game Feature Plugins.
* **The AVVMComponent**: This core component manages a collection of UObject-based Presenters that drive the architecture of both single-player and multiplayer games. 
* **Deterministic Lifecycle**: The lifetime of the AVVMComponent is strictly bound to its strongly-typed Outer (the owning Actor), ensuring clean memory management without dangling references.

2. Reference-Counted Presenters & Manual ViewModels

* Each AVVMPresenter is a reference-counted UObject responsible for managing its own Manual ViewModel instance.
* This design avoids the typical pitfalls of globally scoped ViewModels by scoping data presentation strictly to the lifespan of the relevant gameplay Actor.

3. Type-Erased Notification Subsystem (FInstancedStruct)

* Features a generic notification system driven by the AVVMNotificationSubsystem.
* Utilizes Unreal's FInstancedStruct to achieve clean type erasure, allowing you to pass abstract, lightweight data payloads seamlessly.
* Tag-Based Channels: Presenters can dynamically register to user-defined communication channels using Gameplay Tags.

4. Blueprint & Modular Friendly API

* Out-of-the-box API design that prioritizes Blueprint-exposed registration processes and delegate bindings.
* Provides a clean layer of abstraction around project modules, keeping your codebase decoupled and maintainable.

---

## 🧠 The Problem It Solves

In traditional Unreal Engine UI/Architecture development, relying on Global ViewModels often leads to brittle code and scaling issues:

* State Pollution: Developers build overly complex systems that store persistent information, requiring the cache to be manually updated multiple times across different object instances.
* Seamless Travel Headaches: Persistent global objects must be manually scrubbed, reset, and synchronized with the latest game state during seamless travel between maps.

### Our Solution

UE-AVVM eliminates manual state tracking and cache synchronization. By deferring all data handling directly to Unreal's native Actor Model, the framework automatically:

1. Instances a fresh set of localized Presenters alongside the Actor.
2. Initializes the internal data representation cleanly.
3. Displays the correct content immediately based on the new state.

No more manual resetting between maps, no more stale caches—just predictable, Actor-driven data flow.

---

## 🧩 Available Extensions & Plugins

Enhance your architectural pipeline with these official and community-supported add-on plugins for the UE-AVVM ecosystem:

| Plugin Name            | Description   | Key Feature    |
|:-----------------------|:--------------|:---------------|
| **AVVM**               | loremp ipsum. | lorem ipsum    |
| **AVVMImGui**          | loremp ipsum. | lorem ipsum    |
| **AVVMSlateIM**        | loremp ipsum. | lorem ipsum    |
| **AVVMSample**         | loremp ipsum. | lorem ipsum    |
| **BatchSample**        | loremp ipsum. | lorem ipsum    |
| **DamageSample**       | loremp ipsum. | lorem ipsum    |
| **FencingSample**      | loremp ipsum. | lorem ipsum    |
| **HitDetectionSample** | loremp ipsum. | lorem ipsum    |
| **InteractionSample**  | loremp ipsum. | lorem ipsum    |
| **InventorySample**    | loremp ipsum. | lorem ipsum    |
| **SkillSample**        | loremp ipsum. | lorem ipsum    |
| **SnapshotSample**     | loremp ipsum. | lorem ipsum    |
| **TeamSample**         | loremp ipsum. | lorem ipsum    |
| **TransactionSample**  | loremp ipsum. | lorem ipsum    |
| **WeaponSample**       | loremp ipsum. | lorem ipsum    |

---

# Plugin Description

AVVM (Actor-View-ViewModel) is a modular, high-performance architecture framework designed to streamline UI development, backend integration, and core gameplay systems in Unreal Engine. By decoupling data, visual representation, and game logic, AVVM provides a scalable foundation for both production-ready games and rapid prototyping.

The framework is divided into four highly specialized, interconnected modules:

### 🎮 Core Modules

* **Core UI Architecture (AVVM):** Establishes the foundation for user interface development by implementing the Actor-View-ViewModel paradigm. It exposes base classes for creating View Model instances and features a robust Presenter registration system. This system utilizes a centralized notification network to seamlessly forward generic payload data between your game logic and UI layers.
* **Gameplay Foundation (AVVMGameplay):** Drives core gameplay execution by providing robust base classes that support asynchronous resource loading via FDataRegistryId. Additionally, it features an extensible, tag-based activation system to handle seamless input and ability triggering.
* **Backend & Online Services (AVVMOnline):** Acts as the bridge between your game client and the web. This interface exposes standardized backend request wrappers and predefined data structures that extend the base payload classes used by the AVVM notification system. To ensure hassle-free networking, all user-defined types natively support serialization through Unreal JSON objects, allowing effortless conversion to and from strings when interacting with external REST APIs.
* **Developer Tooling (AVVMDebugger):** A dedicated diagnostic module built to accelerate workflows. It automatically registers descriptor contexts (such as cheat extensions) and utilizes ImGui immediate mode to execute real-time debug draw calls directly in-game.

### ⚔️ Gameplay & Ability Systems

* **AVVMAbilitySystemComponent:** An extended version of Unreal's native Ability System Component (ASC) engineered for modern data workflows. It natively handles the asynchronous loading, caching, and granting of Abilities and AttributeSets to prevent frame drops during runtime initialization.
* **AVVMAbilityInputComponent:** A specialized input routing component that captures user input to trigger abilities registered via gameplay tags, featuring full native support for explicit InputId mapping.

### ⚙️ Systems, Scheduling & Networking

* **AVVMResourceManagerComponent:** A standalone asset management component that asynchronously loads data from the DataRegistry and safely forwards the resulting data payloads back to the calling Outer object.
* **AVVMTickScheduler:** A high-performance optimization system that aggregates Actor ticks on a per-class basis using a Multi-Level Feedback Queue (MLFQ) architecture, utilizing a Round Robin approach to distribute jobs efficiently across frames.
* **AVVMNetSynchronization:** A specialized networking utility that monitors and registers individual feature states across the network, ensuring precise visual synchronization of all relevant Actors (both local and simulated) on the local client's UI.
* **AVVMWorldSetting & AVVMWorldRule:** An extension of Unreal’s AWorldSettings that introduces per-world modular configuration. Developers can define custom AVVMWorldRule assets at the project or plugin level and register them with the system to dynamically configure gameplay logic, toggle features, or dictate subsystem creation on a per-map basis.

### 🛠️ Developer & Editor Tooling

* **Featured Tool:** AVVMDataTableEditor – A custom helper utility built to enforce data integrity during Data Table creation, ensuring entries strictly adhere to constraints defined across the AVVM plugin API and Samples. For example, in the provided UInventorySample, the tool validates that an entry's ActorIdentifier falls within a strictly bounded, user-defined range determined by its referenced item Category type.

---

# GameFeature Plugin Descriptions

## World Interaction System

This GameFeature Plugin (GFP) serves as an extensible blueprint and production-ready sample for handling network-replicated interactions between local players and world actors. Built with multiplayer architecture in mind, the system utilizes a robust handshake protocol to manage actor ownership and eliminate contingency or race conditions when multiple players attempt to interact with the same object simultaneously.

## 🚀 Key Capabilities

* Handshake Ownership Protocol: Prevents interaction concurrency issues in high-latency multiplayer environments.
* Modular Logic Splitting: Decouples validation requirements from the actual execution payload.
* Native Gameplay Effect Integration: Leverages Unreal's Gameplay Ability System (GAS) to safely pass interaction context via active effects.

## 🛠️ Implementation Guide (How-To)

To integrate a world interaction, follow this modular setup pipeline:

### 1. The Interactable Actor Setup

Using the GameFeature data asset configuration (GFP_AddComponent), inject an UActorInteractionComponent onto your target interactable Actor.

* This component manages interaction lifecycle data via a Record Object, which serves as the server-authoritative handshake between the player and the actor to resolve any multiplayer ownership conflicts.
* Define an Interaction Implementation Object inside this component. This object dictates how record locking behaves and manages the granting of proximity-based GameplayEffects.

### 2. Proximity & Context Passing

When a player overlaps with the interactable actor, a GameplayEffect is granted to the player character. Crucially, the interactable actor assigns itself as the EffectCauser within the effect context, allowing data to cleanly bridge into the player's ability system.

### 3. Validation & Execution Logic

Configure your interaction behavior by deriving from two structural types within your Implementation Object:

* FInteractionExecutionRequirements: Create a derived implementation to evaluate and validate whether the interaction criteria are successfully met.
* FInteractionExecutionContext: Create a derived implementation to define the exact logic/action that fires immediately upon successful interaction validation.

### 4. Player Ability Binding

Register a custom ability derived from UPlayerInteractionAbilityBase to your player character.

* Tag Matching: Ensure this ability is bound to a Gameplay Tag identical to the tag applied to the player by the actor's overlap GameplayEffect.
* Actor Retrieval: Upon activation, the ability automatically unrolls the GameplayEffect payload, pulling the interactable actor out of the EffectCauser slot.
* The Handshake: During the execution of the player's interaction ability, the target world actor is safely queried to accept or reject the handshake before running the execution context.

💡 Included Example: See UPlayerHoldInteractionAbility for a complete reference implementation. This sample demonstrates a player executing a "hold input" interaction on a physical world Shop Actor to securely open an empty UI storefront menu.

---

## Inventory & Content Gathering System

This GameFeature Plugin (GFP) provides a data-driven, network-replicated sample system for content gathering, asset socketing, and inventory management. Engineered for versatility, this framework allows players, AI enemies, and inanimate world objects (like chests or item drops) to dynamically exchange, acquire, and release items under authoritative validation.

By utilizing modular configurations (GFP_AddComponent), target actors are automatically provisioned with asynchronous resource loading, automated actor socketing, and relevant AttributeSet registration out of the box.

## 🌐 Network Optimization Note

Depending on your project scale, standard object replication for massive inventories can become a bottleneck. It is highly recommended to implement a FFastArraySerializer proxy layer to mirror the internal state of your replicated UItemObject data. Ensure all data forwarding and state updates hook directly into the existing API to maintain architectural consistency.

## 🛠️ Implementation Guide (How-To)

The system operates on an optimized bit-encoded data scheme and decouples runtime state from visual representations:

### 1. Data Schema & Runtime Representation

Items are split into a lightweight runtime data object and an optional physical actor:

* UItemObject: Create a Blueprint deriving from this class. This is a stateful, fully replicated UObject owned by the inventory component's Outer. It acts as the definitive data authority and manages the lifecycle/spawning of its physical 3D Actor counterpart in the world.

### 2. Provider Interfaces

Your inventory's owning Actor (the Outer) must implement the IInventoryProvider interface. This abstraction layer bridges your game logic to your chosen data source, allowing project-specific queries to seamlessly fetch item definitions from either local DataAssets or an external database.

### 3. Bit-Encoded Item Structuring

To maximize network efficiency and simplify data queries, items are encoded using the {FAVVMPlayerProfile::InventoryIds} bitmask layout. A single integer encapsulates all vital item metadata, including:

* Unique Item ID / Attachment ID
* Target Storage Container & Grid Position
* Stack Count
* Character Soulbound Status

This packed integer enables lightning-fast item filtering, sorting, and container verification without heavy struct unpacking.

### 4. Initialization Pipeline

Inventories can be initialized dynamically via backend payloads or local data structures:

* The component receives a collection of FDataRegistryId handles.
* The system evaluates these handles against the current component state (e.g., stripping out blacklisted items if the player is currently in a pre-game lobby).
* Assets are loaded asynchronously. The initial layout layout is written into the bit-encoded schema.
* Any runtime updates (moving items, splitting stacks) are performed on a local copy and serialized back to the backend or disk when saved.

### 5. Automated Equipment Socketing

During the initialization phase, items automatically read their bitmask data to determine their assigned equipment slots and socket themselves to the parent mesh. ⚠️ Current WIP Constraint: The system currently prevents multiple items sharing the exact same Unique ID from resolving complex sub-attachment nesting chains (e.g., a weapon mod attached to an attachment that is attached to a gun). This will be addressed in a future update.

### 🔗 Online Integration Standards

For external web architectures, your project is expected to comply with the native AVVMOnlinePlayer and AVVMOnlineInventory API standards.
If you are running a purely local or standalone project, simply invoke the core API methods to request your inventory Outer resources directly. UI layouts expect their corresponding ViewModels to receive a UItemObject reference for 2D visual layout representation, while 3D viewport rendering is automatically managed by the underlying Actor reference.

---

## Fencing & Deferred Event System

This GameFeature Plugin (GFP) provides a robust, production-ready framework for managing deferred event execution based on custom, server-authoritative state requirements. Designed to handle complex synchronization scenarios in both single-player and multiplayer environments, the Fencing System ensures that critical gameplay transitions wait smoothly until all pre-requisites are met before triggering the next sequence.

## 🚀 Key Capabilities

* State Synchronization: Perfect for gating execution blocks behind initialization phases, loading screens, network handshakes, or cutscene completions.
* Network-Replicated Fences: Leverages lightweight, replicated tags via the UAVVMReplicatedTagComponent to instantaneously notify local and simulated clients of state changes.
* Race Condition Mitigation: Guarantees that multi-client games stay perfectly in sync during critical world transitions.

## 🛠️ Implementation & Architecture Guidance

The Fencing System acts as a gatekeeper across your subsystems to orchestrate precise execution timing:

### 1. Replicated State Tracking

Instead of relying on heavy RPC polling or fragile timing delays, the system utilizes the UAVVMReplicatedTagComponent. State milestones (e.g., State.Fence.LoadingComplete, State.Fence.CutsceneFinished) are pushed to this component as replicated gameplay tags, which instantly propagate to all connected clients.

### 2. Setting Up a Fence

When a system needs to defer logic, it registers a "Fence" with the framework, specifying the mandatory requirements (tags) that must be present before continuing.

* The system halts downstream execution, allowing background tasks—such as asset streaming, UI asset caching, or backend player profile retrieval—to complete safely without blocking the main game thread or causing script errors.

### 3. Verification & Execution

Once the server updates the UAVVMReplicatedTagComponent and all required tags are present on the client, the fence is considered "dropped." The system immediately executes the deferred actions across all synchronized endpoints.

💡 Best Practice: Integrate the fencing system wherever multi-system synchronization is critical. Ensure that high-impact features like custom loading screen teardowns, cinematics, level streaming handshakes, and multiplayer round countdowns are "fenced" to guarantee a seamless, stutter-free user experience!

---

## Batch Action & Content Processing System

This GameFeature Plugin (GFP) provides a scalable framework for executing batch actions on registered game content. Designed to mitigate performance spikes during heavy collection management, the system allows developers to aggregate actors or data objects and process them simultaneously based on custom, user-defined execution conditions.

## 🚀 Key Capabilities

* Performance Optimization: Prevents frame-rate hitches by grouping heavy, recurring logic into controlled execution windows.
* Conditional Processing: Allows batch jobs to be deferred, filtered, or throttled based on specific runtime criteria (e.g., performance thresholds, gameplay states, or distance metrics).
* Extensible Architecture: Designed to serve as a generalized template for scaling any multi-object operations across the framework.

## 🛠️ Implementation & Architecture Guidance

The Batch System acts as a central registry to decouple what content needs an action from when and how that action is executed:

### 1. Registration Phase

Target objects or actors register themselves with the central batch management subsystem. This places them into an un-tracked, lightweight pool, removing the need for individual actors to independently monitor or poll for state updates.

### 2. Conditional Execution

Instead of executing operations immediately upon request, the manager evaluates user-defined conditions. For example, a bulk Destroy command on a group of discarded world loot drops can be queued and executed over multiple frames, or deferred until the player looks away, ensuring a seamless, stutter-free gameplay experience.

---

## GameState Transaction History & Analytics

This GameFeature Plugin (GFP) provides a lightweight, network-replicated caching system for capturing telemetry and statistics during gameplay events. By parsing transaction payloads into Unreal JSON objects and storing them as serialized strings, the system delivers a flexible, schema-agnostic data pipeline. It natively supports full client replication, allowing players to view real-time synchronized updates across global UI features like leaderboards, match recaps, and after-action reports.

## 🚀 Key Capabilities

* Flexible JSON Payloads: Leverages Unreal's JSON object architecture to store varying data structures into string fields without breaking runtime compatibility.
* Global Synchronization: Replicates critical transaction logs across all connected clients for instant UI previewing.
* Dual-Purpose Design: Operates as both an in-game data aggregator and a lightweight telemetry system.

## ⚠️ Performance & Usage Considerations

* Frequency Constraints: This system is architected for global, high-level data gathering. It is inefficient to parse these string-serialized payloads for high-frequency, frame-by-frame visual updates.
* Telemetry & Crash Reporting: This framework is an excellent candidate for local telemetry buffering; data can be cached seamlessly throughout the match and published to external endpoints only at the end of gameplay or immediately prior to writing a crash dump.

## 🛠️ Implementation Guide (How-To)

To begin logging and tracking match-wide statistics, follow this implementation pipeline:

### 1. GameState Initialization

Using the GameFeature data asset configuration (GFP_AddComponent), inject the UGameStateTransactionHistory component directly onto your project's AGameState actor.

### 2. Dependency Management

Add this plugin as a dependency inside the modules where telemetry or stats tracking is required. Because this system acts as a foundational data logger, this plugin should always remain loaded during the match lifecycle.

### 3. Recording & Querying Transactions

* Writing Data: Invoke the recording API whenever a significant gameplay event occurs (e.g., player elimination, objective capture, match milestone reached) to push a new transaction into the history log.
* Reading Data: For UI and display systems, utilize the built-in C++ Template methods to safely parse, filter, and return aggregated values from the string-serialized history pool.

---

## Weapon & Equippable Item Framework

This GameFeature Plugin (GFP) provides a foundation for handling equippable, triggering actors (such as weapons, armor, or tools) that grant and activate abilities on the player character. Built directly on top of the Inventory Sample plugin, this framework bridges item inventory states with real-time gameplay execution, enabling modular item initialization powered by automated AttributeSet registration.

## 🎯 Architectural Philosophy: Data-Oriented vs. Class-Oriented

Weapon and equipment systems are notorious for bloating base codebases when developers create separate C++ or Blueprint classes for every minor iteration or item tier.

To prevent this architectural debt, this plugin enforces a Data-Oriented Design:

* Use the existing, generic base classes provided by the framework (such as the base Attachment actor).
* Iterate strictly via data configuration in derived Blueprints or Data Assets.
* Control unique item identities purely through data variables: What ability tags does this weapon grant? Which AttributeSet does it initialize? What meshes does it swap out?

## 🛠️ Implementation & Current Status

Because this framework is actively under development, developers are highly encouraged to thoroughly review the underlying Inventory Sample data schema to fully grasp the bit-encoded constraints governing item ownership and attachment structures before extending the system.

### 1. The Triggering Lifecycle

When an item is equipped out of the inventory system, the framework identifies it as a "Triggering" item. The system automatically handles the physical spawning and socketing of the attachment while linking the item's data structure to the player's core Ability System.

### 2. Attribute Set Integration

Upon equipping a weapon or piece of armor, the framework dynamically references and binds the designated AttributeSet (e.g., granting temporary MaxAmmo, WeaponSpread, or BaseDamage attributes unique to that weapon) without requiring permanent attribute bloat on the character's base Actor.

### 3. Ability Mapping & Execution

Once initialized, the equippable actor exposes its gameplay capability mappings to the player. The player's input or tag activation systems can then directly invoke abilities bound specifically to the currently held item context.

---

## Team & Matchmaking Separation System

This GameFeature Plugin (GFP) provides a robust, production-ready framework for managing team alignment, player assignment, and gameplay separation. Engineered to integrate seamlessly with external server architectures, this system exposes a unified API that translates backend profile and matchmaking data into server-authoritative team structures at runtime.

## 🚀 Key Capabilities

* Backend-Driven Team Generation: Automatically maps incoming web/backend metadata to internal gameplay team configurations upon player initialization.
* Modular Separation Logic: Decouples team identity from rigid character classes, allowing smooth runtime team switches, spectator setups, or multi-team game modes.
* Framework-Wide Compatibility: Integrates cleanly with UI ViewModels for scoreboard representation and gameplay systems for friendly-fire or targeting validation.

## 🛠️ Implementation & Architecture Guidance

The Team System acts as an organizational layer that dictates how players and world entities interact based on their assigned allegiance:

### 1. Data Retrieval & Mapping

During the initialization phase (or upon match join), the framework queries the game's network/backend adapter. The system reads the incoming player profile payload and uses the Team API to assign a unique, server-validated team identifier to the player's session.

### 2. Gameplay & Subsystem Isolation

Once team assignments are locked, other core systems can query the Team API to conditionally execute logic. This provides immediate, out-of-the-box support for:

* Targeting & Threat Evaluation: Ensuring AI behaviors or auto-target abilities differentiate between allies and hostiles.
* UI & ViewModel Updates: Filtering scoreboard data, nameplates, and mini-map icons dynamically based on the local player's team perspective.
* Subsystem Routing: Directing team-specific chat channels, voice streams, or specialized spawn rules efficiently.

---

## Skill & Talent Progression System

This GameFeature Plugin (GFP) provides a data-driven, network-replicated framework for managing character skills, talent trees, and active capability progression. Built directly around the highly optimized bitmask architecture established in the Inventory Sample, this plugin treats skills, passive nodes, and masteries as lightweight data payloads. This ensures instant filtering, rapid server validation, and absolute memory efficiency in multiplayer environments.

## 🚀 Key Capabilities

* Unified Architectural footprint: Shares the exact same underlying identification and encoding systems as your inventory, reducing boilerplate and centralizing data formatting rules.
* Network Efficiency: Compresses complex tree progression, node states, and skill tiers into highly optimized bitfields for minimal network replication overhead.
* GAS & Attribute Bonding: Dynamically maps unlocked skill states to active GameplayAbilities and modifies character AttributeSets on the fly.

## 🛠️ Implementation & Architecture Guidance

The Skill System mirrors the workflow of your inventory framework, treating a player's skill book or talent tree as a specialized data container:

### 1. Bit-Encoded Skill Layout

To eliminate massive, nested structural replication over the wire, skills are encoded using the framework's native {FAVVMPlayerProfile} integer data scheme. A single packed integer encapsulates all vital skill metadata, including:

* Unique Skill / Node ID (Mirroring the Item/Attachment Unique ID format).
* Tree Column & Row Position (Mirroring the Container & Grid Position format).
* Current Skill Rank / Tier Count (Mirroring the Stack Count format).
* Unlock / Mutation Status (Mirroring the Soulbound / Custom Flag format).

### 2. Initialization & Validation Pipeline

When a player loads into the world, their skill profile is loaded asynchronously via FDataRegistryId handles. The system parses the bitmask configuration to instantly map out the player’s active character build.
Because the structure matches the inventory API, you can easily implement validation logic using your existing editor toolkit rules (e.g., ensuring a skill's allocated points do not exceed the max tier range assigned to its specific tree category).

### 3. Dynamic Ability Granting

Upon successful runtime verification of a skill bitmask change (such as a player allocating a point into a new talent node), the system instantly triggers a copy update. It then automatically communicates with the player's AVVMAbilitySystemComponent to asynchronously load, cache, and grant the newly unlocked ability or attribute modifiers.

---

## 🤖 Automation Testing (Functional Test / Gauntlet)

> **Current Status Note:** Automated testing is actively under development across all core modules and GameFeature Plugins (GFPs) to ensure the API remains bulletproof against regression. The following matrix outlines the current validation coverage.

| System / Module | Test Type | Coverage Status | Notes / Methodology |
| --- | --- | --- | --- |
| **AVVM Subsystem** | Functional | 🟢 **Passing** | Validates core subsystem life-cycle, initialization, and shutdown sequences. |
| **AVVM Notification Subsystem** | Functional | 🟢 **Passing** | Tests payload routing reliability and Presenter registration callbacks. |
| **AVVM Resource Manager Component** | Functional | 🟢 **Passing** | Utilizes **Latent Commands** to thoroughly validate asynchronous asset streaming from the `DataRegistry`. |
| **BatchSample** | Functional | 🟢 **Passing** | Verifies conditional aggregation and bulk optimization paths (e.g., mass `AActor::Destroy`). |
| **TransactionSample** | Functional | 🟢 **Passing** | Validates JSON parsing stability, string serialization, and historical data logging. |
| **InventorySample** | Unit / Functional | 🟢 **Passing** | Unit Tests are fully operational. **WIP:** Server-client networking and multiplayer handshake testing under Gauntlet are currently being implemented. |
| **SkillSample** | Unit / Functional | 🟢 **Passing** | Unit Tests are fully operational. **WIP:** Multi-client synchronization tests for bit-encoded node unlocks are currently being implemented. |

---

### 🛠️ Running the Tests

#### 1. Via the Unreal Editor (Unreal Automation Tool)

To execute the suite locally within the editor:

1. Open the **Test Automation** window (`Window > Developer Tools > Session Frontend`).
2. Navigate to the **Automation** tab.
3. Filter by the `AVVM` category prefix.
4. Select your desired tests and click **Start Tests**.

#### 2. Via Command Line (CI/CD & Gauntlet)

For headless verification or build-machine integration, execute the Unreal Automation Tool via the CLI:

```bash
Engine\Build\BatchFiles\RunUAT.bat BuildCookRun ^
  -project="YourProjectName" ^
  -runautomationtests ^
  -automationtestfilter="AVVM" ^
  -unattended ^
  -noleaktests

```

---

### 📄 License

#### Distributed under the MIT License. See LICENSE for more information.

## ✉️ Contact & Acknowledgments

If you want to discuss architectural design patterns in Unreal, have questions about this implementation, or wish to reach out regarding professional opportunities, feel free to connect:

* **Developer:** Guyllaume Demers
* **GitHub:** [click me](https://github.com/guyllaumedemers)
* **LinkedIn:** [click me](https://www.linkedin.com/in/guyllaume-demers-4960033b1/)
* **Project Link:** [click me](https://github.com/guyllaumedemers/UE-AVVM)

### Stay tuned!

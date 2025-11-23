# UE-AVVM (Framework)

UE-AVVM. Actor, View, ViewModel. Based on the **RAII** principle, define an **Actor** on which we add runtime components using Unreal **GameFeaturePlugin**. Our runtime component, **AVVMComponent**, expose a collection of **Presenter** UObject that define the architecture of a single/multiplayer game. It's lifecycle depends on it's owning typed **Outer**! Each **Presenter** UObject, **AVVMPresenter** is responsible for it's own **Manual** ViewModel instance and is **ref-count**.

Additionnally, this system is paired with a generic notification system, **AVVMNotificationSubsystem**, using **FInstancedStruct**, which favor type erasure and abstract payload information. Each **Presenter** can register to user defined **channels** using **GameplayTags**. Our api tend to favor exposing the registration process/delegate binding through Blueprints, and provide abstraction around project modules.

Our objective with this system is merely to reduce the challenges that come during development with the usage of **Global** ViewModel type, where often, programmers will build complex system, store information and update it's cache multiple time by interacting with multiple instance of an object. Or create one persistant object, which between seamless travel, has to be updated according to the latest state of the game. Using our system, we remove those operation and defer any data handling to Unreal **Actor Model** to instance a new set of presenters, initialize it's internal representation and display proper content.

Note : This system is currently a work in-progress and has for objective to remove problem caused by data persistency coming from **Global** ViewModel type misuse. A core benefit of using our system can be easilly experience during **Pawn** controller update. Runtime change of **Actor** can often be a burden when handled from a **global** standpoint but become trivial when acted on at the **Actor** level.

Here's an example case explaning our expectations : 

* Steve's controlling a Pawn **Actor**. His Pawn has health, Stamina and a Backpack interaction which allow them to open their inventory.
* Steve sees a car.
* Steve takes control of the car.
* By taking control of the car, a notification was invoked during the **OnPossesChanged**, and now our HUD elements have all updated with what a user would normally expect from a driving experience. We no longer have a backpack showing, our stamina has now become our gas tank, etc... 

### Project Miro

Mind map for the UE-AVVM system and general use case built using **Overwatch 2** for reference. [Note : This mind map is built based on experience gained during production of COOP multiplayer for a AAA game.](https://miro.com/app/board/uXjVI663y_k=/?share_link_id=72860067020)

#### Cheats

To iterate quicker on the development process and validate system features, I added cheats through **GameFeatureActions**. See **UAVVMCheatExtension** for more information! Additionally, to test the various notification channels, make sure to properly inject channel tags that follow the plugin define nomenclature. i.e : **ModuleName.ChannelName.PresenterClass.GameplayEventType**.

Note : Cheats are really how this system will be tested and build upon. Resources to implement proper **Backend** behaviour are not available, and implementing one is out-of-scope for this project. To test any **Backend** request, simply have your **Actor** type implement the **AVVMOnlineInterface** and extend on the exposed virtual calls.

##### Tip

Check **AVVMSample** plugin, under **GameFeaturePlugin**, for details about the project architecture! Also, a quick look at **Modules** default scripts might save you some headache!

##### Warnings

ImGui plugin was added as a submodule, so it likely that any deprecation fixes made locally aren't commited to the following project. Those changes are minimal enough to keep it as such and not have to fork the existing plugin to address them on the local copy.

### UE-Miro

[GameFeature](https://miro.com/app/board/uXjVI9C3ofk=/?share_link_id=470254566267) - UGameFeatureData are name dependent. UGameFeaturePluginStateMachine cannot register/load/activate unless it's named identical to the GameFeature plugin it represent.

[MVVM](https://miro.com/app/board/uXjVI8PJltw=/?share_link_id=952318299614)

[DataRegistry](https://miro.com/app/board/uXjVI8q9jKI=/)

[CommonGame](https://miro.com/app/board/uXjVI8F91lE=/?share_link_id=765292763899)

[ImGui](https://miro.com/app/board/uXjVI5N81qQ=/?share_link_id=426653642808)

[Actor Replication](https://miro.com/app/board/uXjVIxoRe8E=/?share_link_id=43947381721)

#### Preview

![UE-AVVM](https://github.com/guyllaumedemers/UE-AVVM/blob/master/Content/gitRes/UE-AVVM.jpg)

# AVVM

### A quick walkthrough to help the reader be made aware of the existing utilities that can be found under the AVVM plugin.

 * AVVMDebugger : Is a developer tool module that register descriptor context (i.e cheat extension) and execute draw calls using imgui immediate mode.
 * AVVM : Is a core system for UI development. It exposes the base class required to create View Model instance (with the Actor-View-ViewModel paradigm) and register Presenter object with the notification system from which we can forward generic payload information.
 * AVVMOnline : Is an interface exposing general backend request and pre-define data struct to be used with your game. Available payloads extend from the expected base class use by the AVVM notification system. User defined types exposed here all support serialization through Unreal json object and can be converted back/from string, facilitating interfacing with your backend api.
 * AVVMGameplay : Is a core system for gameplay development. It exposes the base class for supporting **Async Resource loading** using FDataRegistryId and **Input/Ability triggering** via tag activation.
 * ...

**Other modules are placeholder work for ideas that are currently floating around.**

## GameFeature Plugin Sample Modules

To properly vehicule the train of thought into laying down good grounds for general game production, scenario example cases were added and can be found under **AVVMGameplaySampleRuntime**, **AVVMSampleRuntime**, **InteractionSample**, **FencingSample**, **InventorySample**, etc... Note that the general architecture of this project follows Unreal Actor Model and any relevant **feature** will most-likely be derived/or owned by Unreal's three main Actor classes. i.e AGameMode, AGameState & APlayerState. General implementation details can be found in the interface those classes implement from their derived **AVVM** type, under AVVMGameplay module. It may be important to note that the general communication method favored here, and to keep modules serarate for your project, rely heavily on Interface dispatch!

### Interaction Sample

This GameFeature plugin is a sample plugin for supporting general interaction between a Local player and a World Actor. The overall system support replication and grant players with the ability to interact with world actor while optionaly preventing contingency during multiplayer scenario.

### Transaction Sample

This GameFeature plugin is a sample plugin for caching statistics captured during gameplay events. Transaction payload are parsed using Unreal json object and store statistics via a string field. The overall system support replication of transactions to all clients who can preview the latest updates from systems such as a leaderboard, after action report and more.

### Inventory Sample

This GameFeature plugin is a sample plugin for supporting content gathering. Players, enemies and inanimate objects can all use this system to exchange, acquire and/or release content from under their authority. Implementation details are still in the earlier stage of development and have to be further polished. Abilities for consuming, equipping and/or dropping items are yet to be put in place. **Edit** : This system has recently been cleanup to allow backend information to populate player representation, and support initialization of ItemActors via GAS AttributeSet.

### Fencing Sample

This GameFeature plugin is a sample plugin for supporting deferred execution of events based on user requirements such as, waiting for initialization phase to be complete, ending of a cutscene or even synchronization between clients. The overall system leverage replicated tags from **UAVVMReplicatedTagComponent** to notify clients of a state change. Use the fencing system wherever possible and ensure systems like loading screen, cutscene and more... are ready to execute their next action!

### Batch Sample

This GameFeature plugin is a sample plugin for supporting batch actions on content that are subject for registration with the managing system. Currently only supporting batching call to AActor::Destroy, the purpose this system is to allow generalizing
batch actions, and execute process based on user-defined conditions.

### Weapon Sample

This GameFeature plugin is a sample plugin that define reusable construct for **Triggering** abilities tied to an Actor our player may reference during gameplay. This system, build a-top the InventorySample plugin, allow creation of **Triggering** items, and their initialization based on AttributeSet reference. Implementation details are still under development. Notes : It's suggested that the data scheme be first reviewed to better understand the constraint within which this system exist.

### Snapshot Sample

tbd

### Team Sample

This GameFeature plugin is a sample plugin for supporting team gameplay separation, and expose the necessary api to building teams based on backend information retrieved.

### Damage Sample

tbd

**Notes : All plugins defined above are example cases that utilize the api from AVVM plugin.**

# Automation Testing (Functional Test/ Gauntlet)

**Notes** : Automated Testing is in the work for existing GFP modules, and will keep being worked on so to ensure the api provided is put to the test. In the meantime, here's a list of the existing api that is validated!

* AVVM
* BatchSample
* TransactionSample

### Stay tuned!

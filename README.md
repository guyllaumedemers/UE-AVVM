# UE-AVVM

UE-AVVM. Actor, View, ViewModel. Based on the **RAII** principle, define an **Actor** on which we add runtime components using Unreal **GameFeaturePlugin**. Our runtime component, **AVVMComponent**, expose a collection of **Presenter** UObject that define the architecture of a single/multiplayer game. It's lifecycle depends on it's owning typed **Outer**! Each **Presenter** UObject, **AVVMPresenter** is responsible for it's own **Manual** ViewModel instance and is **ref-count**.

Additionnally, this system is paired with a generic notification system, **AVVMNotificationSubsystem**, using **FInstancedStruct**, which favor type erasure and abstract payload information. Each **Presenter** can register to user defined **channels** using **GameplayTags**. Our api tend to favor exposing the registration process/delegate binding through Blueprints and favor reusability.

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

To iterate quicker on the development process and prove fatures from the system, I added cheats through **GameFeatureActions**. See **UAVVMCheatExtension** for more information! Additionally, to test the various notification channels, make sure to properly inject channel tags that follow the plugin define nomenclature. i.e : **ModuleName.ChannelName.PresenterClass.GameplayEventType**.

Note : Cheats are really how this system will be tested and build upon. Resources to implement proper **Backend** behaviour are not available, and implementing one is out-of-scope for this project. To test any **Backend** request, simply have your **Actor** type implement the **AVVMOnlineInterface** and extend on the exposed virtual calls.

##### Tip

Check **AVVMSample** plugin, under **GameFeaturePlugin**, for details about the project architecture! Also, a quick look at **Modules** default scripts might save your some headache!

### UE-Miro

[GameFeature](https://miro.com/app/board/uXjVI9C3ofk=/?share_link_id=470254566267) - UGameFeatureData are name dependent. UGameFeaturePluginStateMachine cannot register/load/activate unless it's named identical to the GameFeature plugin it represent.

[MVVM](https://miro.com/app/board/uXjVI8PJltw=/?share_link_id=952318299614)

[DataRegistry](https://miro.com/app/board/uXjVI8q9jKI=/)

[CommonGame](https://miro.com/app/board/uXjVI8F91lE=/?share_link_id=765292763899)

#### Preview

![UE-AVVM](https://github.com/guyllaumedemers/UE-AVVM/blob/master/Content/gitRes/UE-AVVM.jpg)


# UE-AVVM

UE-AVVM. Actor, View, ViewModel system. Based on the RAII principle, this architecture benefit from Ref counting "Manual" ViewModel instance, using it's TypedOuter<Actor>, and is managed via a World Subsystem.
It follows the core architecture of Unreal and ensure consistent data between Travels (as it's being rebuild with the owning Actor).

This system is a Work in-progress with the end goal of removing some of the drawback affecting data persistency that comes with "Global" ViewModel. It's core benefit being that active ViewModel "presenting" can easily
be swapped at Runtime based on the Actor controlled/or interacted with.

### Project Miro

Mind map for the UE-AVVM system and general use case built using "Overwatch 2" for reference. [Note : This mind map is built based on experience gained during production of COOP multiplayer for a AAA game.](https://miro.com/welcomeonboard/bmZmV0ZtQko0Q1U1Y1BjcUdlOEVubkNaWlY5Ulo1Zm44dURNbTI2YlJ2ZFhSNkRZVFNSMVROVzQ3MXpKZ0V0NlMyb1VVY3d1bENxeTFhS2VZcG44L3BrK1c4REtRNkJ4MGhJYjlidlhDdVZKMVZTcVhYck1zcHFVd0t5aGxQK0JyVmtkMG5hNDA3dVlncnBvRVB2ZXBnPT0hdjE=?share_link_id=554815863683)

### UE-Miro

[GameFeaturePlugin](https://miro.com/app/board/uXjVI9C3ofk=/?share_link_id=470254566267)

[MVVM]()

#### Preview

![Mindmap](https://github.com/guyllaumedemers/UE-AVVM/blob/master/Content/gitRes/MindmapPreview.jpg)


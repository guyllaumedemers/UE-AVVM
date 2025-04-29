# UE-AVVM

UE-AVVM. Actor, View, ViewModel. Based on the **RAII** principle, this architecture benefit from **Ref-counting** "Manual" ViewModel instance, using it's TypedOuter<Actor>, and is managed via a World Subsystem.

It follows the core architecture of **Unreal Engine** and ensure consistent data between Travels (as it's being rebuilt with the owning Actor).

This system is a work in-progress and has for end goal to remove some of the drawback affecting data persistency which comes with "Global" ViewModel usage. It's core benefit being that active ViewModel "presenting" can easily
be swapped at Runtime based on the Actor it's owned by and "present" new data without running complex control flow for project specific use-case.

### Project Miro

Mind map for the UE-AVVM system and general use case built using **Overwatch 2** for reference. [Note : This mind map is built based on experience gained during production of COOP multiplayer for a AAA game.](https://miro.com/welcomeonboard/bmZmV0ZtQko0Q1U1Y1BjcUdlOEVubkNaWlY5Ulo1Zm44dURNbTI2YlJ2ZFhSNkRZVFNSMVROVzQ3MXpKZ0V0NlMyb1VVY3d1bENxeTFhS2VZcG44L3BrK1c4REtRNkJ4MGhJYjlidlhDdVZKMVZTcVhYck1zcHFVd0t5aGxQK0JyVmtkMG5hNDA3dVlncnBvRVB2ZXBnPT0hdjE=?share_link_id=554815863683)

### UE-Miro

[GameFeature](https://miro.com/app/board/uXjVI9C3ofk=/?share_link_id=470254566267)

[MVVM](https://miro.com/app/board/uXjVI9IeVNQ=/?share_link_id=266249431038)

[DataRegistry](https://miro.com/app/board/uXjVI8q9jKI=/)

[CommonGame](https://miro.com/app/board/uXjVI8F91lE=/?share_link_id=765292763899)

[CommonUser](https://miro.com/app/board/uXjVI8F9wkE=/?share_link_id=331050441984)

#### Preview

![Mindmap](https://github.com/guyllaumedemers/UE-AVVM/blob/master/Content/gitRes/MindmapPreview.jpg)


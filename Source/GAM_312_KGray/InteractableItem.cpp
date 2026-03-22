#include "InteractableItem.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AInteractableItem::AInteractableItem()
{
	// [Week 2] Resource nodes are static objects in my game, so they don't need to update every frame. Performance win!
	PrimaryActorTick.bCanEverTick = false;

	// [Week 2] Creating the visual mesh component and making it the root.
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	// [Week 2] Setting some default fallback values so the object has basic functionality immediately upon spawning.
	ResourceType = EResourceType::Wood;
	ResourceCapacity = 3;
}

// [Week 2] Executes when the player successfully interacts with this object via their camera line trace.
bool AInteractableItem::GatherResource(EResourceType& OutResourceType)
{
	// Check if the node still has resources left to give.
	if (ResourceCapacity > 0)
	{
		// Pass the specific resource type back to the player's inventory logic.
		OutResourceType = ResourceType;

		// Deplete the node's capacity by 1.
		ResourceCapacity--;

		// If the node is completely empty after this swing, we destroy it to remove it from the game world.
		if (ResourceCapacity <= 0)
		{
			Destroy();
		}

		return true; // Let the player know the gathering action was successful.
	}

	return false; // The node was empty, gathering failed.
}
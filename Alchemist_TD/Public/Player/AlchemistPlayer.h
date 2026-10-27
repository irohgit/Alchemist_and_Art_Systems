#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AlchemistPlayer.generated.h"

class AOmelette;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerPickedUpOmelette);

UCLASS()
class ALCHEMIST_0_API AAlchemistPlayer : public APawn
{
	GENERATED_BODY()

public:

	AAlchemistPlayer();
	//DELEGATES
	UPROPERTY(BlueprintAssignable, Category = "Alchemist|Player") FOnPlayerPickedUpOmelette OnPlayerPickedUpOmelette;

	// -------------------------------------------------------------------------
	// OMELETTE CARRY
	// -------------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Player") TObjectPtr<AOmelette> CarriedOmelette = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Alchemist|Player") bool bIsCarryingOmelette = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Alchemist|Player") FName OmeletteCarrySocketName = FName("CarrySocket");
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Player") void PickUpOmelette(AOmelette* Omelette);
	UFUNCTION(BlueprintCallable, Category = "Alchemist|Player") void DropOmelette();

	// -------------------------------------------------------------------------
	// BLUEPRINT HOOKS
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Player") void OnOmelettePickedUp();
	UFUNCTION(BlueprintImplementableEvent, Category = "Alchemist|Player") void OnOmeletteDropped();
};
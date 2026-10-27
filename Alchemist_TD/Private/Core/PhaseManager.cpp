#include "Core/PhaseManager.h"

#include "Characters/VillagerDefender.h"
#include "Characters/Omelette.h"
#include "Interactables/OmeletteGoal.h"
#include "Core/VillagerDefenderManager.h"
#include "Core/CombatManager.h"
#include "Enemies/EnemyBase.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"

APhaseManager::APhaseManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APhaseManager::BeginPlay()
{
	Super::BeginPlay();
	VillagerDefenderManagerRef = Cast<AVillagerDefenderManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AVillagerDefenderManager::StaticClass()));
	if (!VillagerDefenderManagerRef)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("PhaseManager: VillagerDefenderManager not found in level"));
	}
	else
	{
		VillagerDefenderManagerRef->OnAllVillagersDead.AddDynamic(this, &APhaseManager::TriggerOmeletteCapture);
	}
	CombatManagerRef = Cast<ACombatManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACombatManager::StaticClass()));
	if (!CombatManagerRef)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("PhaseManager: CombatManager not found in level"));
	}
	if (Omelette)
	{
		Omelette->OnOmeletteDropped.AddDynamic(this, &APhaseManager::OnOmeletteDroppedHandler);
	}
	if (!OmeletteGoal)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1,60.f, FColor::Red, TEXT("OmeletteGoal not assigned. Place in level and assign."));
		UE_LOG(LogTemp,Error,TEXT("PhaseManager:OmeletteGoal not assigned. Place in level and assign."))
	}
	else
	{
		OmeletteGoal->OnOmeletteReturned.AddDynamic(this, &APhaseManager::HandleOmeletteReturned);
	}
	if (CombatManagerRef)
	{
		CombatManagerRef->OnEnemyEnterGateAttackZone.AddDynamic(this, &APhaseManager::HandleEnemyEnterGateAttackZone);
		CombatManagerRef->OnGateAttackZoneEmpty.AddDynamic(this, &APhaseManager::HandleGateAttackZoneEmpty);
	}
}
void APhaseManager::EnterVillagerDefence()
{
	if (CurrentPhase == EGamePhase::VillagerDefence) return;
	CurrentPhase = EGamePhase::VillagerDefence;
	OnPhaseChanged.Broadcast(CurrentPhase);
	OnEnterVillagerDefence();
	if (VillagerDefenderManagerRef)
	{
		VillagerDefenderManagerRef->RedistributeVillagers();
	}
}
void APhaseManager::ExitVillagerDefence()
{
	if (CurrentPhase != EGamePhase::VillagerDefence) return;
	CurrentPhase = EGamePhase::LaneDefence;
	OnPhaseChanged.Broadcast(CurrentPhase);
	OnExitVillagerDefence();
}
void APhaseManager::ForceAutoSaveOmelette()
{
	if (bOmeletteSaved) return;
	if (!IsValid(Omelette))
	{
		UE_LOG(LogTemp, Error, TEXT("ForceAutoSaveOmelette: Omelette is null — cannot auto-save"));
		return;
	}
	if (!IsValid(OmeletteGoal))
	{
		UE_LOG(LogTemp, Error, TEXT("ForceAutoSaveOmelette: OmeletteGoal is null — cannot auto-save"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ForceAutoSaveOmelette: teleporting omelette to goal and saving"));

	// Teleport omelette to the goal's location
	Omelette->SetActorLocation(OmeletteGoal->GetActorLocation());

	// Same save path as manual return
	Omelette->OnSavedAtGoalVolume(OmeletteGoal->GetActorLocation());
	HandleOmeletteReturned();   // sets bOmeletteSaved, broadcasts both signals
}
void APhaseManager::HandleEnemyEnterGateAttackZone()
{
	if (CurrentPhase == EGamePhase::LaneDefence)
	{
		EnterVillagerDefence();
	}
}
void APhaseManager::HandleGateAttackZoneEmpty()
{
	ExitVillagerDefence();
}
void APhaseManager::HandleOmeletteReturned()
{
	if (bOmeletteSaved) return;
	bOmeletteSaved = true;
	UE_LOG(LogTemp, Warning, TEXT("PhaseManager: Omelette saved at goal - bOmeletteSaved = true"))
	OnOmeletteSavedSignal.Broadcast();
	OnOmeletteSaved();
}
void APhaseManager::OmelettePhaseBarricadeDestroy()
{
	for (AActor* Barricade : OmelettePhaseBarricades)
	{
		if (!IsValid(Barricade)) continue;
		FVector BarricadeLocation = Barricade->GetActorLocation();
		OnBarricadeDestroyed.Broadcast(BarricadeLocation);
		Barricade->Destroy();
	}
}
void APhaseManager::OnActiveRetrieverLost(AEnemyBase* DyingRetriever)
{
	if (DyingRetriever != ActiveRetriever) return;     // ignore stale notifications
	if (!IsValid(Omelette)) return;
	if (!Omelette->bIsDropped) return;                  // omelette already picked up, nothing to do
	AEnemyBase* ClosestEnemy = FindClosestEnemy(Omelette->GetActorLocation(), OmeletteRetrieveQueryRadius, Omelette);
	if (!ClosestEnemy) return;
	UE_LOG(LogTemp, Warning, TEXT("OnActiveRetrieverLost: reassigning retriever to %s"), *ClosestEnemy->GetName());
	ActiveRetriever = ClosestEnemy;
	ClosestEnemy->StartOmeletteRetrieve(Omelette);
}
void APhaseManager::ResetToLaneDefence()
{
	bOmeletteSaved = false;
	CurrentPhase = EGamePhase::LaneDefence;
	OnPhaseChanged.Broadcast(CurrentPhase);
}
void APhaseManager::TriggerGameOver()
{	
	OnGameOver();
	OnGameOverSignal.Broadcast();
}
void APhaseManager::TriggerOmeletteCapture()
{
	CurrentPhase = EGamePhase::OmeletteCapture;
	OmelettePhaseBarricadeDestroy();
	OnPhaseChanged.Broadcast(CurrentPhase);
	OnEnterOmeletteCapture();
}

//---Helpers---
AEnemyBase* APhaseManager::FindClosestEnemy(FVector Location, float Radius, AActor* ActorToIgnore) const
{
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	if (ActorToIgnore)
	{
		QueryParams.AddIgnoredActor(ActorToIgnore);
	}

	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	AEnemyBase* ClosestEnemy = nullptr;
	float ClosestDistance = FLT_MAX;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(Result.GetActor());
		if (!IsValid(Enemy)) continue;
		if (Enemy->bIsDead) continue;

		float Distance = FVector::Dist(Enemy->GetActorLocation(), Location);
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestEnemy = Enemy;
		}
	}

	return ClosestEnemy;
}
//---OMELETTE MANAGER FUNCTIONS (MOVE LATER)---
void APhaseManager::HandleCapturerKilled()
{
	OnCapturerDied.Broadcast();
}
void APhaseManager::OnOmeletteDroppedHandler(AOmelette* DroppedOmelette)
{
	if (!DroppedOmelette) return;
	AEnemyBase* ClosestEnemy = FindClosestEnemy(DroppedOmelette->GetActorLocation(),OmeletteRetrieveQueryRadius,DroppedOmelette);
	if (!ClosestEnemy) return;
	ActiveRetriever = ClosestEnemy;
	ClosestEnemy->StartOmeletteRetrieve(DroppedOmelette);
}
void APhaseManager::OnPlayerPickedUpOmeletteHandler()
{
	if (!IsValid(ActiveRetriever)) return;
	if (!ActiveRetriever->bIsRetrievingOmelette) return;

	ActiveRetriever->bIsRetrievingOmelette = false;
	ActiveRetriever->bIsCarryingOmelette = false;
	ActiveRetriever->StartChasePlayer();
	ActiveRetriever = nullptr;
}
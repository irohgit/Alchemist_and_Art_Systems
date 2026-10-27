#include "alchemist_0/Public/StatusEffectComponent.h"
#include "Engine/DamageEvents.h"

UStatusEffectComponent::UStatusEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UStatusEffectComponent::ApplyStatus(
    FStatusEffect NewStatus)
{
    switch (NewStatus.Priority)
    {
    case EStatusPriority::Movement_Hard:
        // Freeze — always applies immediately
        // No queue check needed — it wins everything
        ActiveStatuses.Add(NewStatus.StatusType, NewStatus);
        OnStatusApplied.Broadcast(NewStatus.StatusType);
        break;

    case EStatusPriority::Movement_Soft:
        // Fear — check if Freeze is active
        if (HasStatus(EStatusType::Freeze))
        {
            // Queue it — timer starts now, runs during Freeze
            NewStatus.bIsQueued = true;
            MovementQueue.Add(NewStatus);
        }
        else
        {
            ActiveStatuses.Add(NewStatus.StatusType, NewStatus);
            OnStatusApplied.Broadcast(NewStatus.StatusType);
        }
        break;

    case EStatusPriority::Movement_Debuff:
        // Slow — always add, GetMovementMultiplier
        // handles redundancy during Freeze/Fear
        ActiveStatuses.Add(NewStatus.StatusType, NewStatus);
        OnStatusApplied.Broadcast(NewStatus.StatusType);
        break;

    case EStatusPriority::Persistent:
        // Charge, Ignite — always simultaneous
        // If Charge already exists, just refresh
        // If Ignite already exists, refresh duration
        ActiveStatuses.Add(NewStatus.StatusType, NewStatus);
        OnStatusApplied.Broadcast(NewStatus.StatusType);
        break;
    }
}
void UStatusEffectComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UStatusEffectComponent::RemoveStatus(EStatusType StatusType)
{
    ActiveStatuses.Remove(StatusType);
    OnStatusExpired.Broadcast(StatusType);
}

bool UStatusEffectComponent::HasStatus(EStatusType StatusType) const
{
    return ActiveStatuses.Contains(StatusType);
}

FStatusEffect UStatusEffectComponent::GetStatus(EStatusType StatusType) const
{
    if (const FStatusEffect* Found = ActiveStatuses.Find(StatusType))
    {
        return *Found;
    }
    return FStatusEffect();
}

void UStatusEffectComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TickDurations(DeltaTime);
    ProcessMovementQueue();
    
    if (HasStatus(EStatusType::Ignite))
    {
        HandleIgniteTick(DeltaTime);
    }
}

void UStatusEffectComponent::TickDurations(float DeltaTime)
{
    TArray<EStatusType> Expired;
    
    for (auto& Pair : ActiveStatuses)
    {
        FStatusEffect& Status = Pair.Value;
        
        // Skip persistent statuses — no duration
        if (Status.bIsPersistent) continue;
        
        Status.Duration -= DeltaTime;
        
        if (Status.Duration <= 0.0f)
        {
            Expired.Add(Pair.Key);
        }
    }
    
    for (EStatusType ExpiredType : Expired)
    {
        ActiveStatuses.Remove(ExpiredType);
        OnStatusExpired.Broadcast(ExpiredType);
        
        // If Freeze just expired, process movement queue
        if (ExpiredType == EStatusType::Freeze)
        {
            ProcessMovementQueue();
        }
    }
    
    // Tick queued statuses — their timers run during queue
    for (FStatusEffect& Queued : MovementQueue)
    {
        Queued.Duration -= DeltaTime;
    }
    
    // Remove expired queued statuses
    MovementQueue.RemoveAll([](const FStatusEffect& S)
    {
        return S.Duration <= 0.0f;
    });
}

void UStatusEffectComponent::ProcessMovementQueue()
{
    // Only process if Freeze is no longer active
    if (HasStatus(EStatusType::Freeze)) return;
    if (MovementQueue.IsEmpty()) return;
    
    // Take the first queued status with remaining duration
    for (int32 i = 0; i < MovementQueue.Num(); i++)
    {
        if (MovementQueue[i].Duration > 0.0f)
        {
            FStatusEffect NextStatus = MovementQueue[i];
            NextStatus.bIsQueued = false;
            MovementQueue.RemoveAt(i);
            
            // Apply with remaining duration intact
            ActiveStatuses.Add(
                NextStatus.StatusType, NextStatus);
            OnStatusApplied.Broadcast(NextStatus.StatusType);
            return;
        }
    }
}

float UStatusEffectComponent::GetMovementMultiplier() const
{
    // Freeze — hard zero
    if (HasStatus(EStatusType::Freeze)) return 0.0f;
    
    // Fear — negative movement (handled by EnemyBase)
    // Return multiplier for speed only
    if (HasStatus(EStatusType::Fear)) return 1.0f;
    
    // Slow — apply magnitude
    if (HasStatus(EStatusType::Slow))
    {
        return 1.0f - GetStatus(EStatusType::Slow).Magnitude;
    }
    
    return 1.0f;
}

bool UStatusEffectComponent::IsMovementLocked() const
{
    return HasStatus(EStatusType::Freeze);
}

bool UStatusEffectComponent::IsFeared() const
{
    return HasStatus(EStatusType::Fear) && 
           !HasStatus(EStatusType::Freeze);
}

void UStatusEffectComponent::HandleIgniteTick(
    float DeltaTime)
{
    IgniteTickAccumulator += DeltaTime;
    
    if (IgniteTickAccumulator >= IgniteTickRate)
    {
        IgniteTickAccumulator = 0.0f;
        
        // Apply DoT damage via owner's TakeDamage
        if (AActor* Owner = GetOwner())
        {
            // 1 hit of DoT damage per tick
            Owner->TakeDamage(1.0f, FDamageEvent(), 
                nullptr, nullptr);
        }
    }
}
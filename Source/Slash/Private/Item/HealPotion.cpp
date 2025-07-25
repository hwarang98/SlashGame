// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/HealPotion.h"
#include "Interface/PickupInterface.h"

void AHealPotion::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		PickupInterface->AddHealth(this);
		SpawnPickupSystem();
		SpawnPickupSound();
		Destroy();
	}
}

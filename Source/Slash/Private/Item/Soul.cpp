// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Soul.h"
#include "Interface/PickupInterface.h"

void ASoul::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		PickupInterface->AddSoul(this);
		SpawnPickupSystem();
		SpawnPickupSound();
		Destroy();
	}
}

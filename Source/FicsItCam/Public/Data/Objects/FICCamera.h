// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Attributes/FICAttributePosition.h"
#include "Data/Attributes/FICAttributeRotation.h"
#include "Data/Objects/FICSceneObject.h"
#include "FICCamera.generated.h"

UCLASS()
class FICSITCAM_API UFICCamera : public UObject, public IFICSceneObject {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	FFICAttributePosition Position;
	UPROPERTY(SaveGame)
	FFICAttributeRotation Rotation;
	
	UPROPERTY(SaveGame)
	FFICFloatAttribute FOV;
	UPROPERTY(SaveGame)
	FFICFloatAttribute Aperture;
	UPROPERTY(SaveGame)
	FFICFloatAttribute FocusDistance;

	FFICGroupAttribute LensSettings;
	FFICGroupAttribute RootAttribute;

	UFICCamera() {
		LensSettings.AddChildAttribute(TEXT("FOV"), &FOV);
		LensSettings.AddChildAttribute(TEXT("Aperture"), &Aperture);
		LensSettings.AddChildAttribute(TEXT("Focus Distance"), &FocusDistance);
		
		RootAttribute.AddChildAttribute(TEXT("Position"), &Position);
		RootAttribute.AddChildAttribute(TEXT("Rotation"), &Rotation);
		RootAttribute.AddChildAttribute(TEXT("Lens Settings"), &LensSettings);
	}

	// Begin IFICEditorSceneObject-Interface
	FText GetSceneObjectName() override {
		return FText::FromString(TEXT("Camera"));
	}
	
	FFICAttribute& GetRootAttribute() override {
		return RootAttribute;
	}
	// End IFICEditorSceneObject-Interface
};

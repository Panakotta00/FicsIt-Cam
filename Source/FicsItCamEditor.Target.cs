// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class FicsItCamEditorTarget : TargetRules
{
	public FicsItCamEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
        ExtraModuleNames.AddRange( new string[] { "FactoryGame", "SML", "FicsItCam" } );
	}
}

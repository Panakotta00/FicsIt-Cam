#pragma once

#include "SlateBasics.h"

class FFICEditorAttributeBase;
DECLARE_DELEGATE_TwoParams(FFICVectorValueCommitted, float /* New Value */, ETextCommit::Type /*CommitType*/)
DECLARE_DELEGATE_OneParam(FFICVectorValueChanged, float /* New Value */)

class SFICVectorEditor : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFICVectorEditor) :
		_ShowKeyframeControls(false) {}
		SLATE_ARGUMENT(bool, ShowKeyframeControls)
		SLATE_ATTRIBUTE(TOptional<float>, X)
		SLATE_ATTRIBUTE(TOptional<float>, Y)
		SLATE_ATTRIBUTE(TOptional<float>, Z)
		SLATE_ATTRIBUTE(FFICEditorAttributeBase*, XAttr)
		SLATE_ATTRIBUTE(FFICEditorAttributeBase*, YAttr)
		SLATE_ATTRIBUTE(FFICEditorAttributeBase*, ZAttr)
		SLATE_ATTRIBUTE(TOptional<int64>, Frame)
		SLATE_ATTRIBUTE(bool, AutoKeyframe)
		SLATE_EVENT(FFICVectorValueCommitted, OnXCommitted)
		SLATE_EVENT(FFICVectorValueCommitted, OnYCommitted)
		SLATE_EVENT(FFICVectorValueCommitted, OnZCommitted)
		SLATE_EVENT(FFICVectorValueChanged, OnXChanged)
		SLATE_EVENT(FFICVectorValueChanged, OnYChanged)
		SLATE_EVENT(FFICVectorValueChanged, OnZChanged)
	SLATE_END_ARGS()
	
public:
	SFICVectorEditor();
	
	void Construct(FArguments InArgs);

private:
	TAttribute<TOptional<float>> X;
	TAttribute<TOptional<float>> Y;
	TAttribute<TOptional<float>> Z;
	TAttribute<FFICEditorAttributeBase*> XAttr;
	TAttribute<FFICEditorAttributeBase*> YAttr;
	TAttribute<FFICEditorAttributeBase*> ZAttr;
	TAttribute<TOptional<int64>> Frame;
	TAttribute<bool> AutoKeyframe;
	FFICVectorValueCommitted OnXCommitted;
	FFICVectorValueCommitted OnYCommitted;
	FFICVectorValueCommitted OnZCommitted;
	FFICVectorValueChanged OnXChanged;
	FFICVectorValueChanged OnYChanged;
	FFICVectorValueChanged OnZChanged;
	FSlateColorBrush XBrush = FSlateColorBrush(FLinearColor::Red);
	FSlateColorBrush YBrush = FSlateColorBrush(FLinearColor::Green);
	FSlateColorBrush ZBrush = FSlateColorBrush(FLinearColor::Blue);
	FSpinBoxStyle SpinBoxStyle;
};

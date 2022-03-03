#include "Editor/UI/FICKeyframesView.h"

FVector2D SFICKeyframesView::ComputeDesiredSize(float) const {
	return FVector2D(0, 0);
}

int32 SFICKeyframesView::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	return SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

bool SFICKeyframesView::IsInteractable() const {
	return true;
}

FChildren* SFICKeyframesView::GetChildren() {
	return &Children;
}

void SFICKeyframesView::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	
}

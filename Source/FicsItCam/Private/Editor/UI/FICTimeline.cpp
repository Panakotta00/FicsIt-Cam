#include "Editor/UI/FICTimeline.h"

#include "Editor/FICEditorContext.h"
#include "Widgets/Input/SNumericEntryBox.h"

FSlateColorBrush SFICTimelinePanel::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));

FCheckBoxStyle SFICTimelinePanel::DefaultToggleButtonStyle = FCoreStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonStyle");
FSlateBoxBrush SFICTimelinePanel::DefaultToggleButtonChecked = FSlateBoxBrush(((FSlateStyleSet&)FCoreStyle::Get()).RootToContentDir("Common/RoundedSelection_16x",  TEXT(".png")), 4.0f/16.0f, FLinearColor(0.25f, 0.25f, 0.25f));
FSlateBoxBrush SFICTimelinePanel::DefaultToggleButtonUnchecked = FSlateBoxBrush(((FSlateStyleSet&)FCoreStyle::Get()).RootToContentDir("Common/RoundedSelection_16x",  TEXT(".png")), 4.0f/16.0f, FLinearColor(0.72f, 0.72f, 0.72f));

void SFICTimelinePanel::Construct(const FArguments& InArgs) {
	Context = InArgs._Context;
	BackgroundBrush = InArgs._Background;
	
	DefaultToggleButtonStyle.CheckedImage = static_cast<FSlateBrush>(DefaultToggleButtonChecked);
	DefaultToggleButtonStyle.CheckedHoveredImage = static_cast<FSlateBrush>(DefaultToggleButtonChecked);
	DefaultToggleButtonStyle.CheckedPressedImage = static_cast<FSlateBrush>(DefaultToggleButtonChecked);
	DefaultToggleButtonStyle.UncheckedImage = static_cast<FSlateBrush>(DefaultToggleButtonUnchecked);
	DefaultToggleButtonStyle.UncheckedHoveredImage = static_cast<FSlateBrush>(DefaultToggleButtonUnchecked);
	DefaultToggleButtonStyle.UncheckedPressedImage = static_cast<FSlateBrush>(DefaultToggleButtonUnchecked);
	
	for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Attribute : Context->GetAllAttributes()->GetChildAttributes()) {
		Attributes.Add(MakeShared<FFICEditorAttributeReference>(Attribute.Key, &*Attribute.Value));
	}

	TSharedPtr<INumericTypeInterface<int64>> Interface = MakeShared<TDefaultNumericTypeInterface<int64>>();

	ChildSlot[SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(BackgroundBrush)
		]
		+SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)[
			SNew(SGridPanel)
			.FillColumn(1, 1)
			.FillRow(2, 1)
			+SGridPanel::Slot(0,0)
			.RowSpan(2)
			.Padding(5)[
				SNew(SGridPanel)
				+SGridPanel::Slot(0,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("Animation Start:"))
	            ]
	            +SGridPanel::Slot(0, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
	                SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .Value_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.Begin;
	                    })
	                    .MaxValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.End-1;
	                    })
	                    .MinValue(TNumericLimits<int64>::Min())
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        Context->GetScene()->AnimationRange.Begin = Value;
	                    })
	                ]
	            ]
	            +SGridPanel::Slot(1,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("View Start:"))
	            ]
	            +SGridPanel::Slot(1, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
	                SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .MaxValue_Lambda([this]() {
	                        return Context->GetActiveRange().End-1;
	                    })
	                    .MaxSliderValue_Lambda([this]() {
	                        return Context->GetActiveRange().End-1;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.Begin;
	                    })
	                    .MinSliderValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.Begin;
	                    })
	                    .Value_Lambda([this]() {
	                        return Context->GetActiveRange().Begin;
	                    })
	                    .OnValueChanged_Lambda([this](FICFrame Value) {
	                    	FFICFrameRange Range = Context->GetActiveRange();
	                    	Range.Begin = FMath::Clamp(Value, Context->GetScene()->AnimationRange.Begin, Context->GetActiveRange().End-1);
	                        Context->SetActiveRange(Range);
	                    })
	                    .OnValueCommitted_Lambda([this](FICFrame Value, ETextCommit::Type) {
	                    	FFICFrameRange Range = Context->GetActiveRange();
	                    	Range.Begin = FMath::Clamp(Value, Context->GetScene()->AnimationRange.Begin, Context->GetActiveRange().End-1);
	                        Context->SetActiveRange(Range);
	                    })
	                    .AllowSpin(true)
	                ]
	            ]
	            +SGridPanel::Slot(2, 1)
	            .Padding(5)
	            .HAlign(HAlign_Center)
	            .VAlign(VAlign_Center)[
					SNew(SCheckBox)
					.Type(ESlateCheckBoxType::ToggleButton)
					.Style(&DefaultToggleButtonStyle)
					.Padding(5)
					.Content()[
						SNew(STextBlock)
						.Text(FText::FromString("<"))
						.ColorAndOpacity(FColor::Black)
					]
					.ToolTipText_Lambda([this]() {
						bool Ctrl = FSlateApplication::Get().GetModifierKeys().IsControlDown() && Context->GetAnimPlayer() != FIC_PLAY_PAUSED;
						float Factor = Context->GetAnimPlayerFactor();
						if (Context->GetAnimPlayer() == FIC_PLAY_BACKWARDS) {
							return FText::FromString(Ctrl ? FString::Printf(TEXT("Increate to %ix Reverse Play"), (int)Factor+1) : TEXT("Pause Play"));
						} else {
							return FText::FromString(Ctrl ? FString::Printf(TEXT("Decrease to %ix Play"), (int)Factor-1) : TEXT("Play Reverse"));
						}
					})
					.IsChecked_Lambda([this]() {
						return (Context->GetAnimPlayer() == FIC_PLAY_BACKWARDS) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
						bool Ctrl = FSlateApplication::Get().GetModifierKeys().IsControlDown();
						float Factor = Context->GetAnimPlayerFactor();
						switch (Context->GetAnimPlayer()) {
						case FIC_PLAY_PAUSED:
							Context->SetAnimPlayer(EFICAnimPlayerState::FIC_PLAY_BACKWARDS, Factor = 1);
							break;
						case FIC_PLAY_FORWARDS:
							if (Ctrl) {
								Factor -= 1;
							} else {
								Factor = 0;
							}
							break;
						case FIC_PLAY_BACKWARDS:
							if (Ctrl) {
								Factor += 1;
							} else {
								Factor = 0;
							}
							break;
						}
						if (FMath::Abs(Factor) < 0.1) {
							Context->SetAnimPlayer(EFICAnimPlayerState::FIC_PLAY_PAUSED, 0);
						} else {
							Context->SetAnimPlayer(Context->GetAnimPlayer(), Factor);
						}
					})
	            ]
	            +SGridPanel::Slot(3,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("Current Frame:"))
	            ]
	            +SGridPanel::Slot(3, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
	                SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .MaxValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.End;
	                    })
	                    .MaxSliderValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.End;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.Begin;
	                    })
	                    .MinSliderValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.Begin;
	                    })
	                    .Value_Lambda([this]() {
	                        return Context->GetCurrentFrame();
	                    })
	                    .OnValueChanged_Lambda([this](int64 Value) {
	                        Context->SetCurrentFrame(Value);
	                    })
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        Context->SetCurrentFrame(Value);
	                    })
	                    .AllowSpin(true)
	                ]
	            ]
	            +SGridPanel::Slot(4, 1)
	            .Padding(5)
	            .VAlign(VAlign_Center)
	            .HAlign(HAlign_Center)[
		            SNew(SCheckBox)
		            .Type(ESlateCheckBoxType::ToggleButton)
					.Style(&DefaultToggleButtonStyle)
					.Padding(5)
					.Content()[
						SNew(STextBlock)
						.Text(FText::FromString(">"))
						.ColorAndOpacity(FColor::Black)
					]
					.ToolTipText_Lambda([this]() {
						bool Ctrl = FSlateApplication::Get().GetModifierKeys().IsControlDown() && Context->GetAnimPlayer() != FIC_PLAY_PAUSED;
						float Factor = Context->GetAnimPlayerFactor();
						if (Context->GetAnimPlayer() == FIC_PLAY_FORWARDS) {
							return FText::FromString(Ctrl ? FString::Printf(TEXT("Increate to %ix Play"), (int)Factor+1) : TEXT("Pause Play"));
						} else {
							return FText::FromString(Ctrl ? FString::Printf(TEXT("Decrease to %ix Reverse Play"), (int)Factor+1) : TEXT("Play"));
						}
					})
					.IsChecked_Lambda([this]() {
						return (Context->GetAnimPlayer() == FIC_PLAY_FORWARDS) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
						bool Ctrl = FSlateApplication::Get().GetModifierKeys().IsControlDown();
						float Factor = Context->GetAnimPlayerFactor();
						switch (Context->GetAnimPlayer()) {
						case FIC_PLAY_PAUSED:
							Context->SetAnimPlayer(EFICAnimPlayerState::FIC_PLAY_FORWARDS, Factor = 1);
							break;
						case FIC_PLAY_BACKWARDS:
							if (Ctrl) {
								Factor -= 1;
							} else {
								Factor = 0;
							}
							break;
						case FIC_PLAY_FORWARDS:
							if (Ctrl) {
								Factor += 1;
							} else {
								Factor = 0;
							}
							break;
						}
						if (FMath::Abs(Factor) < 0.1) {
							Context->SetAnimPlayer(EFICAnimPlayerState::FIC_PLAY_PAUSED, 0);
						} else {
							Context->SetAnimPlayer(Context->GetAnimPlayer(), Factor);
						}
					})
				]
	            +SGridPanel::Slot(5,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("View End:"))
	            ]
	            +SGridPanel::Slot(5, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
	                SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .MaxValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.End;
	                    })
	                    .MaxSliderValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.End;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetActiveRange().Begin+1;
	                    })
	                    .MinSliderValue_Lambda([this]() {
	                        return Context->GetActiveRange().Begin+1;
	                    })
	                    .Value_Lambda([this]() {
	                        return Context->GetActiveRange().End;
	                    })
	                    .OnValueChanged_Lambda([this](FICFrame Value) {
	                    	FFICFrameRange Range = Context->GetActiveRange();
	                        Range.End = FMath::Clamp(Value, Range.Begin+1, Context->GetScene()->AnimationRange.End);
	                    	Context->SetActiveRange(Range);
	                    })
	                    .OnValueCommitted_Lambda([this](FICFrame Value, ETextCommit::Type) {
	                    	FFICFrameRange Range = Context->GetActiveRange();
	                        Range.End = FMath::Clamp(Value, Range.Begin+1, Context->GetScene()->AnimationRange.End);
	                    	Context->SetActiveRange(Range);
	                    })
	                    .AllowSpin(true)
	                ]
	            ]
	            +SGridPanel::Slot(6,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("Animation End:"))
	            ]
	            +SGridPanel::Slot(6, 1).Padding(5)[
	                SNew(SBox)
	                .WidthOverride(50)
	                .Content()[
						SNew(SNumericEntryBox<int64>)
	                    .TypeInterface(Interface)
	                    .Value_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.End;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetScene()->AnimationRange.Begin+1;
	                    })
	                    .MaxValue(TNumericLimits<int64>::Max())
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        Context->GetScene()->AnimationRange.End = FMath::Clamp(Value, Context->GetScene()->AnimationRange.Begin+1, TNumericLimits<int64>::Max());
	                    })
	                ]
	            ]
			]
			+SGridPanel::Slot(0, 2).Padding(5).VAlign(VAlign_Fill).HAlign(HAlign_Fill)[
				SNew(SVerticalBox)
				+SVerticalBox::Slot().AutoHeight()[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Attributes shown in Graph View:")))
				]
				+SVerticalBox::Slot().Padding(5).FillHeight(1).VAlign(VAlign_Fill).HAlign(HAlign_Fill)[
					SAssignNew(AttributeTree, STreeView<TSharedPtr<FFICEditorAttributeReference>>)
					.TreeItemsSource(&Attributes)
					.SelectionMode(ESelectionMode::None)
					.OnGenerateRow_Lambda([this](TSharedPtr<FFICEditorAttributeReference> Attribute, const TSharedRef<STableViewBase>& Base) {
						return SNew(STableRow<TSharedPtr<FFICEditorAttributeReference>>, Base)
						.Content()[
							SNew(SCheckBox)
							.Content()[
								SNew(STextBlock)
								.Text(FText::FromString(Attribute->Name))
							].IsChecked_Lambda([Attribute]() {
								if (Attribute->Attribute->GetChildAttributes().Num() < 1) return Attribute->Attribute->bShowInGraph ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
								TFunction<bool(FFICEditorAttributeBase*)> HasCheckedChild;
								ECheckBoxState State = ECheckBoxState::Checked;
								HasCheckedChild = [&HasCheckedChild, &State](FFICEditorAttributeBase* Attrib) {
									bool bAtLeastOne = false;
									for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Child : Attrib->GetChildAttributes()) {
										if (Child.Value->GetChildAttributes().Num() < 1) {
											if (Child.Value->bShowInGraph) {
												bAtLeastOne = true;
											} else {
												State = ECheckBoxState::Undetermined;
											}
										} else {
											bAtLeastOne = HasCheckedChild(&*Child.Value) || bAtLeastOne;
										}
									}
									return bAtLeastOne;
								};
								if (HasCheckedChild(Attribute->Attribute)) return State;
								return ECheckBoxState::Unchecked;
							})
							.OnCheckStateChanged_Lambda([this, Attribute](ECheckBoxState State) {
								TFunction<void(FFICEditorAttributeBase*)> SetChildren;
								SetChildren = [&SetChildren, State](FFICEditorAttributeBase* Attrib) {
									Attrib->bShowInGraph = State == ECheckBoxState::Checked;
									for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Child : Attrib->GetChildAttributes()) {
										SetChildren(&*Child.Value);
									}
								};
								SetChildren(Attribute->Attribute);
								UpdateLeafAttributes();
							})
						];
					})
					.OnGetChildren_Lambda([](TSharedPtr<FFICEditorAttributeReference> InEntry, TArray<TSharedPtr<FFICEditorAttributeReference>>& OutArray) {
						for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Attribute : InEntry->Attribute->GetChildAttributes()) {
							OutArray.Add(MakeShared<FFICEditorAttributeReference>(Attribute.Key, &*Attribute.Value));
						}
					})
				]
			]
			+SGridPanel::Slot(1, 0)[
				SAssignNew(VisibleRange, SFICRangeSelector)
				.ActiveFrameEnabled(true)
				.ActiveFrame_Lambda([this]() {
					return Context->GetCurrentFrame();
				})
				.OnActiveFrameChanged_Lambda([this](FICFrame Frame) {
					Context->SetCurrentFrame(Frame);
				})
				.FullRange_Lambda([this]() {
					return Context->GetScene()->AnimationRange;
				})
				.ActiveRange_Lambda([this]() {
					return Context->GetActiveRange();
				})
				.OnActiveRangeChanged_Lambda([this](FFICFrameRange Range) {
					Context->SetActiveRange(Range);
				})
			]
			+SGridPanel::Slot(1, 1)[
				SAssignNew(Scrubber, SFICTimelineScrubber)
				.ActiveFrame_Lambda([this]() {
					return Context->GetCurrentFrame();
				})
				.OnActiveFrameChanged_Lambda([this](FICFrame Cur) {
					Context->SetCurrentFrame(Cur);
				})
				.ActiveRange_Lambda([this]() {
		            return Context->GetActiveRange();
		        })
		        .FullRange_Lambda([this]() {
		            return Context->GetScene()->AnimationRange;
		        })
			]
			+SGridPanel::Slot(1, 2)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)[
				SAssignNew(Graph, SFICGraphView, Context)
				.Attributes({})
				.AutoFit(true)
				.Clipping(EWidgetClipping::ClipToBoundsAlways)
				.Frame_Lambda([this]() {
					return Context->GetCurrentFrame();
				})
				.FrameRange_Lambda([this]() {
					return Context->GetActiveRange();
				})
				.FrameHighlightRange_Lambda([this]() {
					return Context->GetScene()->AnimationRange;
				})
				.ValueRange_Lambda([this]() {
					return ActiveValueRange;
				})
				.OnFrameRangeChanged_Lambda([this](FFICFrameRange Range) {
					Context->SetActiveRange(Range);
				})
				.OnValueRangeChanged_Lambda([this](FFICValueRange Range) {
					ActiveValueRange = Range;
				})
			]
		]
	];
	
	// TArray<FFICEditorAttributeBase*> Attributes;
	// Attributes.Add(Context->All.GetAttributes()["X"].Get());
	// Graph->SetAttributes(Attributes);
}

void SFICTimelinePanel::UpdateLeafAttributes() {
	SelectedLeafAttributes.Empty();
	TFunction<void(FFICEditorAttributeBase*)> AddLeaves;
	AddLeaves = [this, &AddLeaves](FFICEditorAttributeBase* Attribute) {
		if (Attribute->GetChildAttributes().Num() < 1 && Attribute->bShowInGraph) SelectedLeafAttributes.Add(Attribute); 
		for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Child : Attribute->GetChildAttributes()) {
			AddLeaves(&*Child.Value);
		}
	};
	for (TSharedPtr<FFICEditorAttributeReference> Item : Attributes) {
		AddLeaves(Item->Attribute);
	}
	Graph->SetAttributes(SelectedLeafAttributes);
	Graph->FitAll();
}

void SFICTimelinePanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	//ActiveRangeStart = FMath::Clamp(ActiveRangeStart, Context->GetScene()->AnimationRange.Begin, ActiveRangeEnd-1);
	//ActiveRangeEnd = FMath::Clamp(ActiveRangeEnd, ActiveRangeStart+1, Context->GetScene()->AnimationRange.End);
}

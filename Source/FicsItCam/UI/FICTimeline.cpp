#include "FICTimeline.h"

#include "FICEditorContext.h"
#include "FICNumericType.h"
#include "FICRangeSelector.h"
#include "FICTimelineScrubber.h"
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
	
	for (TTuple<FString, TAttribute<FFICEditorAttributeBase*>> Attribute : Context->All.GetChildAttributes()) {
		Attributes.Add(MakeShared<FFICEditorAttributeReference>(Attribute.Key, Attribute.Value.Get()));
	}

	ActiveRangeStart = Context->GetAnimation()->AnimationStart;
	ActiveRangeEnd = Context->GetAnimation()->AnimationEnd;

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
	                        return Context->GetAnimation()->AnimationStart;
	                    })
	                    .MaxValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationEnd-1;
	                    })
	                    .MinValue(TNumericLimits<int64>::Min())
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        Context->GetAnimation()->AnimationStart = Value;
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
	                        return ActiveRangeEnd-1;
	                    })
	                    .MaxSliderValue_Lambda([this]() {
	                        return ActiveRangeEnd-1;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart;
	                    })
	                    .MinSliderValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart;
	                    })
	                    .Value_Lambda([this]() {
	                        return ActiveRangeStart;
	                    })
	                    .OnValueChanged_Lambda([this](int64 Value) {
	                        ActiveRangeStart = FMath::Clamp(Value, Context->GetAnimation()->AnimationStart, ActiveRangeEnd-1);
	                    })
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        ActiveRangeStart = FMath::Clamp(Value, Context->GetAnimation()->AnimationStart, ActiveRangeEnd-1);
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
						if (Context->GetAnimPlayer() == FIC_PLAY_BACKWARDS) {
							return FText::FromString(TEXT("Pause Play"));
						} else {
							return FText::FromString(TEXT("Play Reverse"));
						}
					})
					.IsChecked_Lambda([this]() {
						return (Context->GetAnimPlayer() == FIC_PLAY_BACKWARDS) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
						if (Context->GetAnimPlayer() == FIC_PLAY_BACKWARDS) {
							Context->SetAnimPlayer(EFICAnimPlayerState::FIC_PLAY_PAUSED);
						} else {
							Context->SetAnimPlayer(EFICAnimPlayerState::FIC_PLAY_BACKWARDS);
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
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MaxSliderValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart;
	                    })
	                    .MinSliderValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart;
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
						if (Context->GetAnimPlayer() == FIC_PLAY_FORWARDS) {
							return FText::FromString(TEXT("Pause Play"));
						} else {
							return FText::FromString(TEXT("Play"));
						}
					})
					.IsChecked_Lambda([this]() {
						return (Context->GetAnimPlayer() == FIC_PLAY_FORWARDS) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
						if (Context->GetAnimPlayer() == FIC_PLAY_FORWARDS) {
							Context->SetAnimPlayer(EFICAnimPlayerState::FIC_PLAY_PAUSED);
						} else {
							Context->SetAnimPlayer(EFICAnimPlayerState::FIC_PLAY_FORWARDS);
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
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MaxSliderValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return ActiveRangeStart+1;
	                    })
	                    .MinSliderValue_Lambda([this]() {
	                        return ActiveRangeStart+1;
	                    })
	                    .Value_Lambda([this]() {
	                        return ActiveRangeEnd;
	                    })
	                    .OnValueChanged_Lambda([this](int64 Value) {
	                        ActiveRangeEnd = FMath::Clamp(Value, ActiveRangeStart+1, Context->GetAnimation()->AnimationEnd);
	                    })
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        ActiveRangeEnd = FMath::Clamp(Value, ActiveRangeStart+1, Context->GetAnimation()->AnimationEnd);
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
	                        return Context->GetAnimation()->AnimationEnd;
	                    })
	                    .MinValue_Lambda([this]() {
	                        return Context->GetAnimation()->AnimationStart+1;
	                    })
	                    .MaxValue(TNumericLimits<int64>::Max())
	                    .OnValueCommitted_Lambda([this](int64 Value, ETextCommit::Type) {
	                        Context->GetAnimation()->AnimationEnd = FMath::Clamp(Value, Context->GetAnimation()->AnimationStart+1, TNumericLimits<int64>::Max());
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
									for (TTuple<FString, TAttribute<FFICEditorAttributeBase*>> Child : Attrib->GetChildAttributes()) {
										if (Child.Value.Get()->GetChildAttributes().Num() < 1) {
											if (Child.Value.Get()->bShowInGraph) {
												bAtLeastOne = true;
											} else {
												State = ECheckBoxState::Undetermined;
											}
										} else {
											bAtLeastOne = HasCheckedChild(Child.Value.Get()) || bAtLeastOne;
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
									for (TTuple<FString, TAttribute<FFICEditorAttributeBase*>> Child : Attrib->GetChildAttributes()) {
										SetChildren(Child.Value.Get());
									}
								};
								SetChildren(Attribute->Attribute);
								UpdateLeafAttributes();
							})
						];
					})
					.OnGetChildren_Lambda([](TSharedPtr<FFICEditorAttributeReference> InEntry, TArray<TSharedPtr<FFICEditorAttributeReference>>& OutArray) {
						for (TTuple<FString, TAttribute<FFICEditorAttributeBase*>> Attribute : InEntry->Attribute->GetChildAttributes()) {
							OutArray.Add(MakeShared<FFICEditorAttributeReference>(Attribute.Key, Attribute.Value.Get()));
						}
					})
				]
			]
			+SGridPanel::Slot(1, 0)[
				SAssignNew(VisibleRange, SFICRangeSelector)
				.HighlightEnabled(true)
				.Highlight_Lambda([this]() {
					return Context->GetCurrentFrame();
				})
				.HighlightChanged_Lambda([this](int64 Prev, int64 Cur) {
					Context->SetCurrentFrame(Cur);
				})
				.RangeStart_Lambda([this]() {
					return Context->GetAnimation()->AnimationStart;
				})
				.RangeEnd_Lambda([this]() {
					return Context->GetAnimation()->AnimationEnd;
				})
				.SelectStart_Lambda([this]() {
					return ActiveRangeStart;
				})
				.SelectEnd_Lambda([this]() {
					return ActiveRangeEnd;
				})
				.SelectStartChanged(this, &SFICTimelinePanel::ActiveRangeStartChanged)
				.SelectEndChanged(this, &SFICTimelinePanel::ActiveRangeEndChanged)
			]
			+SGridPanel::Slot(1, 1)[
				SAssignNew(Scrubber, SFICTimelineScrubber)
				.Frame_Lambda([this]() {
					return Context->GetCurrentFrame();
				})
				.FrameChanged_Lambda([this](int64 Prev, int64 Cur) {
					Context->SetCurrentFrame(Cur);
				})
				.RangeStart_Lambda([this]() {
		            return ActiveRangeStart;
		        })
		        .RangeEnd_Lambda([this]() {
		            return ActiveRangeEnd;
		        })
		        .AnimationStart_Lambda([this]() {
			        return Context->GetAnimation()->AnimationStart;
		        })
		        .AnimationEnd_Lambda([this]() {
			        return Context->GetAnimation()->AnimationEnd;
		        })
			]
			+SGridPanel::Slot(1, 2)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)[
				SAssignNew(Graph, SFICGraphView, Context)
				.Attributes({&Context->PosX, &Context->PosY, &Context->PosZ})
				.AutoFit(true)
				.Clipping(EWidgetClipping::ClipToBoundsAlways)
				.Frame_Lambda([this]() {
					return Context->GetCurrentFrame();
				})
				.TimelineRangeBegin_Lambda([this]() {
					return ActiveRangeStart;
				})
				.TimelineRangeEnd_Lambda([this]() {
					return ActiveRangeEnd;
				})
				.AnimationStart_Lambda([this]() {
				return Context->GetAnimation()->AnimationStart;
				})
				.AnimationEnd_Lambda([this]() {
					return Context->GetAnimation()->AnimationEnd;
				})
				.OnTimelineRangedChanged_Lambda([this](int64 Begin, int64 End) {
					ActiveRangeStartChanged(ActiveRangeStart, Begin);
					ActiveRangeEndChanged(ActiveRangeEnd, End);
				})
			]
		]
	];
	
	// TArray<FFICEditorAttributeBase*> Attributes;
	// Attributes.Add(Context->All.GetAttributes()["X"].Get());
	// Graph->SetAttributes(Attributes);
}

void SFICTimelinePanel::ActiveRangeStartChanged(int64 Prev, int64 Cur) {
	ActiveRangeStart = Cur;
}

void SFICTimelinePanel::ActiveRangeEndChanged(int64 Prev, int64 Cur) {
	ActiveRangeEnd = Cur;
}

void SFICTimelinePanel::UpdateLeafAttributes() {
	SelectedLeafAttributes.Empty();
	TFunction<void(FFICEditorAttributeBase*)> AddLeaves;
	AddLeaves = [this, &AddLeaves](FFICEditorAttributeBase* Attribute) {
		if (Attribute->GetChildAttributes().Num() < 1 && Attribute->bShowInGraph) SelectedLeafAttributes.Add(Attribute); 
		for (TTuple<FString, TAttribute<FFICEditorAttributeBase*>> Child : Attribute->GetChildAttributes()) {
			AddLeaves(Child.Value.Get());
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

	//ActiveRangeStart = FMath::Clamp(ActiveRangeStart, Context->GetAnimation()->AnimationStart, ActiveRangeEnd-1);
	//ActiveRangeEnd = FMath::Clamp(ActiveRangeEnd, ActiveRangeStart+1, Context->GetAnimation()->AnimationEnd);
}

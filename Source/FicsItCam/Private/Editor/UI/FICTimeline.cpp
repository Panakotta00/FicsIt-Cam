#include "Editor/UI/FICTimeline.h"

#include "Editor/FICEditorContext.h"
#include "Editor/UI/FICSequencerTreeView.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

FSlateColorBrush SFICTimelinePanel::DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex("030303"));

FCheckBoxStyle SFICTimelinePanel::DefaultToggleButtonStyle = FCoreStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonStyle");
FSlateBoxBrush SFICTimelinePanel::DefaultToggleButtonChecked = FSlateBoxBrush(((FSlateStyleSet&)FCoreStyle::Get()).RootToContentDir("Common/RoundedSelection_16x",  TEXT(".png")), 4.0f/16.0f, FLinearColor(0.25f, 0.25f, 0.25f));
FSlateBoxBrush SFICTimelinePanel::DefaultToggleButtonUnchecked = FSlateBoxBrush(((FSlateStyleSet&)FCoreStyle::Get()).RootToContentDir("Common/RoundedSelection_16x",  TEXT(".png")), 4.0f/16.0f, FLinearColor(0.72f, 0.72f, 0.72f));

const FName FFICTimelineStyle::TypeName = TEXT("FFICTimelineStyle");

const FFICTimelineStyle& FFICTimelineStyle::GetDefault() {
	static FFICTimelineStyle* Default = nullptr;
	if (!Default) {
		Default = new FFICTimelineStyle();
		*Default = FFICEditorStyles::Get().GetWidgetStyle<FFICTimelineStyle>("TimelineStyle");
	}
	return *Default;
}

TArray<TSharedPtr<FFICEditorAttributeReference>> FFICEditorAttributeReference::GetChildren() {
	if (!bChildrenLoaded) {
		for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Attr : Attribute->GetChildAttributes()) {
			Children.Add(MakeShared<FFICEditorAttributeReference>(Attr.Key, Attr.Value));
		}
		bChildrenLoaded = true;
	}
	return Children;
}

void SFICTimelinePanel::Construct(const FArguments& InArgs, UFICEditorContext* InContext) {
	Context = InContext;
	Style = InArgs._Style;
	BackgroundBrush = InArgs._Background;
	
	DefaultToggleButtonStyle.CheckedImage = static_cast<FSlateBrush>(DefaultToggleButtonChecked);
	DefaultToggleButtonStyle.CheckedHoveredImage = static_cast<FSlateBrush>(DefaultToggleButtonChecked);
	DefaultToggleButtonStyle.CheckedPressedImage = static_cast<FSlateBrush>(DefaultToggleButtonChecked);
	DefaultToggleButtonStyle.UncheckedImage = static_cast<FSlateBrush>(DefaultToggleButtonUnchecked);
	DefaultToggleButtonStyle.UncheckedHoveredImage = static_cast<FSlateBrush>(DefaultToggleButtonUnchecked);
	DefaultToggleButtonStyle.UncheckedPressedImage = static_cast<FSlateBrush>(DefaultToggleButtonUnchecked);
	
	TSharedPtr<INumericTypeInterface<int64>> Interface = MakeShared<TDefaultNumericTypeInterface<int64>>();

	ChildSlot[SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SImage)
			.Image(BackgroundBrush)
		]
		+SOverlay::Slot()
		.Padding(0)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)[
			SNew(SGridPanel)
			.FillColumn(1, 1)
			.FillRow(2, 1)
			+SGridPanel::Slot(0,0)
			.RowSpan(2)
			.Padding(0)[
				SNew(SGridPanel)
				+SGridPanel::Slot(0, 0)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(0)[
					SNew(SButton)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Content()[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						.Content()[
							SNew(SBox)
							.Padding(0)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.MaxDesiredHeight(10)
							.MaxDesiredWidth(10)
							.Content()[
								SNew(SImage)
								.Image_Lambda([this]() {
									switch (Mode) {
									case 0:
										return &Style->SequencerIcon;
									case 1:
										return &Style->GraphViewIcon;
									default:
										return &Style->SequencerIcon;
									}
								})
								.ColorAndOpacity(FSlateColor::UseForeground())
							]
						]
					]
					.ToolTipText_Lambda([this]() {
						switch (Mode) {
						default:
						case 0:
							return FText::FromString(TEXT("Switch to Sequencer Mode"));
						case 1:
							return FText::FromString(TEXT("Switch to Graph Mode"));
						}
					})
					.OnClicked_Lambda([this]() {
						++Mode;
						if (Mode > 1) Mode = 0;
						return FReply::Handled();
					})
				]
				+SGridPanel::Slot(1,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("Animation Start:"))
	            ]
	            +SGridPanel::Slot(1, 1).Padding(5)[
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
	                        Context->GetScene()->AnimationRange.SetBegin_Clamped(Value);
	                    })
	                ]
	            ]
	            +SGridPanel::Slot(2,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("View Start:"))
	            ]
	            +SGridPanel::Slot(2, 1).Padding(5)[
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
	            +SGridPanel::Slot(3, 1)
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
	            +SGridPanel::Slot(4,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("Current Frame:"))
	            ]
	            +SGridPanel::Slot(4, 1).Padding(5)[
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
	            +SGridPanel::Slot(5, 1)
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
	            +SGridPanel::Slot(6,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("View End:"))
	            ]
	            +SGridPanel::Slot(6, 1).Padding(5)[
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
	            +SGridPanel::Slot(7,0).Padding(5)[
	                SNew(STextBlock)
	                .Text(FText::FromString("Animation End:"))
	            ]
	            +SGridPanel::Slot(7, 1).Padding(5)[
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
	                        Context->GetScene()->AnimationRange.SetEnd_Clamped(Value);
	                    })
	                ]
	            ]
			]
			+SGridPanel::Slot(0, 2).Padding(5).VAlign(VAlign_Fill).HAlign(HAlign_Fill)[
				SNew(SWidgetSwitcher)
				.WidgetIndex_Lambda([this]() {
					return Mode;
				})
				+SWidgetSwitcher::Slot()[
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
									TFunction<bool(TSharedRef<FFICEditorAttributeBase>)> HasCheckedChild;
									ECheckBoxState State = ECheckBoxState::Checked;
									HasCheckedChild = [&HasCheckedChild, &State](TSharedRef<FFICEditorAttributeBase> Attrib) {
										bool bAtLeastOne = false;
										for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Child : Attrib->GetChildAttributes()) {
											if (Child.Value->GetChildAttributes().Num() < 1) {
												if (Child.Value->bShowInGraph) {
													bAtLeastOne = true;
												} else {
													State = ECheckBoxState::Undetermined;
												}
											} else {
												bAtLeastOne = HasCheckedChild(Child.Value) || bAtLeastOne;
											}
										}
										return bAtLeastOne;
									};
									if (HasCheckedChild(Attribute->Attribute)) return State;
									return ECheckBoxState::Unchecked;
								})
								.OnCheckStateChanged_Lambda([this, Attribute](ECheckBoxState State) {
									TFunction<void(TSharedRef<FFICEditorAttributeBase>)> SetChildren;
									SetChildren = [&SetChildren, State](TSharedRef<FFICEditorAttributeBase> Attrib) {
										Attrib->bShowInGraph = State == ECheckBoxState::Checked;
										for (TTuple<FString, TSharedRef<FFICEditorAttributeBase>> Child : Attrib->GetChildAttributes()) {
											SetChildren(Child.Value);
										}
									};
									SetChildren(Attribute->Attribute);
									UpdateLeafAttributes();
								})
							];
						})
						.OnGetChildren_Lambda([](TSharedPtr<FFICEditorAttributeReference> InEntry, TArray<TSharedPtr<FFICEditorAttributeReference>>& OutArray) {
							OutArray = InEntry->GetChildren();
						})
					]
				]
				+SWidgetSwitcher::Slot()[
					SAssignNew(SequencerTreeView, SFICSequencerTreeView, Context)
					.OnUpdate_Lambda([this]() {
						if (Sequencer) Sequencer->UpdateRows();
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
				SAssignNew(Switcher, SWidgetSwitcher)
				.WidgetIndex_Lambda([this]() {
					return Mode;
				})
				+SWidgetSwitcher::Slot()[
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
				+SWidgetSwitcher::Slot()[
					SAssignNew(Sequencer, SFICSequencer, Context, SequencerTreeView.Get())
					.Frame_Lambda([this]() { return Context->GetCurrentFrame(); })
					.FrameRange_Lambda([this]() { return Context->GetActiveRange(); })
					.FrameHighlightRange_Lambda([this]() { return Context->GetScene()->AnimationRange; })
				]
			]
		]
	];

	OnSceneObjectsChangedDelegateHandle = Context->OnSceneObjectsChanged.AddLambda([this]() {
		UpdateEditorAttributes();
	});
	OnSceneSelectedSceneObjectChangedDelegateHandle = Context->OnSceneObjectSelectionChanged.AddLambda([this]() {
		UpdateEditorAttributeSelection();
	});

	UpdateEditorAttributes();
	
	if (UFICCamera* Camera = Context->GetActiveCamera()) {
		Context->GetEditorAttributes()[Camera]->bShowInGraph = true;
	}
}

void SFICTimelinePanel::UpdateLeafAttributes() {
	SelectedLeafAttributes.Empty();
	TFunction<bool(TSharedPtr<FFICEditorAttributeReference>)> AddLeaves;
	AddLeaves = [this, &AddLeaves](TSharedPtr<FFICEditorAttributeReference> Attribute) {
		bool bHasLeafs = false;
		if (Attribute->Attribute->GetChildAttributes().Num() < 1 && Attribute->Attribute->bShowInGraph) {
			SelectedLeafAttributes.Add(Attribute->Attribute);
			bHasLeafs = true;
		}
		for (TSharedPtr<FFICEditorAttributeReference> Child : Attribute->GetChildren()) {
			bHasLeafs = AddLeaves(Child) || bHasLeafs;
		}
		if (bHasLeafs) AttributeTree->SetItemExpansion(Attribute, true);
		return bHasLeafs;
	};
	for (TSharedPtr<FFICEditorAttributeReference> Item : Attributes) {
		bool _  = AddLeaves(Item);
	}
	Graph->SetAttributes(SelectedLeafAttributes);
	Graph->FitAll();
}

SFICTimelinePanel::~SFICTimelinePanel() {
	Context->OnSceneObjectsChanged.Remove(OnSceneObjectsChangedDelegateHandle);
	Context->OnSceneObjectSelectionChanged.Remove(OnSceneSelectedSceneObjectChangedDelegateHandle);
}

void SFICTimelinePanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

bool SFICTimelinePanel::SupportsKeyboardFocus() const {
	return true;
}

FReply SFICTimelinePanel::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) {
	return FReply::Handled().SetUserFocus(Switcher->GetActiveWidget().ToSharedRef());
}

void SFICTimelinePanel::UpdateEditorAttributes() {
	Attributes.Empty();
	for (TTuple<UObject*, TSharedRef<FFICEditorAttributeBase>> Attribute : Context->GetEditorAttributes()) {
		Attributes.Add(MakeShared<FFICEditorAttributeReference>(Cast<IFICSceneObject>(Attribute.Key)->GetSceneObjectName(), Attribute.Value));
	}
	AttributeTree->RebuildList();
	UpdateEditorAttributeSelection();
}

void SFICTimelinePanel::UpdateEditorAttributeSelection() {
	TFunction<void(TSharedRef<FFICEditorAttributeBase>, bool)> SetAttributeGraph;
	SetAttributeGraph = [&SetAttributeGraph](TSharedRef<FFICEditorAttributeBase> Attribute, bool bShowInGraph) {
		Attribute->bShowInGraph = bShowInGraph;
		for (TPair<FString, TSharedRef<FFICEditorAttributeBase>> Child : Attribute->GetChildAttributes()) {
			SetAttributeGraph(Child.Value, bShowInGraph);
		}
	};
	SetAttributeGraph(Context->GetAllAttributes().ToSharedRef(), false);
	
	if (UObject* Selection = Context->GetSelectedSceneObject()) {
		TSharedRef<FFICEditorAttributeBase> Attribute = Context->GetEditorAttributes()[Selection];
		bool bFound = false;
		TMap<FString, TSharedRef<FFICEditorAttributeBase>> Children = Attribute->GetChildAttributes();
		for (const TPair<FString, TSharedRef<FFICEditorAttributeBase>> Child : Children) {
			if (Child.Value->GetAttributeType() == FFICAttributePosition::TypeName) {
				SetAttributeGraph(Child.Value, true);
				bFound = true;
			}
		}
		if (!bFound) SetAttributeGraph(Attribute, true);
	}
	UpdateLeafAttributes();
}

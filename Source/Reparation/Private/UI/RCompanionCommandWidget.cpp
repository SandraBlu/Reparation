#include "UI/RCompanionCommandWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void URCompanionCommandWidget::NativeTick(const FGeometry& Geometry, float DeltaSeconds)
{
	Super::NativeTick(Geometry, DeltaSeconds);
	if (bTargeting) return;
	const FVector2D Offset = Geometry.AbsoluteToLocal(FSlateApplication::Get().GetCursorPos()) - Geometry.GetLocalSize() * 0.5f;
	Choice = ERCompanionMenuChoice::None;
	if (Offset.Size() < 45.f || Offset.Size() > 260.f) return;
	float Angle = FMath::Atan2(Offset.Y, Offset.X) + PI * 0.5f + PI / 4.f;
	if (Angle < 0.f) Angle += 2.f * PI;
	const int32 Sector = FMath::FloorToInt(Angle / (PI / 2.f)) % 4;
	Choice = static_cast<ERCompanionMenuChoice>(Sector + 1);
}

int32 URCompanionCommandWidget::NativePaint(const FPaintArgs& Args, const FGeometry& Geometry,
	const FSlateRect& CullingRect, FSlateWindowElementList& DrawElements, int32 LayerId,
	const FWidgetStyle& Style, bool bParentEnabled) const
{
	const int32 BaseLayer = Super::NativePaint(Args, Geometry, CullingRect, DrawElements, LayerId, Style, bParentEnabled);
	const FVector2D Center = Geometry.GetLocalSize() * 0.5f;
	const FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Regular", 16);
	auto Text = [&](const FVector2D& Position, const FText& Value, FLinearColor Color)
	{
		FSlateDrawElement::MakeText(DrawElements, BaseLayer + 2,
			Geometry.ToPaintGeometry(FVector2D(350.f, 30.f), FSlateLayoutTransform(Position)), Value, Font, ESlateDrawEffect::None, Color);
	};
	Text(Center + FVector2D(-140.f, -240.f), CompanionLabel, FLinearColor::White);
	Text(Center + FVector2D(-230.f, 230.f), Feedback, FLinearColor::White);
	if (bTargeting)
	{
		Text(Center + FVector2D(-190.f, 40.f), bAttackTargeting
			? NSLOCTEXT("CompanionUI", "PointerEnemy", "Click an enemy to attack; Esc to cancel")
			: NSLOCTEXT("CompanionUI", "PointerDestination", "Click ground to move; Esc to cancel"), FLinearColor::White);
		return BaseLayer + 2;
	}
	FSlateDrawElement::MakeBox(DrawElements, BaseLayer,
		Geometry.ToPaintGeometry(FVector2D(440.f), FSlateLayoutTransform(Center - FVector2D(220.f))),
		FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, FLinearColor(0.015f, 0.025f, 0.04f, 0.9f));
	const FText Labels[] = {NSLOCTEXT("CompanionUI", "Follow", "Follow"), NSLOCTEXT("CompanionUI", "Stay", "Stay"), NSLOCTEXT("CompanionUI", "Go", "Go to location"), NSLOCTEXT("CompanionUI", "Attack", "Attack")};
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const float Angle = -PI * 0.5f + Index * (PI / 2.f);
		const FVector2D Direction(FMath::Cos(Angle), FMath::Sin(Angle));
		const bool bSelected = static_cast<int32>(Choice) == Index + 1;
		TArray<FVector2D> Arc;
		for (int32 Step = 0; Step <= 20; ++Step)
		{
			const float PointAngle = Angle - PI / 4.f + Step * (PI / 2.f) / 20.f;
			Arc.Add(Center + FVector2D(FMath::Cos(PointAngle), FMath::Sin(PointAngle)) * 190.f);
		}
		FSlateDrawElement::MakeLines(DrawElements, BaseLayer + 1, Geometry.ToPaintGeometry(), Arc,
			ESlateDrawEffect::None, bSelected ? FLinearColor(0.25f, 0.8f, 1.f) : FLinearColor(0.4f, 0.45f, 0.5f), true, bSelected ? 5.f : 2.f);
		Text(Center + Direction * 120.f - FVector2D(55.f, 10.f), Labels[Index], bSelected ? FLinearColor(0.25f, 0.8f, 1.f) : FLinearColor::White);
	}
	Text(Center - FVector2D(34.f, 10.f), NSLOCTEXT("CompanionUI", "Cancel", "Cancel"), FLinearColor::Gray);
	return BaseLayer + 2;
}

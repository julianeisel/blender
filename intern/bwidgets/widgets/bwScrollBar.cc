#include "bwPainter.h"
#include "bwStyle.h"

#include "bwScrollBar.h"

using namespace bWidgets;


bwScrollBar::bwScrollBar(
        unsigned int width_hint, unsigned int height_hint) :
    bwAbstractButton("", WIDGET_TYPE_SCROLL_BAR, "bwScrollBar", width_hint, height_hint)
{
	
}

void bwScrollBar::draw(bwStyle& style)
{
	style.setWidgetStyle(*this);

	bwRectanglePixel rect_inner{rectangle};
	/* TODO maybe a wrapper could ensure old style is unchanged after drawing (by resetting it) */
	const char old_shade_top = base_style.shade_top;
	const char old_shade_bottom = base_style.shade_bottom;
	const bwGradient gradient_outer{
	        base_style.backgroundColor(),
	        base_style.shadeBottom(), base_style.shadeTop(),
	        bwGradient::DIRECTION_LEFT_RIGHT
	};
	bwPainter painter;

	rect_inner.ymax += (ratio * scroll_offset);
	rect_inner.ymin = rect_inner.ymax - (ratio * rectangle.height());

	painter.drawRoundboxWidgetBase(base_style, style, rectangle, gradient_outer, base_style.corner_radius);

	if (base_style.shadeTop() == base_style.shadeBottom()) {
		// no shading, skip
	}
	else if (base_style.shadeTop() > base_style.shadeBottom()) {
		base_style.shade_top += 20;
	}
	else {
		base_style.shade_bottom += 20;
	}

	const bwGradient gradient_inner{
	        base_style.decorationColor(),
	        base_style.shadeTop(), base_style.shadeBottom(),
	        bwGradient::DIRECTION_LEFT_RIGHT
	};
	painter.drawRoundboxWidgetBase(base_style, style, rect_inner, gradient_inner, base_style.corner_radius);

	base_style.shade_top = old_shade_top;
	base_style.shade_bottom = old_shade_bottom;
}

void bwScrollBar::mousePressEvent(
        const bwWidget::MouseButton button,
        const bwPoint& location)
{
	bwAbstractButton::mousePressEvent(button, location);
	mouse_press_scroll_offset = scroll_offset;
}

void bwScrollBar::mouseReleaseEvent(
        const bwWidget::MouseButton button,
        const bwPoint& location)
{
	bwAbstractButton::mouseReleaseEvent(button, location);
}

void bwScrollBar::mouseDragEvent(
        const bwWidget::MouseButton button,
        const bwDistance drag_distance)
{
	if (button == MOUSE_BUTTON_LEFT) {
		setScrollOffset(mouse_press_scroll_offset + drag_distance.y);
	}
}

void bwScrollBar::setScrollOffset(int value)
{
	scroll_offset = value;
	apply();
}

#include "bwAbstractButton.h"
#include "bwPainter.h"

#include "bwStyleCSS.h"

using namespace bWidgets;


void(* bwStyleCSS::polish_cb)(class bwWidget&) = nullptr;


bwStyleCSS::bwStyleCSS() :
    bwStyle(STYLE_CLASSIC_CSS)
{
	
}

#include "bwTextBox.h" // XXX Ugly
#include "bwPanel.h"
void bwStyleCSS::setWidgetStyle(bwWidget& widget)
{
	bwOptional<std::reference_wrapper<bwWidgetBaseStyle>> base_style;

	polish(widget);

	if (auto* button = widget_cast<bwAbstractButton*>(&widget)) {
		button->base_style.roundbox_corners = button->rounded_corners;
		base_style = button->base_style;
	}
	else if (auto* panel = widget_cast<bwPanel*>(&widget)) {
		panel->base_style.roundbox_corners = RoundboxCorner::ALL;
		base_style = panel->base_style;
	}
	else if (auto* text_box = widget_cast<bwTextBox*>(&widget)) {
		text_box->base_style.roundbox_corners = RoundboxCorner::ALL; // XXX Incorrect, should set this in layout.
		base_style = text_box->base_style;
	}
	else {
//		base_style->roundbox_corners = RoundboxCorner::ALL;
	}
}

void bwStyleCSS::polish(bwWidget& widget)
{
	if (polish_cb) {
		polish_cb(widget);
	}
}

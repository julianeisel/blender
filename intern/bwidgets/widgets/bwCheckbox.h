#pragma once

#include "bwAbstractButton.h"

namespace bWidgets {

class bwCheckbox : public bwAbstractButton {
 public:
  bwCheckbox(const std::string& text,
             std::optional<unsigned int> width_hint = std::nullopt,
             std::optional<unsigned int> height_hint = std::nullopt);

  void draw(class bwStyle& style) override;

  auto createHandler() -> std::unique_ptr<bwScreenGraph::EventHandler> override;

  auto isChecked() const -> bool;

 private:
  auto isInsideCheckbox(const bwPoint& point) const -> bool;

  auto getCheckboxRectangle() const -> bwRectanglePixel;
  auto getTextRectangle(const bwRectanglePixel& checkbox_rectangle) const -> bwRectanglePixel;
};

}  // namespace bWidgets

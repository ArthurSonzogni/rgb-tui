#include <fmt/core.h>
#include <cstddef>
#include <iostream>
#include "ftxui/component/checkbox.hpp"
#include "ftxui/component/container.hpp"
#include "ftxui/component/input.hpp"
#include "ftxui/component/menu.hpp"
#include "ftxui/component/radiobox.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/toggle.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

using namespace ftxui;

Element ColorTile(int r, int g, int b) {
  return text(L"")                        //
         | size(WIDTH, EQUAL, 20)         //
         | size(HEIGHT, EQUAL, 7)         //
         | bgcolor(Color::RGB(r, g, b));  //
}

wchar_t HexLetter(int x) {
  if (x <= 9)
    return U'0' + x;
  else
    return U'A' + x - 9;
};

std::wstring HexColor(int r, int g, int b) {
  std::wstring out;
  out += L"#";
  out += HexLetter(r / 16);
  out += HexLetter(r % 16);
  out += HexLetter(g / 16);
  out += HexLetter(g % 16);
  out += HexLetter(b / 16);
  out += HexLetter(b % 16);
  return out;
}

Element HexaElement(int r, int g, int b) {
  return text(HexColor(r, g, b));
}

void ToRGB(int h,
           int s,
           int v,
           int& r,
           int& g,
           int& b) {
  if (s == 0) {
    r = v;
    g = v;
    b = v;
    return;
  }

  uint8_t region = h / 43;
  uint8_t remainder = (h - (region * 43)) * 6;
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

  // clang-format off
  switch (region) {
    case 0: r = v, g = t, b = p; return;
    case 1: r = q, g = v, b = p; return;
    case 2: r = p, g = v, b = t; return;
    case 3: r = p, g = q, b = v; return;
    case 4: r = t, g = p, b = v; return;
    case 5: r = v, g = p, b = q; return;
  }
  // clang-format on
}

void ToHSV(int r, int g, int b, int& h, int& s, int& v) {
  int rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
  int rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);

  v = rgbMax;
  if (v == 0) {
    h = 0;
    s = 0;
    return;
  }

  s = 255 * int(rgbMax - rgbMin) / v;
  if (s == 0) {
    h = 0;
    return;
  }

  if (rgbMax == r)
    h = 0 + 43 * (g - b) / (rgbMax - rgbMin);
  else if (rgbMax == g)
    h = 85 + 43 * (b - r) / (rgbMax - rgbMin);
  else
    h = 171 + 43 * (r - g) / (rgbMax - rgbMin);
}

class GaugeInteger: public Component {
 public:
  GaugeInteger(std::wstring title, int& value, int min, int max)
      : title_(title), value_(value), min_(min), max_(max) {}

  std::function<void()> increase = [] {};
  std::function<void()> decrease = [] {};

  Element Render() {
    auto style = dim;
    auto gauge_color =
        Focused() ? color(Color::GrayLight) : color(Color::GrayDark);
    float percent = float(value_ - min_) / float(max_ - min_);
    return hbox({
               text(title_) | style | vcenter,
               hbox({
                   text(L"["),
                   gauge(percent) |underlined | xflex | reflect(gauge_box_),
                   text(L"]"),
               }) | xflex,
           }) |
           gauge_color | xflex | reflect(box_);
  }

  bool OnEvent(Event event) final {
    if (event.is_mouse())
      return OnMouseEvent(event);

    if (event == Event::ArrowLeft || event == Event::Character('h')) {
      value_ -= increment_;
      value_ = std::max(value_, min_);
      return true;
    }

    if (event == Event::ArrowRight || event == Event::Character('l')) {
      value_ += increment_;
      value_ = std::min(value_, max_);
      return true;
    }

    return Component::OnEvent(event);
  }

  bool OnMouseEvent(Event event) {
    if (!box_.Contain(event.mouse().x, event.mouse().y))
      return false;
    TakeFocus();
    if (!gauge_box_.Contain(event.mouse().x, event.mouse().y))
      return false;
    if (event.mouse().button == Mouse::Left &&
        event.mouse().motion == Mouse::Pressed) {
      value_ = min_ + (event.mouse().x - gauge_box_.x_min) * (max_ - min_) /
                          (gauge_box_.x_max - gauge_box_.x_min);
    }
    return true;
  }

 private:
  int& value_;
  int min_;
  int max_;
  int increment_ = 1;
  std::wstring title_;
  Box box_;
  Box gauge_box_;
};

class MainComponent : public Component {
 public:
  MainComponent(int& r, int& g, int& b) : r_(r), g_(g), b_(b) {
    Add(&container_);
    container_.Add(&color_hue_);
    container_.Add(&color_saturation_);
    container_.Add(&color_value_);
    container_.Add(&color_red_);
    container_.Add(&color_green_);
    container_.Add(&color_blue_);
    ToHSV(r_, g_, b_, h_, s_, v_);
    box_color_.x_min = 0;
    box_color_.y_min = 0;
    box_color_.x_max = 80;
    box_color_.y_max = 1;
  }

 private:
  Element Render() final {
    std::string rgb_txt = fmt::format("{:3} , {:3} , {:3} ", r_, g_, b_);
    std::string hsv_txt = fmt::format("{:3}°, {:3}%, {:3}%",  //
                                      int(h_ * 360. / 255.),  //
                                      int(s_ * 100. / 255.),  //
                                      int(v_ * 100. / 255.)   //
    );

    int hue = h_;
    Elements array;
    int x_length = std::max(10, box_color_.x_max - box_color_.x_min) + 1;
    int y_length = 11;
    for (int y = 0; y < y_length; ++y) {
      Elements line;
      for (int x = 0; x < x_length; ++x) {
        int saturation_1 = 255 * (y + 0.0f) / float(y_length);
        int saturation_2 = 255 * (y + 0.5f) / float(y_length);
        int value = 255 * x / float(x_length);
        line.push_back(text(L"▀")                                     //
                       | color(Color::HSV(hue, saturation_1, value))  //
                       | bgcolor(Color::HSV(hue, saturation_2, value)));
      }
      array.push_back(hbox(std::move(line)));
    }
    for (int saturation = 0; saturation < 255; saturation += 20) {
      Elements line;
      // for (int hue = 0; hue < 255; hue += 2) {
      array.push_back(hbox(std::move(line)));
    }

    return vbox({
               window(
                   text(L"[ rgb-tui ]") | center,  //
                   vbox({
                       hbox({
                           vbox(std::move(array)) | flex | reflect(box_color_),
                       }),
                       separator(),
                       hbox({
                           ColorTile(r_, g_, b_),
                           separator(),
                           vbox({
                               color_hue_.Render(),
                               color_saturation_.Render(),
                               color_value_.Render(),
                               separator(),
                               color_red_.Render(),
                               color_green_.Render(),
                               color_blue_.Render(),
                           }) | flex,
                       }),
                   })),
               hbox({
                   window(text(L" Hexa ") | center, HexaElement(r_, g_, b_)),
                   window(text(L" RGB ") | center, text(to_wstring(rgb_txt))),
                   window(text(L" HSV ") | center, text(to_wstring(hsv_txt))),
               }),
           }) |
           size(WIDTH, LESS_THAN, 80);
  };

  bool OnEvent(Event event) final {

    int r = r_;
    int g = g_;
    int b = b_;
    int h = h_;
    int s = s_;
    int v = v_;

    bool out = false;
    if (!event.is_mouse() || !OnMouseEvent(std::move(event)))
      out = Component::OnEvent(std::move(event));

    if (h != h_ || s != s_ || v != v_) {
      ToRGB(h_, s_, v_,  //
            r_, g_, b_);
    } else if (r != r_ || g != g_ || b != b_) {
      ToHSV(r_, g_, b_,  //
            h_, s_, v_);
    }
    return out;
  }

  bool OnMouseEvent(Event event) {
    if (!box_color_.Contain(event.mouse().x, event.mouse().y))
      return false;
    TakeFocus();
    if (event.mouse().button == Mouse::Left &&
        event.mouse().motion == Mouse::Pressed) {
      v_ = (event.mouse().x - box_color_.x_min) * 255 /
           (box_color_.x_max - box_color_.x_min);
      s_ = (event.mouse().y - box_color_.y_min) * 255 /
           (box_color_.y_max - box_color_.y_min);
    }
    return true;
  }

  int& r_;
  int& g_;
  int& b_;
  int h_;
  int s_;
  int v_;
  GaugeInteger color_red_ = GaugeInteger(L"Red:        ", r_, 0, 255);
  GaugeInteger color_green_ = GaugeInteger(L"Green:      ", g_, 0, 255);
  GaugeInteger color_blue_ = GaugeInteger(L"Blue:       ", b_, 0, 255);
  GaugeInteger color_hue_ = GaugeInteger(L"Hue:        ", h_, 0, 255);
  GaugeInteger color_saturation_ = GaugeInteger(L"Saturation: ", s_, 0, 255);
  GaugeInteger color_value_ = GaugeInteger(L"Value:      ", v_, 0, 255);
  Container container_ = Container::Vertical();

  Box box_color_;
};

int main(void) {
  int r = 255;
  int g = 0;
  int b = 0;
  auto screen = ScreenInteractive::TerminalOutput();
  auto main_component = MainComponent(r, g, b);
  screen.Loop(&main_component);

  return EXIT_SUCCESS;
}

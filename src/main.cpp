#include <fmt/core.h>
#include "clip.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/string.hpp"

using namespace ftxui;

Element ColorTile(int r, int g, int b) {
  return text("")                         //
         | size(WIDTH, EQUAL, 20)         //
         | size(HEIGHT, EQUAL, 7)         //
         | bgcolor(Color::RGB(r, g, b));  //
}

wchar_t HexLetter(int x) {
  if (x <= 9)
    return U'0' + x;
  else
    return U'A' + x - 0xA;
};

std::string HexColor(int r, int g, int b) {
  std::string out;
  out += "#";
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

void ToRGB(int h, int s, int v, int& r, int& g, int& b) {
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

class MainComponent : public ComponentBase {
 public:
  MainComponent(int& r, int& g, int& b) : r_(r), g_(g), b_(b) {
    Add(container_);
    ToHSV(r_, g_, b_, h_, s_, v_);
    box_color_.x_min = 0;
    box_color_.y_min = 0;
    box_color_.x_max = 80;
    box_color_.y_max = 1;
    clip::get_text(clipboard_);

    // Copy buttons
    for (int i = 0; i < output_.size(); i++) {
      copy_button_text_.push_back(output_[i]());
    }
    ButtonOption button_option = ButtonOption::Animated();
    for (int i = 0; i < output_.size(); i++) {
      copy_button_.push_back(Button(
          &copy_button_text_[i],
          [this, i] {
            clip::set_text(output_[i]());
            clip::get_text(clipboard_);
          },
          button_option));
      container_->Add(copy_button_.back());
    }
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
    int y_length = 16;

    int h, s, v;
    ToHSV(r_, g_, b_, h, s, v);
    int target_x = std::max(0, std::min(x_length - 1, (v * x_length) / 255));
    int target_y =
        std::max(0, std::min(2 * y_length - 1, (s * 2 * y_length) / 255));

    for (int y = 0; y < y_length; ++y) {
      int saturation_1 = 255 * (y + 0.0f) / float(y_length);
      int saturation_2 = 255 * (y + 0.5f) / float(y_length);
      Elements line;
      for (int x = 0; x < x_length; ++x) {
        int value = 255 * x / float(x_length);
        if (x == target_x) {
          if (2 * y == target_y - 1) {
            line.push_back(text("▀")                                      //
                           | color(Color::HSV(hue, saturation_1, value))  //
                           | bgcolor(Color::Black));                      //
            continue;
          }
          if (2 * y == target_y) {
            line.push_back(text("▀")              //
                           | color(Color::Black)  //
                           | bgcolor(Color::HSV(hue, saturation_2, value)));
            continue;
          }
        }
        line.push_back(text("▀")                                      //
                       | color(Color::HSV(hue, saturation_1, value))  //
                       | bgcolor(Color::HSV(hue, saturation_2, value)));
      }
      array.push_back(hbox(std::move(line)));
    }

    Elements copy_button_list;
    for (int i = 0; i < copy_button_.size(); i++) {
      copy_button_text_[i] = output_[i]();
      copy_button_list.push_back(copy_button_[i]->Render());
      copy_button_list.push_back(text(" "));
    }

    return hbox({
        window(text("  rgb-tui  ") | bold | center,
               vbox({
                   hbox({
                       vbox(std::move(array)) | flex | reflect(box_color_),
                   }),
                   separator(),
                   hbox({
                       ColorTile(r_, g_, b_),
                       separator(),
                       vbox({
                           color_hue_->Render(),
                           color_saturation_->Render(),
                           color_value_->Render(),
                           separator(),
                           color_red_->Render(),
                           color_green_->Render(),
                           color_blue_->Render(),
                       }) | flex,
                   }),
                   dbox({
                       separator(),
                       text("  Copy  ") | bold | center,
                   }),
                   hbox({
                       copy_button_[0]->Render(),
                       text(" "),
                       copy_button_[1]->Render(),
                       text(" "),
                       copy_button_[2]->Render(),
                   }),
                   text(" "),
                   hbox({
                       copy_button_[3]->Render(),
                       text(" "),
                       copy_button_[4]->Render(),
                   }),
                   dbox({
                       separator(),
                       text("  Clipboard  ") | bold | center,
                   }),
                   text(clipboard_),
               }) | size(WIDTH, LESS_THAN, 80)),
        filler(),
    });
  };

  bool OnEvent(Event event) final {
    int r = r_;
    int g = g_;
    int b = b_;
    int h = h_;
    int s = s_;
    int v = v_;

    bool out = false;

    if (event.is_mouse())
      out |= OnMouseEvent(std::move(event));
    out |= ComponentBase::OnEvent(std::move(event));

    if (h != h_ || s != s_ || v != v_) {
      ToRGB(h_, s_, v_,  //
            r_, g_, b_);
    } else if (r != r_ || g != g_ || b != b_) {
      ToHSV(r_, g_, b_,  //
            h_, s_, v_);
    }

    r_ = std::clamp(r_, 0, 255);
    g_ = std::clamp(g_, 0, 255);
    b_ = std::clamp(b_, 0, 255);
    h_ = std::clamp(h_, 0, 255);
    s_ = std::clamp(s_, 0, 255);
    v_ = std::clamp(v_, 0, 255);

    return out;
  }

  bool OnMouseEvent(Event event) {
    if (event.mouse().motion == Mouse::Released) {
      captured_mouse_ = nullptr;
      return true;
    }

    if (box_color_.Contain(event.mouse().x, event.mouse().y) &&
        CaptureMouse(event)) {
      TakeFocus();
    }

    if (event.mouse().button == Mouse::Left &&
        event.mouse().motion == Mouse::Pressed &&
        box_color_.Contain(event.mouse().x, event.mouse().y) &&
        !captured_mouse_) {
      captured_mouse_ = CaptureMouse(event);
    }

    if (captured_mouse_) {
      v_ = (event.mouse().x - box_color_.x_min) * 255 /
           (box_color_.x_max - box_color_.x_min);
      s_ = (event.mouse().y - box_color_.y_min) * 255 /
           (box_color_.y_max - box_color_.y_min);
      v_ = std::max(0, std::min(255, v_));
      s_ = std::max(0, std::min(255, s_));
      return true;
    }
    return false;
  }

  int& r_;
  int& g_;
  int& b_;
  int h_;
  int s_;
  int v_;
  std::string clipboard_;

  Component color_hue_ = Slider("Hue:        ", &h_, 0, 255, 1);
  Component color_saturation_ = Slider("Saturation: ", &s_, 0, 255, 1);
  Component color_value_ = Slider("Value:      ", &v_, 0, 255, 1);
  Component color_red_ = Slider("Red:        ", &r_, 0, 255, 1);
  Component color_green_ = Slider("Green:      ", &g_, 0, 255, 1);
  Component color_blue_ = Slider("Blue:       ", &b_, 0, 255, 1);

  std::vector<std::function<std::string()>> output_ = {
      [&] { return HexColor(r_, g_, b_); },
      [&] { return fmt::format("rgb({}, {}, {})", r_, g_, b_); },
      [&] {
        return fmt::format("hsl({}, {}%, {}%)",
                           int(h_ * 360. / 255.),  //
                           int(s_ * 100. / 255.),  //
                           int(v_ * 100. / 255.)   //
        );
      },
      [&] {
        int white = (255 - s_) * v_ / 255;
        int black = 255 - v_;
        return fmt::format("hwb({}, {}%, {}%)",       //
                           int(h_ * 360. / 255.),     //
                           int(white * 100. / 255.),  //
                           int(black * 100. / 255.)   //
        );
      },
      [&] {
        float rf = r_ / 255.0;
        float gf = g_ / 255.0;
        float bf = b_ / 255.0;

        float k = 1.0 - std::max({rf, gf, bf});
        if (k == 1.0) {
          return std::string("cmyk(0%, 0%, 0%, 100%)");
        }
        float c = (1.0 - rf - k) / (1.0 - k);
        float m = (1.0 - gf - k) / (1.0 - k);
        float y = (1.0 - bf - k) / (1.0 - k);
        return fmt::format("cmyk({}%, {}%, {}%, {}%)",  //
                           int(c * 100.),               //
                           int(m * 100.),               //
                           int(y * 100.),               //
                           int(k * 100.)                //
        );
      },
  };

  std::vector<std::string> copy_button_text_;
  std::vector<Component> copy_button_;

  Component container_ = Container::Vertical({
      color_hue_,
      color_saturation_,
      color_value_,
      color_red_,
      color_green_,
      color_blue_,
  });

  Box box_color_;
  CapturedMouse captured_mouse_;
};

int main(void) {
  int r = 255;
  int g = 0;
  int b = 0;
  auto screen = ScreenInteractive::TerminalOutput();
  screen.Loop(Make<MainComponent>(r, g, b));

  return EXIT_SUCCESS;
}

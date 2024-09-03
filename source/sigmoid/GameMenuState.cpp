#include "states.hpp"
#include <nwge/render/AspectRatio.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/Font.hpp>
#include <nwge/render/Texture.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sigmoid {

static void todo(const StringView &what) {
  dialog::info("TODO"_sv, what);
}

enum MenuButton {
  ButtonInvalid = -1,
  ButtonNew,
  ButtonLoad,
  ButtonExit,
  ButtonMax,
};

class GameMenuState final: public State {
public:
  GameMenuState(Game &&game)
    : mGame(std::move(game))
  {}

  bool preload() override {
    mBundle
      .load({"sigmoid.bndl"_sv})
      .nqFont("INTER.CFN"_sv, mFont);

    mHasLogo = !mGame.logo().empty();
    if(mHasLogo) {
      mGame.bundle().nqTexture(mGame.logo(), mLogo);
    }

    mHasBackground = !mGame.menuBackground().empty();
    if(mHasBackground) {
      mGame.bundle().nqTexture(mGame.menuBackground(), mBackground);
    }
    return true;
  }

  bool init() override {
    if(mHasLogo) {
      glm::ivec2 logoSize = mLogo.size();
      mLogoExtents = {
        cLogoH*f32(logoSize.x)/f32(logoSize.y),
        cLogoH
      };
    }
    return true;
  }

  bool on(Event &evt) override {
    bool result;
    switch(evt.type) {
    case Event::MouseMotion:
      mHover = buttonAt(evt.motion.to);
      break;

    case Event::MouseDown:
      mHover = mSelection = buttonAt(evt.click.pos);
      break;

    case Event::MouseUp:
      mHover = buttonAt(evt.click.pos);
      if(mHover != mSelection) {
        mSelection = mHover;
        break;
      }
      result = handleButtonPress();
      mSelection = ButtonInvalid;
      return result;

    default:
      break;
    }
    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});
    if(mHasBackground) {
      m4x3.rect({
        cBackgroundPos, cBackgroundExtents, mBackground.id
      }).draw();
    } else {
      render::color(cBackgroundColor);
      m4x3.rect({
        cBackgroundPos, cBackgroundExtents
      }).draw();
      render::color();
    }

    if(mHasLogo) {
      drawRect(cLogoPos, mLogoExtents, mLogo);
    } else {
      drawText(mGame.title(), cLogoPos, cBigText);
    }

    drawButton(ButtonNew, "New Game"_sv);
    drawButton(ButtonLoad, "Load Game"_sv);
    drawButton(ButtonExit, "Exit"_sv);

    auto cursor = mFont.cursor(m4x3.pos(cTitlePos), cSmallText);
    cursor
      << mGame.title() << '\n'
      << "by "_sv << mGame.author() << '\n'
      << 'v' << mGame.version();
    cursor.draw();
  }

private:
  data::Bundle mBundle;
  render::Font mFont;

  Game mGame;

  render::AspectRatio m1x1{1, 1};
  render::AspectRatio m4x3{4, 3};
  constexpr inline void drawRect(
    glm::vec3 pos, glm::vec2 extents
  ) const {
    render::rect(m4x3.pos(pos), m1x1.size(extents));
  }
  constexpr inline void drawRect(
    glm::vec3 pos, glm::vec2 extents, const render::Texture &texture
  ) const {
    render::rect(m4x3.pos(pos), m1x1.size(extents), texture);
  }
  constexpr inline void drawText(
    const StringView &text, glm::vec3 pos, f32 height
  ) const {
    mFont.draw(text, m4x3.pos(pos), height);
  }

  bool mHasBackground = false;
  render::Texture mBackground;
  static constexpr glm::vec3 cBackgroundPos{0, 0, 0.9f};
  static constexpr glm::vec2 cBackgroundExtents{1, 1};
  static constexpr glm::vec3 cBackgroundColor{0.1f, 0.1f, 0.1f};

  bool mHasLogo = false;
  render::Texture mLogo;
  static constexpr glm::vec3 cLogoPos{0.1f, 0.1f, 0.5f};
  static constexpr f32 cLogoH = 0.15f;
  static constexpr glm::vec2 cDefaultLogoExtents{3*cLogoH, cLogoH};
  glm::vec2 mLogoExtents = cDefaultLogoExtents;

  MenuButton mHover = ButtonInvalid;
  MenuButton mSelection = ButtonInvalid;

  static constexpr f32 cBigText = 0.05f;
  static constexpr f32 cButtonPadding = 0.01f;
  static constexpr glm::vec2 cButtonExtents{
    cDefaultLogoExtents.x,
    cBigText + 2*cButtonPadding
  };
  static constexpr glm::vec3 cButtonAnchor{
    cLogoPos.x,
    cLogoPos.y + cDefaultLogoExtents.y + cButtonExtents.y,
    0.5f
  };
  static constexpr f32 cButtonTextZOffset = -0.1f;
  static constexpr glm::vec3 cButtonBgColor{0.2f, 0.2f, 0.2f};
  static constexpr glm::vec3 cButtonTextColor{1, 1, 1};
  static constexpr glm::vec3 cButtonHoverColor{0.5f, 0.5f, 0.5f};
  static constexpr glm::vec3 cButtonSelectColor{0.7f, 0.7f, 0.7f};

  constexpr inline void drawButton(MenuButton idx, const StringView &text) const {
    glm::vec3 bgOff{0, f32(idx) * cButtonExtents.y, 0};
    if(idx == mSelection) {
      render::color(cButtonSelectColor);
    } else if(idx == mHover) {
      render::color(cButtonHoverColor);
    } else {
      render::color(cButtonBgColor);
    }
    drawRect(cButtonAnchor + bgOff, cButtonExtents);

    glm::vec3 textOff{cButtonPadding, cButtonPadding, cButtonTextZOffset};
    render::color(cButtonTextColor);
    drawText(text, cButtonAnchor + bgOff + textOff, cBigText);
  }

  [[nodiscard]]
  constexpr inline MenuButton buttonAt(glm::vec2 pos) const {
    glm::vec3 trueAnchor = m4x3.pos(cButtonAnchor);
    glm::vec2 trueExtents = m1x1.size(cButtonExtents);
    if(pos.x < trueAnchor.x) {
      return ButtonInvalid;
    }
    if(pos.y < trueAnchor.y) {
      return ButtonInvalid;
    }
    if(pos.x >= trueAnchor.x + trueExtents.x) {
      return ButtonInvalid;
    }
    if(pos.y >= trueAnchor.y + f32(ButtonMax)*trueExtents.y) {
      return ButtonInvalid;
    }
    return MenuButton(
      (pos.y - trueAnchor.y) / trueExtents.y
    );
  }

  bool handleButtonPress() {
    switch(mSelection) {
    case ButtonNew:
      swapStatePtr(scene(std::move(mGame), mGame.startScene()));
      return true;
    case ButtonLoad:
      todo("load game sub state");
      return true;
    case ButtonExit:
      return !dialog::confirm("Exit"_sv,
        "Are you sure you want to exit?");;
    default:
      return true;
    }
  }

  static constexpr f32 cSmallText = 0.03f;
  static constexpr glm::vec3 cVersionPos{0.0f, 1 - cSmallText, 0.5f};
  static constexpr glm::vec3 cAuthorPos{0.0f, cVersionPos.y - cSmallText, 0.5f};
  static constexpr glm::vec3 cTitlePos{0.0f, cAuthorPos.y - cSmallText, 0.5f};
};

State *gameMenu(Game &&game) {
  return new GameMenuState(std::move(game));
}

} // namespace sigmoid

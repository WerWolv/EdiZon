#include "guis/gui_guide.hpp"
#include "json.hpp"

using json = nlohmann::json;

GuiGuide::GuiGuide() : Gui() {
  std::ifstream configFile("romfs:/guide/" + std::to_string((g_selectedPage / 6) + 1) + "/" + std::to_string(g_selectedPage) + "/config.json");
  json configJson;
  configFile >> configJson;

  configFile.close();

  try {
    for (auto image : configJson["images"]) {
      std::ifstream data("romfs:/guide/" + std::to_string((g_selectedPage / 6) + 1) + "/" + std::to_string(g_selectedPage) + "/" + image["path"].get<std::string>(), std::ios::binary | std::ios::ate);
      GuiGuide::guide_obj_t imageObj = { 0 };

      imageObj.length = data.tellg();
      imageObj.data = new char[imageObj.length];
      imageObj.x = image["x"];
      imageObj.y = image["y"];
      imageObj.w = image["w"];
      imageObj.h = image["h"];
      imageObj.title = image["title"];
      imageObj.type = GuiGuide::guide_obj_t::TYPE_IMAGE;

      data.seekg(0, std::ios::beg);
      
      data.read(imageObj.data, imageObj.length);
      data.close();


      m_objects.push_back(imageObj);
    }
  } catch(json::parse_error& e) { }

  try {
    for (auto text : configJson["text"]) {
      std::ifstream data("romfs:/guide/" + std::to_string((g_selectedPage / 6) + 1) + "/" + std::to_string(g_selectedPage) + "/" + text["path"].get<std::string>(), std::ios::ate);
      GuiGuide::guide_obj_t textObj = { 0 };

      textObj.length = data.tellg();
      textObj.data = new char[textObj.length + 1]();
      textObj.x = text["x"];
      textObj.y = text["y"];
      textObj.type = GuiGuide::guide_obj_t::TYPE_TEXT;

      data.seekg(0, std::ios::beg);
      
      data.read(textObj.data, textObj.length);
      data.close();

      textObj.title = text["title"];

      m_objects.push_back(textObj);
    }
  } catch(json::parse_error& e) { }
}

GuiGuide::~GuiGuide() {
  for (GuiGuide::guide_obj_t obj : m_objects) {
    delete[] obj.data;
  }
}

void GuiGuide::update() {
  Gui::update();
}

void GuiGuide::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 87, 1220, 1, currTheme.textColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  Gui::drawTextAligned(fontTitle, 70, 60, currTheme.textColor, "\uE142", ALIGNED_LEFT);
  Gui::drawTextAligned(font24, 70, 23, currTheme.textColor, "        Quick start guide", ALIGNED_LEFT);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0E4 Previous     \uE0E5 Next     \uE0E1 Back", ALIGNED_RIGHT);
  
  color_t darkened = currTheme.backgroundColor;
  darkened.a = 0x9F;
  
  if (GuiGuide::g_selectedPage == 1)
    Gui::drawRectangled(800, Gui::g_framebuffer_height - 50, 200, 50, darkened);

  if (GuiGuide::g_selectedPage == GUIDE_PAGE_CNT)
    Gui::drawRectangled(1000, Gui::g_framebuffer_height - 50, 110, 50, darkened);

  for (GuiGuide::guide_obj_t obj : m_objects) {
    if (obj.type == GuiGuide::guide_obj_t::TYPE_TEXT) {
      Gui::drawTextAligned(font24, obj.x, obj.y, currTheme.textColor, obj.title.c_str(), ALIGNED_LEFT);
      Gui::drawTextAligned(font24, obj.x + 1, obj.y, currTheme.textColor, obj.title.c_str(), ALIGNED_LEFT);

      Gui::drawTextAligned(font14, obj.x + 20, obj.y + 45, currTheme.textColor, obj.data, ALIGNED_LEFT);
    }
    else {
      Gui::drawImage(obj.x, obj.y, obj.w, obj.h, (u8*)obj.data, IMAGE_MODE_RGBA32);
      Gui::drawTextAligned(font14, obj.x + obj.w - 20, obj.y + obj.h + 5, currTheme.textColor, obj.title.c_str(), ALIGNED_RIGHT);
    }
  }

  Gui::endDraw();
}

void GuiGuide::onInput(u32 kdown) {
  if (kdown & KEY_B) {
    GuiGuide::g_selectedPage = 1;
    Gui::g_nextGui = GUI_MAIN;
    return;
  }
  if (kdown & KEY_R) {

    if (GuiGuide::g_selectedPage < GUIDE_PAGE_CNT)
    GuiGuide::g_selectedPage++;
    Gui::g_nextGui = GUI_GUIDE;
    return;
  }

  if (kdown & KEY_L) {
    if (GuiGuide::g_selectedPage > 1)
      GuiGuide::g_selectedPage--;
    Gui::g_nextGui = GUI_GUIDE;
    return;
  }
}

void GuiGuide::onTouch(touchPosition &touch) {

}

void GuiGuide::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}

#include "guis/gui_information.hpp"
#include "json.hpp"

using json = nlohmann::json;

GuiInformation::GuiInformation() : Gui() {
  std::ifstream configFile("romfs:/guide/" + std::to_string(g_selectedPage) + "/config.json");
  json configJson;
  configFile >> configJson;

  configFile.close();

  for (auto image : configJson["images"]) {
    std::ifstream data("romfs:/guide/" + std::to_string(g_selectedPage) + "/" + image["path"].get<std::string>(), std::ios::binary | std::ios::ate);
    guide_obj_t imageObj = { .x = image["x"], .y = image["y"], .w = image["w"], .h = image["h"], .type = guide_obj_t::TYPE_IMAGE };

    imageObj.length = data.tellg();
    imageObj.data = new char[imageObj.length];
    data.seekg(0, std::ios::beg);
    
    data.read(imageObj.data, imageObj.length);
    data.close();

    imageObj.title = image["title"];

    m_objects.push_back(imageObj);
  }

  for (auto text : configJson["text"]) {
    std::ifstream data("romfs:/guide/" + std::to_string(g_selectedPage) + "/" + text["path"].get<std::string>(), std::ios::ate);
    guide_obj_t textObj = { .x = text["x"], .y = text["y"], .type = guide_obj_t::TYPE_TEXT };

    textObj.length = data.tellg();
    textObj.data = new char[textObj.length + 1]();
    data.seekg(0, std::ios::beg);
    
    data.read(textObj.data, textObj.length);
    data.close();

    textObj.title = text["title"];

    m_objects.push_back(textObj);
  }
}

GuiInformation::~GuiInformation() {
  for (guide_obj_t obj : m_objects) {
    delete[] obj.data;
  }
}

void GuiInformation::update() {
  Gui::update();
}

void GuiInformation::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 87, 1220, 1, currTheme.textColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  Gui::drawTextAligned(fontTitle, 70, 60, currTheme.textColor, "\uE142", ALIGNED_LEFT);
  Gui::drawTextAligned(font24, 70, 23, currTheme.textColor, "        Quick start guide", ALIGNED_LEFT);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0E4 Previous     \uE0E5 Next     \uE0E1 Back", ALIGNED_RIGHT);
  
  color_t darkened = currTheme.backgroundColor;
  darkened.a = 0x9F;
  
  if (GuiInformation::g_selectedPage == 1)
    Gui::drawRectangled(800, Gui::g_framebuffer_height - 50, 200, 50, darkened);

  if (GuiInformation::g_selectedPage == GUIDE_PAGE_CNT)
    Gui::drawRectangled(1000, Gui::g_framebuffer_height - 50, 110, 50, darkened);

  for (guide_obj_t obj : m_objects) {
    if (obj.type == guide_obj_t::TYPE_TEXT) {
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

void GuiInformation::onInput(u32 kdown) {
  if (kdown & KEY_B) {
    GuiInformation::g_selectedPage = 1;
    Gui::g_nextGui = GUI_MAIN;
    return;
  }
  if (kdown & KEY_R) {

    if (GuiInformation::g_selectedPage < GUIDE_PAGE_CNT)
    GuiInformation::g_selectedPage++;
    Gui::g_nextGui = GUI_INFORMATION;
    return;
  }

  if (kdown & KEY_L) {
    if (GuiInformation::g_selectedPage > 1)
      GuiInformation::g_selectedPage--;
    Gui::g_nextGui = GUI_INFORMATION;
    return;
  }
}

void GuiInformation::onTouch(touchPosition &touch) {

}

void GuiInformation::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}

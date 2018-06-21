#include "widget.hpp"
#include "widget_switch.hpp"
#include "widget_value.hpp"

#include <sstream>
#include <fstream>
#include "json.hpp"

#include "save.hpp"

u16 Widget::g_selectedWidgetIndex = 0;

Widget::Widget() {

}

Widget::~Widget() {

}

void Widget::drawWidgets(Gui *gui, WidgetList &widgets, u16 y, u16 start, u16 end) {
  for(;start < end; start++) {
    if(start > widgets.size() - 1) break;

    if(start == Widget::g_selectedWidgetIndex) {
      gui->drawRectangled(150, y, gui->framebuffer_width - 300, WIDGET_HEIGHT, currTheme.highlightColor);
      gui->drawRectangle(155, y + 5, gui->framebuffer_width - 315, WIDGET_HEIGHT - 10, currTheme.selectedButtonColor);
      gui->drawShadow(150, y, gui->framebuffer_width - 300, WIDGET_HEIGHT);
    }

    u32 textWidth, textHeight;
    gui->getTextDimensions(font20, widgets[start].title.c_str(), &textWidth, &textHeight);
    gui->drawText(font20, 200, y + ((WIDGET_HEIGHT / 2.0F) - (textHeight / 2.0F)) - 13, currTheme.textColor, widgets[start].title.c_str());
    gui->drawRectangle(50, y + WIDGET_HEIGHT + (WIDGET_SEPARATOR / 2) - 1, gui->framebuffer_width - 100, 1, currTheme.separatorColor);
    widgets[start].widget->draw(gui, gui->framebuffer_width - WIDGET_WIDTH - 100, y - 13);

    y += WIDGET_HEIGHT + WIDGET_SEPARATOR;
  }
}

bool Widget::getList(WidgetList& list, std::vector<std::tuple<std::string, size_t, u8*>> files)
{
  std::stringstream ss;
  ss << CONFIG_ROOT << std::uppercase << std::setfill('0') << std::setw(sizeof(u64)*2) << std::hex << Title::g_currTitle->getTitleID();
  ss << ".json";
	std::ifstream i(ss.str().c_str());
	if (i.fail())
  {
    printf("Failed to open config file.\n");
		return false;
  }

  nlohmann::json j;
	try { i >> j; }
	catch (nlohmann::json::parse_error& e)
	{
		printf("Failed to parse JSON file.\n");
		return false;
	}

  for (auto& item : j["files"])
  {
    u8* buffer;
    size_t size;
    if (loadSaveFile(&buffer, &size, Title::g_currTitle->getTitleID(), Account::g_currAccount->getUserID(), item.get<std::string>().c_str()) != 0)
      return false;
    files.push_back({item.get<std::string>(), size, buffer});
  }

  for (auto& item : j["items"])
  {
    std::string type = item["widget"]["type"].get<std::string>();
    u16 offsetAddr = (u16)stoul(item["offsetAddress"].get<std::string>(), NULL, 16);
    u16 address = (u16)stoul(item["address"].get<std::string>(), NULL, 16);
    printf("%x %x %s\n", offsetAddr, address, type.c_str());
    if (type == "bool")
      list.push_back({item["name"].get<std::string>(), new WidgetSwitch(item["widget"]["onValue"].get<u16>(), item["widget"]["offValue"].get<u16>(), offsetAddr, address, std::get<u8*>(files[item["file"].get<int>()]))});
    else if (type == "int")
      list.push_back({item["name"].get<std::string>(), new WidgetValue(item["widget"]["minValue"].get<u16>(), item["widget"]["maxValue"].get<u16>(), offsetAddr, address, std::get<u8*>(files[item["file"].get<int>()]))});
    else
    {
      printf("Unknown widget type: %s\n", type.c_str());
      return false;
    }
  }

	return true;
}

void Widget::handleInput(u32 kdown, WidgetList &widgets) {
  if(widgets.size() <= 0) return;
  widgets[Widget::g_selectedWidgetIndex].widget->onInput(kdown);
}

u16 Widget::getValue() {
  return m_value;
}

void Widget::setValue(u16 value) {
  this->m_value = value;
}

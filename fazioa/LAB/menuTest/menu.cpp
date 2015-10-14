#include <Arduino.h>
#include "constants.h"
#include <MenuSystem.h>


// Menu variables
MenuSystem ms;
Menu mmRoot("Smart Souliss Thermostat");
MenuItem mm_miBack("back");
Menu muMenu("Menu");
Menu muCrono("Crono");

Menu muMenu_mi_Bright("Luminosita'");
MenuItem muMenu_mi_Bright_100("100%");
MenuItem muMenu_mi_Bright_80("80%");
MenuItem muMenu_mi_Bright_60("60%");
MenuItem muMenu_mi_Bright_50("50%");
MenuItem muMenu_mi_Bright_30("30%");
MenuItem muMenu_mi_Bright_10("10%");
MenuItem muMenu_mi_Bright_5("5%");
MenuItem muMenu_mi_Bright_2("2%");


Menu muMenu_Clock("Orologio");
MenuItem muMenu_mi_Clock_ON("ON");
MenuItem muMenu_mi_Clock_OFF("OFF");

Menu muMenu_Crono("Crono");
MenuItem muMenu_mi_Crono_ON("ON");
MenuItem muMenu_mi_Crono_OFF("OFF");
MenuItem muMenu_mi_Crono_LEARN("Learn Mode");

Menu muMenu_System("Dispositivo");
MenuItem muMenu_mi_System_ON("ON");
MenuItem muMenu_mi_System_OFF("OFF");

Menu muMenu_Layouts("Layouts");
MenuItem muMenu_mi_Layouts_1("Lineare");
MenuItem muMenu_mi_Layouts_2("Rotondo");



MenuItem mu1_mi1("Level 2 - Item 1 (Item)");
MenuItem mu1_mi2("Level 2 - Item 2 (Item)");
MenuItem mu1_mi3("Level 2 - Item 3 (Item)");
MenuItem mu1_mi4("Level 2 - Item 4 (Item)");



MenuSystem* getMenu() {
  return &ms;
}

void on_itemBack_selected(MenuItem* p_menu_item)
{
  SERIAL_OUT.println("Back Selected");
  ms.back();

}

void on_item2_selected(MenuItem* p_menu_item)
{
  SERIAL_OUT.println("Item2 Selected");

}

void on_item3_selected(MenuItem* p_menu_item)
{
  SERIAL_OUT.println("L2 Item3 Selected");

}

void on_item4_selected(MenuItem* p_menu_item)
{
  SERIAL_OUT.println("L2 Item4 Selected");

}

void on_item5_selected(MenuItem* p_menu_item)
{
  SERIAL_OUT.println("L2 Item5 Selected");

}

void on_item6_selected(MenuItem* p_menu_item)
{
  SERIAL_OUT.println("L2 Item6 Selected");
}

void initMenu() {

  // Menu setup
  mmRoot.add_item(&mm_miBack, &on_itemBack_selected);
  mmRoot.add_menu(&muMenu);
  mmRoot.add_menu(&muCrono);

  muMenu.add_item(&mm_miBack, &on_itemBack_selected);
  muMenu.add_menu(&muMenu_mi_Bright);
  muMenu_mi_Bright.add_item(&mm_miBack, &on_itemBack_selected);
  muMenu_mi_Bright.add_item(&muMenu_mi_Bright_100, &on_item2_selected);
  muMenu_mi_Bright.add_item(&muMenu_mi_Bright_80, &on_item2_selected);
  muMenu_mi_Bright.add_item(&muMenu_mi_Bright_60, &on_item2_selected);
  muMenu_mi_Bright.add_item(&muMenu_mi_Bright_50, &on_item2_selected);
  muMenu_mi_Bright.add_item(&muMenu_mi_Bright_30, &on_item2_selected);
  muMenu_mi_Bright.add_item(&muMenu_mi_Bright_5, &on_item2_selected);
  muMenu_mi_Bright.add_item(&muMenu_mi_Bright_2, &on_item2_selected);


  muMenu.add_menu(&muMenu_Clock);
  muMenu_Clock.add_item(&mm_miBack, &on_itemBack_selected);
  muMenu_Clock.add_item(&muMenu_mi_Clock_ON, &on_item2_selected);
  muMenu_Clock.add_item(&muMenu_mi_Clock_OFF, &on_item2_selected);


  muMenu.add_menu(&muMenu_Crono);
  muMenu_Crono.add_item(&mm_miBack, &on_itemBack_selected);
  muMenu_Crono.add_item(&muMenu_mi_Crono_ON, &on_item2_selected);
  muMenu_Crono.add_item(&muMenu_mi_Crono_OFF, &on_item2_selected);
  muMenu_Crono.add_item(&muMenu_mi_Crono_LEARN, &on_item2_selected);

  muMenu.add_menu(&muMenu_System);
  muMenu_System.add_item(&mm_miBack, &on_itemBack_selected);
  muMenu_System.add_item(&muMenu_mi_System_ON, &on_item2_selected);
  muMenu_System.add_item(&muMenu_mi_System_OFF, &on_item2_selected);

  muMenu.add_menu(&muMenu_Layouts);
  muMenu_Layouts.add_item(&mm_miBack, &on_itemBack_selected);
  muMenu_Layouts.add_item(&muMenu_mi_Layouts_1, &on_item2_selected);
  muMenu_Layouts.add_item(&muMenu_mi_Layouts_2, &on_item2_selected);

  ms.set_root_menu(&mmRoot);
}


void printMenu() {
  // Display the menu
  Menu const* cp_menu;
  cp_menu = ms.get_current_menu();

  Serial.print("Current menu name: ");
  Serial.println(cp_menu->get_name());

  MenuComponent const* cp_menu_sel = cp_menu->get_selected();
  for (int i = 0; i < cp_menu->get_num_menu_components(); ++i)
  {
    MenuComponent const* cp_m_comp = cp_menu->get_menu_component(i);
    SERIAL_OUT.print(cp_m_comp->get_name());

    if (cp_menu_sel == cp_m_comp)
      SERIAL_OUT.print("<<< ");

    SERIAL_OUT.println("");
  }
}






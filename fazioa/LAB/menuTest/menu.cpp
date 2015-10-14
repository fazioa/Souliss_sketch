#include <Arduino.h>
#include "constants.h"
#include <MenuSystem.h>


// Menu variables
MenuSystem ms;
Menu mmRoot("Smart Souliss Thermostat");
MenuItem mm_miBack("back");
Menu muMenu("Menu");
Menu muCrono("Crono");

MenuItem muMenu_Bright("Luminosita'");
MenuItem muMenu_Clock("Orologio");
MenuItem muMenu_Crono("Crono");
MenuItem muMenu_System("Dispositivo");
MenuItem muMenu_Layouts("Layouts");

MenuItem mu1_mi1("Level 2 - Item 1 (Item)");
MenuItem mu1_mi2("Level 2 - Item 2 (Item)");
MenuItem mu1_mi3("Level 2 - Item 3 (Item)");
MenuItem mu1_mi4("Level 2 - Item 4 (Item)");



MenuSystem getMenu(){
 return ms;
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

void initMenu(){

   // Menu setup
  mmRoot.add_item(&mm_miBack, &on_itemBack_selected);
  mmRoot.add_menu(&muMenu);
  mmRoot.add_menu(&muCrono);
  
  muMenu.add_item(&mm_miBack, &on_itemBack_selected);
  muMenu.add_item(&muMenu_Bright, &on_item3_selected);
  muMenu.add_item(&muMenu_Clock, &on_item4_selected);
  muMenu.add_item(&muMenu_Crono, &on_item5_selected);
  muMenu.add_item(&muMenu_System, &on_item6_selected);
  muMenu.add_item(&muMenu_Layouts, &on_item5_selected);
  
  
  ms.set_root_menu(&mmRoot);
}


void printMenu(MenuSystem ms){
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






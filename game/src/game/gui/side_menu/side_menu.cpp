#include "side_menu.h"

#include "lc_client/eng_gui/includes.h"
#include "buttons/task_menu_button.h"


SideMenu::SideMenu(TaskMenu* pTaskMenu, GuiDependencies dependencies) : Widget() {
	auto pMenuBackground = new ColorBackground(dependencies.pStyle->getColor("background_dark"), dependencies);
	auto pButtonBackground = new ColorBackground(dependencies.pStyle->getColor("button"), dependencies);

	this->setSize(500, 800);
	this->setName("side_menu");
	this->setBackground(pMenuBackground);

	VBox* pVBox = new VBox();
	pVBox->setMode(BoxMode::STRETCH_SPACING);
	pVBox->setPadding(5, 5);
	TaskMenuButton* pButtonTasks = new TaskMenuButton(pTaskMenu, dependencies);
	Button* pButton2 = new Button(dependencies);
	pButton2->setBackground(pButtonBackground);
	pButton2->setSize(80, 80);
	pButton2->setName("button_other");
	Button* pButton3 = new Button(dependencies);
	pButton3->setBackground(pButtonBackground);
	pButton3->setSize(80, 80);
	pButton3->setName("button_other");

	Button* pButton4 = new Button(dependencies);
	pButton4->setBackground(pButtonBackground);
	pButton4->setSize(80, 80);
	pButton4->setName("button_other");

	pVBox->addChild(pButtonTasks);
	pVBox->addChild(pButton2);
	pVBox->addChild(pButton3);

	this->setLayout(pVBox);
}

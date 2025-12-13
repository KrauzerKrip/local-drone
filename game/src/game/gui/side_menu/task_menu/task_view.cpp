#include "task_view.h"

#include "lc_client/eng_gui/includes.h"
#include "lc_client/eng_gui/paint_objects/color_background.h"


TaskView::TaskView(GuiDependencies dependencies) {
	auto pBaseBackground = new ColorBackground(dependencies.pStyle->getColor("button"), dependencies);

	this->setSize(1, 50);

	this->setBackground(pBaseBackground);

	HBox* pHBox = new HBox();
	pHBox->setBoxMode(BoxMode::STRETCH_WIDGETS);
	m_pLabel = new TextWidget(pBaseBackground, dependencies);
	m_pLabel->setTextSize(16);
	m_pLabel->setTextColor(0, 0, 0, 255);
	m_pLabel->setSize(70, 36);
	pHBox->addChild(m_pLabel);
	this->setLayout(pHBox);
}

void TaskView::setPurchasesData(const TaskData& taskData) { m_pLabel->setText(taskData.name); }

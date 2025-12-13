#include "gl_gui_dependencies_fabric.h"

#include "lc_client/eng_graphics/gui/opengl/gl_background_render.h"
#include "lc_client/eng_graphics/gui/opengl/gl_text_render.h"
#include "lc_client/eng_graphics/gui/opengl/gl_text_zoffset_calculator.h"
#include "lc_client/eng_graphics/gui/opengl/gl_widget_zoffset_calculator.h"
#include "lc_client/eng_gui/widgets/style_impl.h"


GuiDependenciesFabricGl::GuiDependenciesFabricGl(IConsole* pConsole, ShaderLoaderGl* pShaderWorkGl, IInput* pInput,
	TextureManager* pTextureManager, FramebufferController* pFramebufferController, IWindow* pWindow,
	std::filesystem::path fontPath) {
	BackgroundRenderGl* pBackgroundRender =
		new BackgroundRenderGl(pConsole, pShaderWorkGl, pTextureManager, pFramebufferController, pWindow);
	TextRenderGl* pTextRender = new TextRenderGl(pWindow, pConsole, pShaderWorkGl, fontPath);
	WidgetZOffsetCalculatorGl* pWidgetZOffsetCalculator = new WidgetZOffsetCalculatorGl();
	TextZOffsetCalculatorGl* pTextZOffsetCalculator = new TextZOffsetCalculatorGl();
	InputController* pInputController = new InputController(pInput);

	// @TODO move somewhere
	StyleImpl* pStyle = new StyleImpl();
	// pStyle->setColor("blur_background_base", 160, 160, 160, 255);
	pStyle->setColor("blur_background_base", 160, 160, 160, 255);
	pStyle->setColor("blur_background_dark", 32, 32, 32, 255);
	pStyle->setColor("background_base", 255, 255, 255, 255);
	pStyle->setColor("background_dark", 0, 0, 0, 255);
	pStyle->setColor("button", 239, 225, 9, 255);
	pStyle->setColor("button_hover", 48, 48, 48, 255);
	pStyle->setColor("background_inaccessible", 121, 121, 121, 255);
	pStyle->setColor("button_inaccessible", 91, 91, 91, 255);
	pStyle->setBlurIntensitry("base", 0.05);

	m_dependecies.pBackgroundRender = pBackgroundRender;
	m_dependecies.pWidgetZOffsetCalculator = pWidgetZOffsetCalculator;
	m_dependecies.pBackgroundRender = pBackgroundRender;
	m_dependecies.pTextRender = pTextRender;
	m_dependecies.pWidgetZOffsetCalculator = pWidgetZOffsetCalculator;
	m_dependecies.pTextZOffsetCalculator = pTextZOffsetCalculator;
	m_dependecies.pInputController = pInputController;
	m_dependecies.pStyle = pStyle;
}

GuiDependencies GuiDependenciesFabricGl::getDependencies() { return m_dependecies; }

#ifndef GUI_H
#define GUI_H

#include "Common.h"
#include "Math3D.h"
#include "Shader.h"
#include "glad_Core-33/include/glad/glad.h"

enum Text_AlignHorizontal { TAH_Left, TAH_Center, TAH_Right };
enum Text_AlignVertical { TAV_Top, TAV_Center, TAV_Bottom };

typedef struct Font Font;

typedef Vec3 RGB;
typedef Vec4 RGBA;

typedef struct GUI_Text {
	Font *Font;

	const char *TextContents;

	RGBA Color_FG, Color_BG;
	i32 Bold, Underlined, Italics;

	enum Text_AlignHorizontal AlignHorizontal;
	enum Text_AlignVertical AlignVertical;
} GUI_Text;

enum GUI_ElementType {
	// Static elements
	GUI_ElementType_ColoredRect,
	GUI_ElementType_Image,
	GUI_ElementType_TextBox,

	//
	GUI_ElementType_Button,
	GUI_ElementType_Checkbox,
	GUI_ElementType_Radio,
	GUI_ElementType_Slider,
};

typedef struct GUI_Skin {
} GUI_Skin;

typedef struct GUI_Element {
	i32 Layer;
	Vec2 Position;
	Vec2 Size;
	enum GUI_ElementType Type;

	union {
		RGBA Color;
		GLuint Texture;
	};
} GUI_Element;

typedef struct GUI_Panel {
	i32 Layer;
	GUI_Element *PanelElements;

	GUI_Element *ChildElements;
} GUI_Panel;

typedef struct GUI {
	GUI_Panel *Panels;
	GUI_Element *Elements;

	// Element that has been hovered over by the mouse.
	GUI_Element *HotElement;
	// Element, over which the mouse has had one of its keys held down.
	GUI_Element *ActiveElement;

	GLuint Framebuffer;
	GLuint ColorAttachment;

	GLuint *VAOs, *VBOs;
	Shader *Shader;
} GUI;

extern GUI *GUI_Init();
extern void GUI_Free(GUI *);

extern GUI_Panel *GUI_AddPanel(GUI *);
extern GUI_Element *GUI_AddElement(GUI *);
extern GUI_Element *GUI_Panel_AddElement(GUI_Panel *);

extern void GUI_HandleInput(GUI *);
extern void GUI_Render(GUI *);

#endif

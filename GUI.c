#if 0
#	include "GUI.h"

#	include <stdlib.h>
#	include <string.h>

static const char *GUI_ShaderVert =
    "#version 330\n"
    ""
    "in vec2 pos;\n"
    "in vec2 uv;\n"
    "out vec2 fUV;\n"
    "uniform mat4 persp;\n"
    ""
    "void main() {"
    "    gl_Position = persp * vec4(pos, 0, 1); "
    "    fUV = uv;"
    "}"
    ;
static const char *GUI_ShaderFrag =
    "#version 330"               "\n"
    ""
    "in vec2 fUV;"               "\n"
    "out vec4 fColor;"           "\n"
    "uniform vec4 color;"        "\n"
    "uniform sampler2D texture;" "\n"
    "uniform float hasTexture;"  "\n"
    ""
    "void main() {"
    "    fColor = mix(color, texture(texture, fUV), hasTexture);"
    "}"
    ;

GUI *GUI_Init()
{
    GUI *Res = malloc(sizeof(GUI));
    memset(Res, 0, sizeof(GUI));

    Res->Elements = malloc(sizeof(Rect) * 64);
    Res->Shader = Shader_FromSrc(GUI_ShaderVert, GUI_ShaderFrag);

    // Initialize GUI framebuffer.
    u32 Width = 1280, Height = 720;
    glGenFramebuffers(1, &Res->Framebuffer);

    glGenTextures(1, &Res->ColorAttachment);
    glBindTexture(GL_TEXTURE_2D, Res->ColorAttachment);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, Res->Framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Res->ColorAttachment, 0);

    // Clean up after ourselves.
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return Res;
}

void GUI_HandleInput(GUI* gui)
{
}

void GUI_Render(GUI* gui)
{
    // TODO: Record performance

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gui->Framebuffer);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
#endif

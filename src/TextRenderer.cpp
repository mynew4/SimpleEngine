#include "TextRenderer.h"
#include "Environment.h"
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H


TextRenderer::TextRenderer()
{
	// Configure VAO/VBO for texture quads
	std::string vShader(Environment::GetShaderPath("text.vs")),
		fShader(Environment::GetShaderPath("text.fs"));
	shader = new Shader(vShader, fShader);
	glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
	shader->Bind();
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "projection"), 1, GL_FALSE, (GLfloat*)&projection);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	shader->Unbind();
}

TextRenderer::~TextRenderer()
{

}

void TextRenderer::Draw(std::wstring text,GLfloat x, GLfloat y,glm::vec3 color)
{
	// RenderText(L"\u554a\u662f\u7684\u8bf7\u95ee\u8bf7\u95ee\u53bb\u95ee\u9a71\u868a\u5668\u4e3a\u5168\u5a01\u5168\u5a01\u5168\u5a01\u5168\u5a01\u6076\u8da3\u5473\u6076\u6c14",
	// 	x, y,
	// 	1,
	// 	glm::vec3(1.0f, 1.0f,0));
    TextRenderer::RenderText(text, x, y,1, color);
}


void TextRenderer::RenderText(std::wstring text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	// Activate corresponding render state	
	shader->Bind();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glUniform3f(glGetUniformLocation(shader->GetProgramID(), "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	// Load font as face
	FT_Face face;

	if (FT_New_Face(ft, Environment::GetAbsPath("Resource/Font/simsun.ttc").c_str(), 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 28);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (auto i = 0; i<text.size(); i++)
	{
		wchar_t *c = &text[i];

		if (Characters.find(*c) == Characters.end()) {
			// Load character glyph 
			if (FT_Load_Char(face, *c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// Generate texture
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Now store character for later use
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				(GLuint)face->glyph->advance.x
			};
	
			Characters[*c] = character;
		}

		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y -(ch.Size.y - ch.Bearing.y)* scale ;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		// GLfloat vertices[6][4] = {
		// 	{ xpos,     ypos + h,   0.0,0.0 },
		// 	{ xpos,     ypos,       0.0,1.0 },
		// 	{ xpos + w, ypos,       1.0, 1.0 },
		// 	{ xpos,     ypos + h,   0.0, 0.0 },
		// 	{ xpos + w, ypos,       1.0, 1.0 },
		// 	{ xpos + w, ypos + h,   1.0, 0.0 }
		// };

		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0,0.0 },
			{ xpos,     ypos,       0.0,1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindVertexArray(VAO);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}

	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	shader->Unbind();
}

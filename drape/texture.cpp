#include "drape/texture.hpp"

#include "drape/glfunctions.hpp"
#include "drape/glextensions_list.hpp"

#include "base/math.hpp"

#define ASSERT_ID ASSERT(GetID() != -1, ())

namespace dp
{

Texture::ResourceInfo::ResourceInfo(m2::RectF const & texRect)
  : m_texRect(texRect) {}

m2::RectF const & Texture::ResourceInfo::GetTexRect() const
{
  return m_texRect;
}

//////////////////////////////////////////////////////////////////

Texture::Texture()
  : m_textureID(-1)
  , m_width(0)
  , m_height(0)
  , m_format(dp::UNSPECIFIED)
{
}

Texture::~Texture()
{
  if (m_textureID != -1)
    GLFunctions::glDeleteTexture(m_textureID);
}

void Texture::Create(uint32_t width, uint32_t height, TextureFormat format)
{
  Create(width, height, format, MakeStackRefPointer<void>(NULL));
}

void Texture::Create(uint32_t width, uint32_t height, TextureFormat format, RefPointer<void> data)
{
  m_format = format;
  m_width = width;
  m_height = height;
  if (!GLExtensionsList::Instance().IsSupported(GLExtensionsList::TextureNPOT))
  {
    m_width = my::NextPowOf2(width);
    m_height = my::NextPowOf2(height);
  }

  m_textureID = GLFunctions::glGenTexture();
  GLFunctions::glBindTexture(m_textureID);

  glConst layout;
  glConst pixelType;
  UnpackFormat(format, layout, pixelType);

  GLFunctions::glTexImage2D(m_width, m_height, layout, pixelType, data.GetRaw());
  SetFilterParams(gl_const::GLLinear, gl_const::GLLinear);
  SetWrapMode(gl_const::GLClampToEdge, gl_const::GLClampToEdge);
}

void Texture::SetFilterParams(glConst minFilter, glConst magFilter)
{
  ASSERT_ID;
  GLFunctions::glTexParameter(gl_const::GLMinFilter, minFilter);
  GLFunctions::glTexParameter(gl_const::GLMagFilter, magFilter);
}

void Texture::SetWrapMode(glConst sMode, glConst tMode)
{
  ASSERT_ID;
  GLFunctions::glTexParameter(gl_const::GLWrapS, sMode);
  GLFunctions::glTexParameter(gl_const::GLWrapT, tMode);
}

void Texture::UploadData(uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                         TextureFormat format, RefPointer<void> data)
{
  ASSERT_ID;
  ASSERT(format == m_format, ());
  glConst layout;
  glConst pixelType;

  UnpackFormat(format, layout, pixelType);

  GLFunctions::glTexSubImage2D(x, y, width, height, layout, pixelType, data.GetRaw());
}

TextureFormat Texture::GetFormat() const
{
  return m_format;
}

uint32_t Texture::GetWidth() const
{
  ASSERT_ID;
  return m_width;
}

uint32_t Texture::GetHeight() const
{
  ASSERT_ID;
  return m_height;
}

float Texture::GetS(uint32_t x) const
{
  ASSERT_ID;
  return x / (float)m_width;
}

float Texture::GetT(uint32_t y) const
{
  ASSERT_ID;
  return y / (float)m_height;
}

void Texture::Bind() const
{
  ASSERT_ID;
  GLFunctions::glBindTexture(GetID());
}

uint32_t Texture::GetMaxTextureSize()
{
  return GLFunctions::glGetInteger(gl_const::GLMaxTextureSize);
}

void Texture::UnpackFormat(TextureFormat format, glConst & layout, glConst & pixelType)
{
  bool requiredFormat = GLExtensionsList::Instance().IsSupported(GLExtensionsList::RequiredInternalFormat);
  switch (format) {
  case RGBA8:
    layout = requiredFormat ? gl_const::GLRGBA8 : gl_const::GLRGBA;
    pixelType = gl_const::GL8BitOnChannel;
    break;
  case RGBA4:
    layout = requiredFormat ? gl_const::GLRGBA4 : gl_const::GLRGBA;
    pixelType = gl_const::GL4BitOnChannel;
    break;
  case ALPHA:
    layout = requiredFormat ? gl_const::GLAlpha8 : gl_const::GLAlpha;
    pixelType = gl_const::GL8BitOnChannel;
    break;
  default:
    ASSERT(false, ());
    break;
  }
}

int32_t Texture::GetID() const
{
  return m_textureID;
}

} // namespace dp

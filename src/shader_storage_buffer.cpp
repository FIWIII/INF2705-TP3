#include "shader_storage_buffer.hpp"

ShaderStorageBuffer::ShaderStorageBuffer()
{
}

ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer&& other)
{
    *this = std::move(other);
}


ShaderStorageBuffer::~ShaderStorageBuffer()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &id_);
}

void ShaderStorageBuffer::allocate(const void* data, GLsizeiptr byteSize, GLenum usage)
{
    glGenBuffers(1, &id_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, usage);
}

void ShaderStorageBuffer::setBindingIndex(GLuint index)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id_);
}

void ShaderStorageBuffer::updateData(const void* data, GLintptr offset, GLsizeiptr byteSize)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id_);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteSize, data);
}

void ShaderStorageBuffer::bindAsArray()
{
    glBindBuffer(GL_ARRAY_BUFFER, id_);
}

ShaderStorageBuffer& ShaderStorageBuffer::operator=(ShaderStorageBuffer&& other)
{
    id_ = other.id_;
    other.id_ = 0;
    return *this;
}
